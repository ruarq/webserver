#include <stdio.h>
#include <stdlib.h>

#include <http/http.h>

int main()
{
	char const *example_request = "GET /test HTTP/1.1\r\nContent-Type: text/plain\r\n"
				      "Content-Length: 10\r\n\r\n"
				      "1234567890";

	char const *example_response = "HTTP/1.1 200\r\n"
				       "Host: 127.0.0.1:1337\r\n\r\n";

	http_request_t request;
	char	      *buf;

	http_response_t response;

	http_request_init(&request);
	if (!http_request_from_string(&request, example_request)) {
		return 1;
	}
	buf = http_request_to_string(&request);
	printf("%s\n", buf);
	free(buf);
	http_request_deinit(&request);

	http_response_init(&response);
	if (!http_response_from_string(&response, example_response)) {
		return 1;
	}

	buf = http_response_to_string(&response);
	printf("%s\n", buf);
	free(buf);
	http_response_deinit(&response);

	return 0;
}
