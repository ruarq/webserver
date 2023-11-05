#include "request.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

http_method_t http_method_from_string(char const *str)
{
	if (strcmp(str, HTTP_METHOD_GET_STRING) == 0) {
		return http_method_get;
	} else if (strcmp(str, HTTP_METHOD_PUT_STRING) == 0) {
		return http_method_put;
	} else if (strcmp(str, HTTP_METHOD_DELETE_STRING) == 0) {
		return http_method_delete;
	}

	return http_method_undef;
}

char const *http_method_to_string(http_method_t method)
{
	switch (method) {
	case http_method_get:
		return HTTP_METHOD_GET_STRING;

	case http_method_put:
		return HTTP_METHOD_PUT_STRING;

	case http_method_delete:
		return HTTP_METHOD_DELETE_STRING;

	default:
	case http_method_undef:
		return "UNDEF";
	}
}

void http_request_init(http_request_t *request)
{
	http_message_init(&request->message);
	request->method	      = http_method_undef;
	request->path	      = NULL;
	request->http_version = NULL;
}

void http_request_deinit(http_request_t *request)
{
	http_message_deinit(&request->message);
	free(request->path);
	free(request->http_version);
}

void http_request_set_path(http_request_t *request, char const *path)
{
	size_t path_len = strlen(path);

	free(request->path);
	request->path = calloc(path_len + 1, sizeof(char));
	strncpy(request->path, path, path_len);
}

void http_request_set_http_version(http_request_t *request, char const *http_version)
{
	size_t http_version_len = strlen(http_version);

	free(request->http_version);
	request->http_version = calloc(http_version_len + 1, sizeof(char));
	strncpy(request->http_version, http_version, http_version_len);
}

bool http_request_from_string(http_request_t *request, char const *str)
{
	size_t	    i = 0;
	char	    buf[HTTP_REQUEST_BUFFER_SIZE];
	char const *method;
	char const *path;
	char const *http_version;

	method = buf;
	while (*str != ' ') {
		if (*str == '\0') {
			return false;
		}

		buf[i++] = *str++;
	}
	++str;
	buf[i++] = '\0';

	path = buf + i;
	while (*str != ' ') {
		if (*str == '\0') {
			return false;
		}

		buf[i++] = *str++;
	}

	++str;
	buf[i++] = '\0';

	http_version = buf + i;
	while (strncmp(str, "\r\n", 2) != 0) {
		if (*str == '\0') {
			return false;
		}

		buf[i++] = *str++;
	}
	str += 2;
	buf[i++] = '\0';

	if (!http_message_from_string(&request->message, str)) {
		return false;
	}

	if (strlen(method) == 0 || strlen(path) == 0 || strlen(http_version) == 0) {
		return false;
	}

	if (strcmp(HTTP_PROTOCOL_VERSION, http_version) != 0) {
		return false;
	}

	request->method = http_method_from_string(method);
	if (request->method == http_method_undef) {
		return false;
	}

	http_request_set_path(request, path);
	http_request_set_http_version(request, http_version);
	return true;
}

char *http_request_to_string(http_request_t *request)
{
	char	   *message_string  = http_message_to_string(&request->message);
	char const *method_string   = http_method_to_string(request->method);
	size_t	    required_length = strlen(message_string) + strlen(request->path) +
				 strlen(request->http_version) + strlen(method_string);
	char *out = calloc(required_length + 1, sizeof(char));
	snprintf(out, required_length, "%s %s %s\r\n", method_string, request->path,
		 request->http_version);
	strcat(out, message_string);
	free(message_string);
	return out;
}
