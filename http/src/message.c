#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "parser.h"
#include "config.h"

static void _http_message_init(http_message_t *message)
{
	assert(message);

	message->header_count	= 0;
	message->content	= NULL;
	message->content_length = 0;

	http_version_init(&message->version, HTTP_VERSION_MAJOR, HTTP_VERSION_MINOR);
}

void http_message_init(void *message)
{
	_http_message_init((http_message_t *)message);
}

static void _http_message_deinit(http_message_t *message)
{
	assert(message);

	size_t i;

	for (i = 0; i < message->header_count; ++i) {
		http_header_deinit(&message->header[i]);
	}
	free(message->content);
}

void http_message_deinit(void *message)
{
	_http_message_deinit((http_message_t *)message);
}

static void _http_message_content_set(http_message_t *message, char const *str)
{
	assert(message && str);

	char buf[sizeof(size_t) * 8 + 1];

	free(message->content);
	message->content_length = strlen(str);
	message->content	= calloc(message->content_length, sizeof(char));
	memcpy(message->content, str, message->content_length);

	snprintf(buf, sizeof(size_t) * 8 + 1, "%zu", message->content_length);
	http_message_header_set(message, HTTP_CONTENT_LENGTH, buf);
}

void http_message_content_set(void *message, char const *str)
{
	_http_message_content_set((http_message_t *)message, str);
}

static bool _http_message_header_set_n(http_message_t *message, char const *key, size_t key_n,
				       char const *value, size_t value_n)
{
	assert(message && key && value);

	http_header_t header;
	size_t	      i;

	for (i = 0; i < message->header_count; ++i) {
		if (strcasecmp(message->header[i].key, key) == 0) {
			http_header_value_set(&message->header[i], value);
			return true;
		}
	}

	if (message->header_count >= HTTP_MESSAGE_HEADER_COUNT_MAX) {
		return false;
	}

	http_header_init(&header);
	http_header_set_n(&header, key, key_n, value, value_n);
	message->header[message->header_count++] = header;
	return true;
}

bool http_message_header_set(void *message, char const *key, char const *value)
{
	return http_message_header_set_n(message, key, strlen(key), value, strlen(value));
}

bool http_message_header_set_n(void *message, char const *key, size_t key_n, char const *value,
			       size_t value_n)
{
	return _http_message_header_set_n((http_message_t *)message, key, key_n, value, value_n);
}

static char const *_http_message_header_get(http_message_t *message, char const *key)
{
	assert(message && key);

	size_t i;

	for (i = 0; i < message->header_count; ++i) {
		if (strcasecmp(message->header[i].key, key) == 0) {
			return message->header[i].value;
		}
	}

	return NULL;
}

char const *http_message_header_get(void *message, char const *key)
{
	return _http_message_header_get((http_message_t *)message, key);
}

static bool _http_message_header_remove(http_message_t *message, char const *key)
{
	assert(message && key);

	size_t i;

	if (message->header_count == 0) {
		return false;
	} /* else if (message->header_count == 1) {
		--message->header_count;
		return true;
	} */

	for (i = 0; i < message->header_count; ++i) {
		if (strcasecmp(message->header[i].key, key) == 0) {
			break;
		}
	}

	if (i == message->header_count) {
		return false;
	}

	--message->header_count;
	message->header[i] = message->header[message->header_count];
	return true;
}

bool http_message_header_remove(void *message, char const *key)
{
	return _http_message_header_remove((http_message_t *)message, key);
}

int http_message_from_string(http_message_t *message, char const *str)
{
	assert(message && str);

	http_parser_t parser;

	http_parser_init(&parser);
	parser.source	  = str;
	parser.source_len = strlen(str);
	return http_parser_parse_message(&parser, message);
}

char *http_message_to_string(http_message_t *message)
{
	assert(message);

	char *buf;
	/* key: value\r\n */
	/* \r\n<content>\0 */
	char  *header_strings[HTTP_MESSAGE_HEADER_COUNT_MAX];
	size_t required_len = 0;
	size_t i;
	size_t header_len;
	size_t key_len;
	size_t value_len;

	for (i = 0; i < message->header_count; ++i) {
		key_len	  = strlen(message->header[i].key);
		value_len = strlen(message->header[i].value);

		header_len	  = key_len + 2 + value_len + 2;
		header_strings[i] = calloc(header_len + 1, sizeof(char));
		snprintf(header_strings[i], header_len + 1, "%s: %s\r\n", message->header[i].key,
			 message->header[i].value);
		required_len += header_len;
	}

	required_len += 2 + message->content_length;

	buf = calloc(required_len + 1, sizeof(char));

	for (i = 0; i < message->header_count; ++i) {
		strcat(buf, header_strings[i]);
		free(header_strings[i]);
	}

	strcat(buf, "\r\n");

	if (message->content) {
		strncat(buf, (char const *)message->content, message->content_length);
	}

	return buf;
}
