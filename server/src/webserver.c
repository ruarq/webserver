#include "webserver.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <http/http.h>
#include <string.h>

bool webserver_init(webserver_t *server, char const *ip_addr, char const *port)
{
	struct addrinfo hints, *result;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family	  = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(ip_addr, port, &hints, &result) != 0) {
		return false;
	}

	server->socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server->socket == -1) {
		return false;
	}

	if (bind(server->socket, result->ai_addr, result->ai_addrlen) == -1) {
		return false;
	}

	return true;
}

void webserver_deinit(webserver_t *server)
{
	close(server->socket);
}

void webserver_run(webserver_t *server)
{
}
