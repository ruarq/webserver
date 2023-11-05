#include "response.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char const *http_status_to_string(http_status_t status)
{
	switch (status) {
	case http_status_ok:
		return HTTP_STATUS_OK_STRING;

	case http_status_created:
		return HTTP_STATUS_CREATED_STRING;

	case http_status_no_content:
		return HTTP_STATUS_NO_CONTENT_STRING;

	case http_status_bad_request:
		return HTTP_STATUS_BAD_REQUEST_STRING;

	case http_status_forbidden:
		return HTTP_STATUS_FORBIDDEN_STRING;

	case http_status_not_found:
		return HTTP_STATUS_NOT_FOUND_STRING;

	default:
		return NULL;
	}
}

void http_response_init(http_response_t *response)
{
	http_message_init(&response->message);
	response->http_version = NULL;
}

void http_response_deinit(http_response_t *response)
{
	http_message_deinit(&response->message);
	free(response->http_version);
}

void http_response_set_http_version(http_response_t *response, char const *http_version)
{
	size_t http_version_len = strlen(http_version);

	free(response->http_version);
	response->http_version = calloc(http_version_len + 1, sizeof(char));
	strncpy(response->http_version, http_version, http_version_len);
}

bool http_response_from_string(http_response_t *response, char const *str)
{
	size_t	    i = 0;
	char	    buf[HTTP_RESPONSE_BUFFER_SIZE];
	char const *http_version;
	char const *status;

	http_version = buf;

	// TODO(ruarq): write a function for this shit
	while (*str != ' ') {
		if (*str == '\0') {
			return false;
		}
		buf[i++] = *str++;
	}
	++str;
	buf[i++] = '\0';

	status = buf + i;
	while (*str != ' ' && strncmp(str, "\r\n", 2) != 0) {
		if (*str == '\0') {
			return false;
		}
		buf[i++] = *str++;
	}

	buf[i++] = '\0';

	// skip status message if present
	if (*str == ' ') {
		while (strncmp(str, "\r\n", 2) != 0) {
			if (*str == '\0') {
				return false;
			}
			++str;
		}
	}

	str += 2;

	if (strlen(http_version) == 0 || strlen(status) == 0) {
		return false;
	}

	if (!http_message_from_string(&response->message, str)) {
		return false;
	}
	http_response_set_http_version(response, http_version);
	response->status = (http_status_t)atoi(status);
	if (http_status_to_string(response->status) == NULL) {
		return false;
	}

	return true;
}

char *http_response_to_string(http_response_t *response)
{
	char	   *message_string  = http_message_to_string(&response->message);
	char const *status_string   = http_status_to_string(response->status);
	size_t	    required_length = strlen(message_string) + strlen(status_string) +
				 strlen(response->http_version) + 2 + (sizeof(size_t) * 8 + 1);
	char *out = calloc(required_length + 1, sizeof(char));

	snprintf(out, required_length, "%s %d %s\r\n%s", response->http_version, response->status,
		 status_string, message_string);

	free(message_string);
	return out;
}
