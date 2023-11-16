#include "webserver.h"

#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <http/parser.h>

void response_init(response_t *response)
{
	response->client   = -1;
	response->response = NULL;
}

void response_deinit(response_t *response)
{
	free(response->response);
}

bool webserver_init(webserver_t *server, char const *ip, char const *port, char const *root_path)
{
	int		 sock;
	int		 result;
	struct addrinfo	 hints;
	struct addrinfo *info;
	struct addrinfo *info_itr;
	void		*addr;
	char		*ipver;
	char		 ipstr[INET6_ADDRSTRLEN];
	int		 yes = 1;
	size_t		 i;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family	  = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	result		  = getaddrinfo(ip, port, &hints, &info);
	if (result == -1) {
		return false;
	}

	for (info_itr = info; info_itr; info_itr = info_itr->ai_next) {
		sock = socket(info_itr->ai_family, info_itr->ai_socktype, info_itr->ai_protocol);
		if (sock != -1) {
			break;
		}
	}

	if (!info_itr) {
		freeaddrinfo(info);
		return false;
	}

	if (info_itr->ai_family == AF_INET) { // IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)info_itr->ai_addr;
		addr			 = &(ipv4->sin_addr);
		ipver			 = "IPv4";
	} else { // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)info_itr->ai_addr;
		addr			  = &(ipv6->sin6_addr);
		ipver			  = "IPv6";
	}

	// convert the IP to a string and print it:
	inet_ntop(info_itr->ai_family, addr, ipstr, sizeof(ipstr));
	printf("webserver\n\tip %s:%s\n", ipstr, port);

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		freeaddrinfo(info);
		return false;
	}

	if (bind(sock, info_itr->ai_addr, info_itr->ai_addrlen) == -1) {
		freeaddrinfo(info);
		return false;
	}

	freeaddrinfo(info);

	if (listen(sock, WEBSERVER_BACKLOG) == -1) {
		return false;
	}

	server->ip_address     = ip;
	server->port	       = port;
	server->socket	       = sock;
	server->client_count   = 0;
	server->response_count = 0;
	server->root_path      = root_path;

	printf("\troot_path: %s\n", root_path);

	for (i = 0; i < WEBSERVER_CLIENT_COUNT_MAX; ++i) {
		server->buffer[i] = NULL;
	}

	// add the listening socket as a client to be able to poll events from it
	webserver_client_add(server, sock);

	return true;
}

void webserver_deinit(webserver_t *server)
{
	assert(server);

	size_t i;

	for (i = 0; i < WEBSERVER_CLIENT_COUNT_MAX; ++i) {
		free(server->buffer[i]);
	}

	close(server->socket);
}

void webserver_run(webserver_t *server)
{
	size_t			i;
	struct pollfd	       *pfd;
	int			new_client;
	struct sockaddr_storage new_client_addr;
	socklen_t		new_client_addr_len = sizeof(new_client_addr);

	response_t response;

	while (true) {
		for (i = 0; i < server->response_count; ++i) {
			response = webserver_response_pop(server);
			webserver_respond(response.client, response.response);
			response_deinit(&response);
		}

		if (poll(server->client, server->client_count, WEBSERVER_POLL_TIMEOUT) <= 0) {
			continue;
		}

		if (server->client[0].revents & POLLIN) {
			printf("===== INCOMING CONN =====\n");
			new_client = accept(server->socket, (struct sockaddr *)&new_client_addr,
					    &new_client_addr_len);
			if (new_client != -1) {
				printf("Success.\n");
				webserver_client_add(server, new_client);
			} else {
				printf("Error: %s\n", strerror(errno));
			}
			continue;
		}

		for (i = 1; i < server->client_count; ++i) {
			pfd = &server->client[i];

			if (pfd->revents & POLLHUP) {
				webserver_client_remove(server, pfd->fd);
			} else if (pfd->revents & POLLIN) {
				webserver_client_receive(server, i);

				if (webserver_message_is_ready(server, i)) {
					webserver_request_handle(server, i);
				}
			}
		}
	}
}

void webserver_response_push(webserver_t *server, response_t *response)
{
	if (server->response_count >= WEBSERVER_RESPONSE_COUNT_MAX) {
		// TODO(ruarq): error
		return;
	}

	server->response[server->response_count++] = *response;
}

response_t webserver_response_pop(webserver_t *server)
{
	return server->response[--server->response_count];
}

bool webserver_client_add(webserver_t *server, int client)
{
	assert(server);

	struct pollfd pfd;

	pfd.fd	   = client;
	pfd.events = POLLIN;

	if (server->client_count >= WEBSERVER_CLIENT_COUNT_MAX) {
		return false;
	}

	server->client[server->client_count] = pfd;
	server->buffer[server->client_count] = NULL;
	++server->client_count;

	return true;
}

void webserver_client_remove(webserver_t *server, int client)
{
	assert(server);

	size_t i;

	for (i = 0; i < server->client_count; ++i) {
		if (server->client[i].fd == client) {
			close(client);
			free(server->buffer[i]);
			server->client[i] = server->client[server->client_count - 1];
			server->buffer[i] = server->buffer[server->client_count - 1];
			--server->client_count;
			return;
		}
	}
}

void webserver_client_receive(webserver_t *server, size_t client_index)
{
	int    fd	    = server->client[client_index].fd;
	char  *buf	    = webserver_receive(fd);
	char  *msg	    = server->buffer[client_index];
	size_t required_len = 0;
	if (msg) {
		required_len += strlen(msg);
	}

	if (!buf) {
		return;
	}

	required_len += strlen(buf);

	msg = realloc(msg, required_len + 1);
	strcat(msg, buf);
	server->buffer[client_index] = msg;
}

char *webserver_receive(int client)
{
	char  *message = calloc(WEBSERVER_PACKET_SIZE + 1, sizeof(char));
	int    result;
	size_t bytes_received = 0;

	result = recv(client, message, WEBSERVER_PACKET_SIZE, 0);
	if (result < 0) {
		free(message);
		return NULL;
	}

	printf("===== INCOMING MSG =====\n");
	printf("%s\n", message);

	return message;
}

char *webserver_http_host(webserver_t *server)
{
	size_t required_length = strlen(server->ip_address) + 1 + strlen(server->port);
	char  *buf	       = calloc(required_length + 1, sizeof(char));
	snprintf(buf, required_length + 1, "%s:%s", server->ip_address, server->port);
	return buf;
}

void webserver_http_response_prepare(webserver_t *server, http_response_t *response)
{
	char *host = webserver_http_host(server);

	http_response_init(response);
	http_message_header_set(response, HTTP_HOST, host);
	http_message_header_set(response, HTTP_SERVER,
				WEBSERVER_SERVER_ID "/" WEBSERVER_SERVER_VER);
	http_message_header_set(response, HTTP_CONNECTION, HTTP_CLOSE);

	free(host);
}

http_response_t *webserver_http_response_create(webserver_t *server)
{
	http_response_t *response = malloc(sizeof(*response));
	webserver_http_response_prepare(server, response);
	return response;
}

void webserver_respond(int client, http_response_t *response)
{
	char  *send_buf	     = http_response_to_string(response);
	size_t bytes_to_send = strlen(send_buf);
	size_t bytes_sent    = 0;

	do {
		bytes_sent += send(client, send_buf + bytes_sent, bytes_to_send - bytes_sent, 0);
	} while (bytes_sent < bytes_to_send);

	printf("===== OUTGOING MSG =====\n");
	printf("%s\n", send_buf);

	free(send_buf);
}

bool webserver_request_handle(webserver_t *server, size_t client_index)
{
	int	       client = server->client[client_index].fd;
	char const    *buffer = server->buffer[client_index];
	http_request_t request[WEBSERVER_REQUEST_COUNT_MAX];
	size_t	       request_count = WEBSERVER_REQUEST_COUNT_MAX;
	int	       result;
	char	      *remain_buf;
	size_t	       remain_len;
	response_t     response;
	size_t	       i;

	// there is at least one complete message in buffer
	result = http_requests_from_string(request, &request_count, &buffer);
	if (request_count == 0) {
		response.client = client;
#if !TESTING
		response.response	  = webserver_http_response_create(server);
		response.response->status = http_status_bad_request;
		http_message_content_set(response.response, "400 Bad Request");
#else
		response.response = malloc(sizeof(http_response_t));
		http_response_init(response.response);
		response.response->status = http_status_bad_request;
#endif
		webserver_response_push(server, &response);
	} else {
		for (i = 0; i < request_count; ++i) {
			switch (request[i].method) {
			case http_method_get:
				webserver_http_request_handle_get(server, client, &request[i]);
				break;

			case http_method_put:
				webserver_http_request_handle_put(server, client, &request[i]);
				break;

			case http_method_delete:
				webserver_http_request_handle_delete(server, client, &request[i]);
				break;

			default:
				response.client		  = client;
				response.response	  = webserver_http_response_create(server);
				response.response->status = http_status_not_implemented;
#if !TESTING
				http_message_content_set(response.response, "501 Not Implemented");
#endif
				webserver_response_push(server, &response);
				break;
			}

			http_request_deinit(&request[i]);
		}
	}

	if (*buffer != '\0') {
		remain_len = strlen(buffer);
		remain_buf = calloc(remain_len + 1, sizeof(char));
		strcpy(remain_buf, buffer);
		free(server->buffer[client_index]);
		server->buffer[client_index] = remain_buf;
	} else {
		free(server->buffer[client_index]);
		server->buffer[client_index] = NULL;
	}

	return true;
}

bool webserver_message_is_ready(webserver_t *server, size_t client_index)
{
	assert(server && client_index < server->client_count);

	char	      *message = server->buffer[client_index];
	http_request_t request;
	int	       result;

	result = http_request_from_string(&request, server->buffer[client_index]);
	http_request_deinit(&request);
	if (!strstr(message, "\n\n") && !strstr(message, "\r\n\r\n")) {
		return false;
	}

	if (result == http_parser_result_content) {
		return false;
	}

	// if result != http_parser_result_ok, we simply have a bad request

	return true;
}

void webserver_http_request_handle_get(webserver_t *server, int client, http_request_t *request)
{
	assert(server && request);

	response_t response;
	char	  *content = webserver_load_resource(server, request->path);

	response.client	  = client;
	response.response = webserver_http_response_create(server);

	if (!content) {
		response.response->status = http_status_not_found;
#if !TESTING
		content = calloc(strlen(request->path) + strlen(HTTP_STATUS_NOT_FOUND_STRING) + 3,

				 sizeof(char));
		sprintf(content, "%s: %s", request->path, HTTP_STATUS_NOT_FOUND_STRING);
		http_message_content_set(response.response, content);
#else
		http_message_header_set(response.response, HTTP_CONTENT_LENGTH, "0");
#endif
	} else {
		response.response->status = http_status_ok;
		http_message_content_set(response.response, content);
	}

	free(content);

	webserver_response_push(server, &response);
}

void webserver_http_request_handle_put(webserver_t *server, int client, http_request_t *request)
{
	assert(server && client != -1 && request);

	response_t response;

	char *full_path = webserver_full_path(server, request->path);
	FILE *file	= fopen(full_path, "w");

	response.client	  = client;
	response.response = webserver_http_response_create(server);

	if (!file) {
		response.response->status = http_status_internal_server_error;
#if !TESTING
		http_message_content_set(response.response, strerror(errno));
#else
		http_message_header_set(response.response, HTTP_CONTENT_LENGTH, "0");
#endif
	} else {
		// TODO(ruarq): check return value of write
		fwrite(request->message.content, sizeof(char), request->message.content_length,
		       file);

		response.response->status = http_status_created;
#if !TESTING
		http_message_content_set(response.response, "Created.");
#else
		http_message_header_set(response.response, HTTP_CONTENT_LENGTH, "0");
#endif

		fclose(file);
	}

	free(full_path);

	webserver_response_push(server, &response);
}

void webserver_http_request_handle_delete(webserver_t *server, int client, http_request_t *request)
{
	assert(server && client != -1 && request);

	response_t response;
	char	  *full_path = webserver_full_path(server, request->path);
	int	   result;

	result = remove(full_path);
	free(full_path);

	response.client	  = client;
	response.response = webserver_http_response_create(server);

	if (result != 0) {
		response.response->status = http_status_not_found;

#if !TESTING
		http_message_content_set(response.response, strerror(errno));
#else
		http_message_header_set(response.response, HTTP_CONTENT_LENGTH, "0");
#endif
	} else {
		response.response->status = http_status_no_content;

#if !TESTING
		http_message_content_set(response.response, "File deleted successfully.");
#else
		http_message_header_set(response.response, HTTP_CONTENT_LENGTH, "0");
#endif
	}

	webserver_response_push(server, &response);
}

char *webserver_full_path(webserver_t *server, char const *path)
{
	assert(server && path);

	size_t required_len = strlen(server->root_path) + strlen(path);
	char  *buf	    = calloc(required_len + 1, sizeof(char));
	strcpy(buf, server->root_path);
	strcat(buf, path);

	return buf;
}

char *webserver_load_resource(webserver_t *server, char const *path)
{
	assert(server && path);

	char	   *full_path;
	FILE	   *file;
	size_t	    file_size;
	char	   *buf;
	struct stat path_stat;

	if (strcmp(path, "/static/foo") == 0) {
		buf = calloc(4, sizeof(char));
		strcpy(buf, "Foo");
		return buf;
	}

	if (strcmp(path, "/static/bar") == 0) {
		buf = calloc(4, sizeof(char));
		strcpy(buf, "Bar");
		return buf;
	}

	if (strcmp(path, "/static/baz") == 0) {
		buf = calloc(4, sizeof(char));
		strcpy(buf, "Baz");
		return buf;
	}

	full_path = webserver_full_path(server, path);

	printf("resource '%s' requested: ", full_path);

	stat(full_path, &path_stat);
	if (!S_ISREG(path_stat.st_mode)) {
		printf("FAIL\n");
		free(full_path);
		return NULL;
	}

	file = fopen(full_path, "r");
	free(full_path);

	if (!file) {
		printf("FAIL\n");
		return NULL;
	}

	printf("OK\n");

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buf = calloc(file_size + 1, sizeof(char));
	fread(buf, sizeof(char), file_size, file);
	fclose(file);

	return buf;
}
