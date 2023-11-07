#include "header.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void http_header_init(http_header_t *header)
{
	assert(header);

	header->key   = NULL;
	header->value = NULL;
}

void http_header_deinit(http_header_t *header)
{
	assert(header);

	free(header->key);
	free(header->value);
}

void http_header_set(http_header_t *header, char const *key, char const *value)
{
	assert(header && key && value);

	http_header_key_set(header, key);
	http_header_value_set(header, value);
}

void http_header_set_n(http_header_t *header, char const *key, size_t key_n, char const *value,
		       size_t value_n)
{
	assert(header && key && value);

	http_header_key_set_n(header, key, key_n);
	http_header_value_set_n(header, value, value_n);
}

void http_header_key_set(http_header_t *header, char const *key)
{
	assert(header && key);

	http_header_key_set_n(header, key, strlen(key));
}

void http_header_key_set_n(http_header_t *header, char const *key, size_t n)
{
	assert(header && key);

	free(header->key);
	header->key = calloc(n + 1, sizeof(char));
	strncpy(header->key, key, n);
}

void http_header_value_set(http_header_t *header, char const *value)
{
	assert(header && value);

	http_header_value_set_n(header, value, strlen(value));
}

void http_header_value_set_n(http_header_t *header, char const *value, size_t n)
{
	assert(header && value);

	free(header->value);
	header->value = calloc(n + 1, sizeof(char));
	strncpy(header->value, value, n);
}
