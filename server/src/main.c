#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "webserver.h"

int main(int const argc, char **argv) {
	char working_dir[PATH_MAX + 1];

	webserver_t server;
	char const *ip = "127.0.0.1";
	char const *port = "1337";

	if (argc >= 3) {
		ip = argv[1];
		port = argv[2];
	}

	if (argc >= 4) {
		strncpy(working_dir, argv[3], PATH_MAX);
	}

	if (getcwd(working_dir, PATH_MAX) == NULL) {
		printf("%s\n", strerror(errno));
		return 1;
	}

	strncat(working_dir, "/root", PATH_MAX);

	if (!webserver_init(&server, ip, port, working_dir)) {
		printf("%s\n", strerror(errno));
		return 1;
	}

	webserver_run(&server);
	webserver_deinit(&server);

	return 0;
}
