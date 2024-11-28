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
#include <http/parser.h>

void response_init(response_t *response)
{
	response->client   = -1;
	response->response = NULL;
}

void response_deinit(response_t const *response)
{
	free(response->response);
}

bool webserver_init(webserver_t *server, char const *ip, char const *port, char const *root_path)
{
	int		 sock = -1;
	struct addrinfo	 hints;
	struct addrinfo *info;
	struct addrinfo *info_itr;
	void		*addr;
	char		*ipver;
	char		 ipstr[INET6_ADDRSTRLEN];
	int const	 yes = 1;

	server->running = false;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family	  = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int const result  = getaddrinfo(ip, port, &hints, &info);
	if (result == -1) {
		return false;
	}

	for (info_itr = info; info_itr; info_itr = info_itr->ai_next) {
		sock = socket(info_itr->ai_family, info_itr->ai_socktype, info_itr->ai_protocol);
		if (sock != -1) {
			break;
		}
	}

	if (info_itr == NULL) {
		freeaddrinfo(info);
		return false;
	}

	if (info_itr->ai_family == AF_INET) {
		// IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)info_itr->ai_addr;
		addr			 = &(ipv4->sin_addr);
		ipver			 = "IPv4";
	} else {
		// IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)info_itr->ai_addr;
		addr			  = &(ipv6->sin6_addr);
		ipver			  = "IPv6";
	}

	// convert the IP to a string and print it:
	inet_ntop(info_itr->ai_family, addr, ipstr, sizeof(ipstr));
	printf("webserver\n%s %s:%s\n", ipver, ipstr, port);

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

	for (size_t i = 0; i < WEBSERVER_CLIENT_COUNT_MAX; ++i) {
		server->buffer[i] = NULL;
	}

	// add the listening socket as a client to be able to poll events from it
	webserver_client_add(server, sock);

	server->running = true;

	return true;
}

void webserver_deinit(webserver_t const *server)
{
	assert(server);

	for (size_t i = 0; i < WEBSERVER_CLIENT_COUNT_MAX; ++i) {
		free(server->buffer[i]);
	}

	close(server->socket);
}

void webserver_run(webserver_t *server)
{
	struct sockaddr_storage new_client_addr;
	socklen_t		new_client_addr_len = sizeof(new_client_addr);

	response_t response;

	while (server->running) {
		for (size_t i = 0; i < server->response_count; ++i) {
			response = webserver_response_pop(server);
			webserver_respond(response.client, response.response);
			webserver_client_remove(server, response.client);
			response_deinit(&response);
		}

		int const result =
			poll(server->client, server->client_count, WEBSERVER_POLL_TIMEOUT);
		switch (result) {
		case -1:
			printf("Error: poll: %s\n", strerror(result));
			continue;

		case 0:
			continue;

		default:
			break;
		}

		if (server->client[0].revents & POLLIN) {
			printf("===== INCOMING CONN =====\n");
			int const new_client = accept(server->socket,
						      (struct sockaddr *)&new_client_addr,
						      &new_client_addr_len);
			if (new_client != -1) {
				printf("Success.\n");
				webserver_client_add(server, new_client);
			} else {
				printf("Error: %s\n", strerror(errno));
			}
			continue;
		}

		for (size_t i = 1; i < server->client_count; ++i) {
			struct pollfd const *pfd = &server->client[i];

			if (pfd->revents & POLLHUP) {
				printf("===== CLOSING CONN (%d) =====\n", pfd->fd);
				webserver_client_remove(server, i);
			} else if (pfd->revents & POLLIN) {
				webserver_client_receive(server, i);

				if (webserver_message_is_ready(server, i)) {
					webserver_request_handle(server, i);
				}
			}
		}
	}
}

void webserver_response_push(webserver_t *server, response_t const *response)
{
	if (server->response_count >= WEBSERVER_RESPONSE_COUNT_MAX) {
		printf("Error: max response count reached (%d)\n", WEBSERVER_RESPONSE_COUNT_MAX);
		return;
	}

	server->response[server->response_count++] = *response;
}

response_t webserver_response_pop(webserver_t *server)
{
	return server->response[--server->response_count];
}

bool webserver_client_add(webserver_t *server, int const client)
{
	assert(server);

	struct pollfd pfd;

	pfd.fd	    = client;
	pfd.events  = POLLIN;
	pfd.revents = 0;

	if (server->client_count >= WEBSERVER_CLIENT_COUNT_MAX) {
		return false;
	}

	server->client[server->client_count] = pfd;
	server->buffer[server->client_count] = NULL;
	++server->client_count;

	return true;
}

void webserver_client_remove(webserver_t *server, int const client)
{
	assert(server);

	for (size_t i = 0; i < server->client_count; ++i) {
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

void webserver_client_receive(webserver_t *server, size_t const client_index)
{
	int const   fd		 = server->client[client_index].fd;
	char const *buf		 = webserver_receive(fd);
	char	   *msg		 = server->buffer[client_index];
	size_t	    required_len = 0;

	if (msg) {
		required_len += strlen(msg);
	}

	if (buf == NULL) {
		return;
	}

	required_len += strlen(buf);

	size_t const old_len = msg ? strlen(msg) : 0;
	msg		     = realloc(msg, required_len + 1);
	memset(msg + old_len, '\0', required_len - old_len);
	strcat(msg, buf);
	server->buffer[client_index] = msg;
}

char *webserver_receive(int const client)
{
	char *message = calloc(WEBSERVER_PACKET_SIZE + 1, sizeof(char));

	int const result = recv(client, message, WEBSERVER_PACKET_SIZE, 0);
	if (result < 0) {
		free(message);
		return NULL;
	}

	printf("===== INCOMING MSG (%d) =====\n", client);
	printf("%s\n", message);

	return message;
}

char *webserver_http_host(webserver_t const *server)
{
	size_t const required_length = strlen(server->ip_address) + 1 + strlen(server->port);
	// ReSharper disable once CppDFAMemoryLeak
	char *buf = calloc(required_length + 1, sizeof(char));
	snprintf(buf, required_length + 1, "%s:%s", server->ip_address, server->port);
	return buf;
}

void webserver_http_response_prepare(webserver_t const *server, http_response_t *response)
{
	// ReSharper disable once CppDFAMemoryLeak
	char *host = webserver_http_host(server);

	http_response_init(response);
	http_message_header_set(response, HTTP_HOST, host);
	http_message_header_set(response, HTTP_SERVER,
				WEBSERVER_SERVER_ID "/" WEBSERVER_SERVER_VER);
	http_message_header_set(response, HTTP_CONNECTION, HTTP_CLOSE);

	free(host);
}

http_response_t *webserver_http_response_create(webserver_t const *server)
{
	// ReSharper disable once CppDFAMemoryLeak
	http_response_t *response = malloc(sizeof(*response));
	webserver_http_response_prepare(server, response);
	return response;
}

void webserver_respond(int const client, http_response_t const *response)
{
	char	    *send_buf	   = http_response_to_string(response);
	size_t const bytes_to_send = strlen(send_buf);
	size_t	     bytes_sent	   = 0;

	do {
		bytes_sent += send(client, send_buf + bytes_sent, bytes_to_send - bytes_sent, 0);
	} while (bytes_sent < bytes_to_send);

	printf("===== OUTGOING MSG (%d) =====\n", client);
	printf("%s\n", send_buf);

	free(send_buf);
}

bool webserver_request_handle(webserver_t *server, size_t const client_index)
{
	int const      client = server->client[client_index].fd;
	char const    *buffer = server->buffer[client_index];
	http_request_t request[WEBSERVER_REQUEST_COUNT_MAX];
	size_t	       request_count = WEBSERVER_REQUEST_COUNT_MAX;
	response_t     response;

	// there is at least one complete message in buffer
	http_parser_result_t const result =
		http_requests_from_string(request, &request_count, &buffer);

	if (result != http_parser_result_ok) {
		printf("Error: Invalid http request: %s\n", http_strerror(result));

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
		for (size_t i = 0; i < request_count; ++i) {
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
				response.client = client;
				// ReSharper disable once CppDFAMemoryLeak
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

	// There is remaining data in the buffer
	/*
	if (*buffer != '\0') {
		size_t const remain_len = strlen(buffer);
		char	    *remain_buf = calloc(remain_len + 1, sizeof(char));
		strcpy(remain_buf, buffer);
		printf("COPIED %zu bytes: %s\n", strlen(buffer), buffer);
		free(server->buffer[client_index]);
		server->buffer[client_index] = remain_buf;
	} else */

	free(server->buffer[client_index]);
	server->buffer[client_index] = NULL;

	return true;
}

bool webserver_message_is_ready(webserver_t const *server, size_t const client_index)
{
	assert(server && client_index < server->client_count);

	char const    *message = server->buffer[client_index];
	http_request_t request;
	int const      result = http_request_from_string(&request, server->buffer[client_index]);
	http_request_deinit(&request);
	if (strstr(message, "\n\n") == NULL && strstr(message, "\r\n\r\n") == NULL) {
		return false;
	}

	if (result == http_parser_result_content) {
		return false;
	}

	// if result != http_parser_result_ok, we simply have a bad request

	return true;
}

void webserver_http_request_handle_get(webserver_t *server, int const client,
				       http_request_t const *request)
{
	assert(server && request);

	response_t response;
	char	  *content = webserver_load_resource(server, request->path);

	response.client	  = client;
	response.response = webserver_http_response_create(server);

	if (content == NULL) {
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

void webserver_http_request_handle_put(webserver_t *server, int const client,
				       http_request_t const *request)
{
	assert(server && client != -1 && request);

	response_t response;

	char *full_path = webserver_full_path(server, request->path);
	FILE *file	= fopen(full_path, "w");

	response.client = client;
	// ReSharper disable once CppDFAMemoryLeak
	response.response = webserver_http_response_create(server);

	if (file == NULL) {
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

void webserver_http_request_handle_delete(webserver_t *server, int const client,
					  http_request_t const *request)
{
	assert(server && client != -1 && request);

	response_t response;
	char	  *full_path = webserver_full_path(server, request->path);

	int const result = remove(full_path);
	free(full_path);

	response.client = client;
	// ReSharper disable once CppDFAMemoryLeak
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

char *webserver_full_path(webserver_t const *server, char const *path)
{
	assert(server && path);

	size_t const required_len = strlen(server->root_path) + strlen(path);
	char	    *buf	  = calloc(required_len + 1, sizeof(char));
	strcpy(buf, server->root_path);
	strcat(buf, path);

	return buf;
}

char *webserver_load_resource(webserver_t const *server, char const *path)
{
	assert(server && path);

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

	char *full_path = webserver_full_path(server, path);

	printf("resource '%s' requested: ", full_path);

	stat(full_path, &path_stat);

	// ReSharper disable once CppRedundantComplexityInComparison
	if (!S_ISREG(path_stat.st_mode)) {
		printf("FAIL\n");
		free(full_path);
		return NULL;
	}

	FILE *file = fopen(full_path, "r");
	free(full_path);

	if (file == NULL) {
		printf("FAIL\n");
		return NULL;
	}

	printf("OK\n");

	fseek(file, 0, SEEK_END);
	size_t const file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buf = calloc(file_size + 1, sizeof(char));
	fread(buf, sizeof(char), file_size, file);
	fclose(file);

	return buf;
}
