#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "header.h"
#include "version.h"

#define HTTP_MESSAGE_HEADER_COUNT_MAX 100

typedef struct {
	http_header_t header[HTTP_MESSAGE_HEADER_COUNT_MAX];
	size_t header_count;
	uint8_t *content;
	size_t content_length;

	http_version_t version;
} http_message_t;

void http_message_init(void *message);

void http_message_deinit(void *message);

void http_message_content_set(void *message, char const *str);

void http_message_content_set_n(void *message, uint8_t const *data, size_t n);

bool http_message_header_set(void *message, char const *key, char const *value);

bool http_message_header_set_n(void *message, char const *key, size_t key_n, char const *value,
                               size_t value_n);

char const *http_message_header_get(void const *message, char const *key);

bool http_message_header_remove(void *message, char const *key);

int http_message_from_string(http_message_t *message, char const *str);

char *http_message_to_string(http_message_t const *message);

#endif

