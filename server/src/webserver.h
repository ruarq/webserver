#ifndef SERVER_WEBSERVER_H
#define SERVER_WEBSERVER_H

#include <http/http.h>
#include <stdbool.h>
#include <poll.h>

#define WEBSERVER_SERVER_ID	     "RQ"
#define WEBSERVER_SERVER_VER	     "0.1.0"
#define WEBSERVER_BACKLOG	     10
#define WEBSERVER_PACKET_SIZE	     8192
#define WEBSERVER_MESSAGE_SIZE	     (1 << 14)
#define WEBSERVER_REQUEST_COUNT_MAX  32
#define WEBSERVER_RESPONSE_COUNT_MAX 32
#define WEBSERVER_POLL_TIMEOUT	     1000
#define WEBSERVER_CLIENT_COUNT_MAX   (32 + 1)
#define TESTING			     0

typedef struct {
	int	       client;
	http_request_t request;
} message_t;

typedef struct {
	int		 client;
	http_response_t *response;
} response_t;

void response_init(response_t *response);

void response_deinit(response_t const *response);

typedef struct {
	char const *ip_address;
	char const *port;
	int	    socket;

	char const *root_path;

	struct pollfd client[WEBSERVER_CLIENT_COUNT_MAX];
	char	     *buffer[WEBSERVER_CLIENT_COUNT_MAX];
	size_t	      client_count;

	response_t response[WEBSERVER_RESPONSE_COUNT_MAX];
	size_t	   response_count;

	bool running; ///< Just to get rid of the endless-loop warning
} webserver_t;

bool webserver_init(webserver_t *server, char const *ip, char const *port, char const *root_path);

void webserver_deinit(webserver_t const *server);

void webserver_run(webserver_t *server);

void webserver_response_push(webserver_t *server, response_t const *response);

response_t webserver_response_pop(webserver_t *server);

bool webserver_client_add(webserver_t *server, int client);

void webserver_client_remove(webserver_t *server, int client);

void webserver_client_receive(webserver_t *server, size_t client_index);

char *webserver_receive(int client);

char *webserver_http_host(webserver_t const *server);

void webserver_http_response_prepare(webserver_t const *server, http_response_t *response);

http_response_t *webserver_http_response_create(webserver_t const *server);

void webserver_respond(int client, http_response_t *response);

bool webserver_request_handle(webserver_t *server, size_t client_index);

bool webserver_message_is_ready(webserver_t const *server, size_t client_index);

void webserver_http_request_handle_get(webserver_t *server, int client, http_request_t *request);

void webserver_http_request_handle_put(webserver_t *server, int client,
				       http_request_t const *request);

void webserver_http_request_handle_delete(webserver_t *server, int client,
					  http_request_t const *request);

char *webserver_full_path(webserver_t const *server, char const *path);

char *webserver_load_resource(webserver_t const *server, char const *path);

#endif
