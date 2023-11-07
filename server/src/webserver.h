#ifndef SERVER_WEBSERVER_H
#define SERVER_WEBSERVER_H

#include <http/http.h>

#define WEBSERVER_SERVER       "RN/0.01"
#define WEBSERVER_BACKLOG      10
#define WEBSERVER_MESSAGE_SIZE 1024

typedef struct {
	char const *ip_address;
	char const *port;
	int	    socket;
} webserver_t;

bool webserver_init(webserver_t *server, char const *ip, char const *port);
void webserver_deinit(webserver_t *server);

void webserver_run(webserver_t *server);

char *webserver_http_host(webserver_t *server);
void  webserver_http_response_prepare(webserver_t *server, http_response_t *response);
void  webserver_http_request_handle(webserver_t *server, int client, http_request_t *request);
void  webserver_respond(int client, http_response_t *response);

#endif
