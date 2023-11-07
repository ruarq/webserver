#include "webserver.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

bool webserver_init(webserver_t *server, char const *ip, char const *port)
{
	int		 sock;
	int		 result;
	struct addrinfo	 hints;
	struct addrinfo *info;
	struct addrinfo *info_itr;

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

	if (bind(sock, info_itr->ai_addr, info_itr->ai_addrlen) == -1) {
		return false;
	}

	freeaddrinfo(info);

	if (listen(sock, WEBSERVER_BACKLOG) == -1) {
		return false;
	}

	server->ip_address = ip;
	server->port	   = port;
	server->socket	   = sock;

	return true;
}

void webserver_deinit(webserver_t *server)
{
	close(server->socket);
}

void webserver_run(webserver_t *server)
{
	int			client;
	struct sockaddr_storage client_addr;
	socklen_t		client_addrlen;
	char			buf[WEBSERVER_MESSAGE_SIZE];
	size_t			bytes_received;
	http_request_t		request;
	http_response_t		response;
	int			parse_result;

	while (true) {
		client_addrlen = sizeof(client_addr);
		client = accept(server->socket, (struct sockaddr *)&client_addr, &client_addrlen);
		if (client == -1) {
			continue;
		}

		bytes_received	    = recv(client, buf, WEBSERVER_MESSAGE_SIZE, 0);
		buf[bytes_received] = '\0';

		printf("===== INCOMING MSG =====\n");
		printf("%s\n", buf);
		printf("========================\n");

		parse_result = http_request_from_string(&request, buf);
		if (parse_result != 0) {
			webserver_http_response_prepare(server, &response);
			response.status = http_status_bad_request;
			http_message_content_set(&response, http_strerror(parse_result));
			webserver_respond(client, &response);
			close(client);
			http_response_deinit(&response);
			continue;
		}

		webserver_http_request_handle(server, client, &request);
		http_request_deinit(&request);
	}
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
	http_message_header_set(response, HTTP_SERVER, WEBSERVER_SERVER);
	http_message_header_set(response, HTTP_CONNECTION, HTTP_CLOSE);

	free(host);
}

void webserver_http_request_handle(webserver_t *server, int client, http_request_t *request)
{
	http_response_t response;

	switch (request->method) {
	default:
		webserver_http_response_prepare(server, &response);
		response.status = http_status_not_implemented;
		http_message_content_set(&response, http_method_to_string(request->method));
		webserver_respond(client, &response);
		break;
	}

	http_response_deinit(&response);
}

void webserver_respond(int client, http_response_t *response)
{
	char  *send_buf	     = http_response_to_string(response);
	size_t bytes_to_send = strlen(send_buf);
	size_t bytes_sent    = 0;

	printf("===== OUTGOING MSG =====\n");
	printf("%s\n", send_buf);
	printf("========================\n");

	do {
		bytes_sent += send(client, send_buf + bytes_sent, bytes_to_send - bytes_sent, 0);
	} while (bytes_sent < bytes_to_send);

	free(send_buf);
}
