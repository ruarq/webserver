#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void http_message_init(http_message_t *message)
{
	message->header_count	= 0;
	message->content	= NULL;
	message->content_length = 0;
}

void http_message_deinit(http_message_t *message)
{
	size_t i;

	for (i = 0; i < message->header_count; ++i) {
		http_header_deinit(&message->header[i]);
	}

	free(message->content);
}

bool http_message_header_set(http_message_t *message, char const *key, char const *value)
{
	http_header_t *header;
	size_t	       i;

	if (message->header_count >= HTTP_MESSAGE_HEADER_COUNT) {
		return false;
	}

	for (i = 0; i < message->header_count; ++i) {
		if (strcmp(message->header[i].key, key) == 0) {
			http_header_set_value(&message->header[i], value);
			return true;
		}
	}

	header = &message->header[message->header_count++];
	http_header_init(header);
	http_header_set(header, key, value);
	return true;
}

char const *http_message_header_get(http_message_t *message, char const *key)
{
	size_t i;

	for (i = 0; i < message->header_count; ++i) {
		if (strcmp(message->header[i].key, key) == 0) {
			return message->header[i].value;
		}
	}

	return NULL;
}

bool http_message_content_set(http_message_t *message, char const *content_type,
			      char const *content)
{
	free(message->content);
	message->content_length = strlen(content);
	message->content	= calloc(message->content_length, sizeof(char));
	strncpy(message->content, content, message->content_length);

	// https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-ultoa-convert-unsigned-long-into-string
	char buf[sizeof(size_t) * 8 + 1];
	sprintf(buf, "%lu", message->content_length);
	printf("Setting: %s\n", buf);

	bool result;
	result = http_message_header_set(message, HTTP_CONTENT_TYPE, content_type);
	result = result && http_message_header_set(message, HTTP_CONTENT_LENGTH, buf);
	return result;
}

bool http_message_from_string(http_message_t *message, char const *str)
{
	size_t	    i;
	char	    buf[HTTP_HEADER_SIZE];
	char const *key;
	char const *value;
	char const *content_type;
	char const *content_length;

	http_message_deinit(message);
	http_message_init(message);

	while (strncmp(str, "\r\n", 2) != 0) {
		if (*str == '\0') {
			return false;
		}

		i   = 0;
		key = buf;
		while (strncmp(str, ": ", 2) != 0) {
			if (*str == '\0' || i >= HTTP_HEADER_SIZE) {
				return false;
			}
			buf[i++] = *str++;
		}

		buf[i++] = '\0';
		str += 2;

		value = buf + i;

		while (strncmp(str, "\r\n", 2) != 0) {
			if (*str == '\0' || i >= HTTP_HEADER_SIZE) {
				return false;
			}
			buf[i++] = *str++;
		}

		buf[i++] = '\0';
		str += 2;

		if (!http_message_header_set(message, key, value)) {
			return false;
		}
	}

	str += 2;
	if (*str == '\0') {
		return true;
	}

	content_type = http_message_header_get(message, HTTP_CONTENT_TYPE);
	if (content_type == NULL) {
		return false;
	}

	content_length = http_message_header_get(message, HTTP_CONTENT_LENGTH);
	if (content_length == NULL) {
		return false;
	}

	message->content_length = atoi(content_length);
	message->content	= calloc(message->content_length + 1, sizeof(char));

	for (i = 0; i < message->content_length; ++i) {
		message->content[i] = *str++;
	}

	return true;
}

char *http_message_to_string(http_message_t *message)
{
	char   buf[HTTP_HEADER_SIZE + 2];
	char  *out;
	size_t buffer_size =
		message->header_count * (HTTP_HEADER_SIZE + 2) + 4 + message->content_length + 1;
	size_t i;

	out = calloc(buffer_size, sizeof(char));
	for (i = 0; i < message->header_count; ++i) {
		sprintf(buf, "%s: %s\r\n", message->header[i].key, message->header[i].value);
		strcat(out, buf);
	}

	strcat(out, "\r\n");
	strncat(out, message->content, message->content_length);

	return out;
}
