#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stdbool.h>

typedef struct {
	int socket;
} webserver_t;

bool webserver_init(webserver_t *server, char const *ip_addr, char const *port);
void webserver_deinit(webserver_t *server);

void webserver_run(webserver_t *server);

#endif
