#include "webserver.h"

int main()
{
	webserver_t server;
	if (!webserver_init(&server, "127.0.0.1", "1337")) {
		return 1;
	}
	webserver_run(&server);
	webserver_deinit(&server);
	return 0;
}
