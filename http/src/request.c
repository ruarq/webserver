#include "request.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "parser.h"

char const *http_method_to_string_map[] = {
	[http_method_connect] = HTTP_METHOD_CONNECT_STRING,
	[http_method_delete]  = HTTP_METHOD_DELETE_STRING,
	[http_method_head]    = HTTP_METHOD_HEAD_STRING,
	[http_method_get]     = HTTP_METHOD_GET_STRING,
	[http_method_options] = HTTP_METHOD_OPTIONS_STRING,
	[http_method_patch]   = HTTP_METHOD_PATCH_STRING,
	[http_method_post]    = HTTP_METHOD_POST_STRING,
	[http_method_put]     = HTTP_METHOD_PUT_STRING,
	[http_method_trace]   = HTTP_METHOD_TRACE_STRING,
};

char const *http_method_to_string(http_method_t const method)
{
	if (method >= http_method_count) {
		return NULL;
	}

	return http_method_to_string_map[method];
}

void http_request_init(http_request_t *request)
{
	assert(request);

	http_message_init(request);
	request->path = NULL;
}

void http_request_deinit(http_request_t *request)
{
	assert(request);

	http_message_deinit(request);
	free(request->path);
}

void http_request_path_set(http_request_t *request, char const *path)
{
	assert(request && path);

	http_request_path_set_n(request, path, strlen(path));
}

void http_request_path_set_n(http_request_t *request, char const *path, size_t n)
{
	assert(request && path);

	free(request->path);
	request->path = calloc(n + 1, sizeof(char));
	strncpy(request->path, path, n);
}

int http_request_from_string(http_request_t *request, char const *str)
{
	assert(request && str);

	http_parser_t parser;

	http_parser_init(&parser);
	parser.source			  = str;
	parser.source_len		  = strlen(str);
	http_parser_result_t const result = http_parser_parse_request(&parser, request);

	return result;
}

char *http_request_to_string(http_request_t const *request)
{
	assert(request);

	char const  *method_string   = http_method_to_string(request->method);
	char	    *version_string  = http_version_to_string(&request->message.version);
	char	    *message_string  = http_message_to_string(&request->message);
	size_t const required_length = strlen(method_string) + 1 + strlen(request->path) + 1 +
				       strlen(version_string) + 2 + strlen(message_string);
	char *buf = calloc(required_length + 1, sizeof(char));

	snprintf(buf, required_length + 1, "%s %s %s\r\n%s", method_string, request->path,
		 version_string, message_string);

	free(version_string);
	free(message_string);

	return buf;
}

int http_requests_from_string(http_request_t *requests, size_t *n, char const **str)
{
	http_parser_t parser;

	http_parser_init(&parser);
	parser.source			  = *str;
	parser.source_len		  = strlen(*str);
	http_parser_result_t const result = http_parser_parse_requests(&parser, requests, n);
	*str				  = parser.source + parser.current;

	return result;
}
