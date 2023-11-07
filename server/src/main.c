#include <stdio.h>
#include <stdlib.h>

#include "webserver.h"

int main(int argc, char **argv)
{
	webserver_t server;
	char const *ip	 = "127.0.0.1";
	char const *port = "1337";

	if (argc == 3) {
		ip   = argv[1];
		port = argv[2];
	}

	if (!webserver_init(&server, ip, port)) {
		return false;
	}

	webserver_run(&server);
	webserver_deinit(&server);

	return 0;
}
