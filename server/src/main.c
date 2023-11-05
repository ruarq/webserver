#include <stdio.h>
#include <stdlib.h>

#include <http/http.h>

int main()
{
	char const *example_request = "GET /test HTTP/1.1\r\nContent-Type: text/plain\r\n"
				      "Content-Length: 10\r\n\r\n"
				      "1234567890";

	http_request_t request;
	char	      *buf;

	http_request_init(&request);
	if (!http_request_from_string(&request, example_request)) {
		return 1;
	}
	buf = http_request_to_string(&request);
	printf("%s\n", buf);
	free(buf);
	http_request_deinit(&request);

	return 0;
}
