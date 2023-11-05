#include "header.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void http_header_init(http_header_t *header)
{
	header->key   = NULL;
	header->value = NULL;
}

void http_header_deinit(http_header_t *header)
{
	free(header->key);
	free(header->value);
}

void http_header_set_key(http_header_t *header, char const *key)
{
	size_t key_len = strlen(key);

	free(header->key);
	header->key = calloc(key_len + 1, sizeof(char));
	strncpy(header->key, key, key_len);
}

void http_header_set_value(http_header_t *header, char const *value)
{
	size_t value_len = strlen(value);

	free(header->value);
	header->value = calloc(value_len + 1, sizeof(char));
	strncpy(header->value, value, value_len);
}

void http_header_set(http_header_t *header, char const *key, char const *value)
{
	http_header_set_key(header, key);
	http_header_set_value(header, value);
}
