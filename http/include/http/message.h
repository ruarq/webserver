#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <stddef.h>
#include <stdbool.h>

#include "header.h"

#define HTTP_MESSAGE_HEADER_COUNT 100

typedef struct {
	http_header_t header[HTTP_MESSAGE_HEADER_COUNT];
	size_t	      header_count;
	char	     *content;
	size_t	      content_length;
} http_message_t;

void http_message_init(http_message_t *message);
void http_message_deinit(http_message_t *message);

bool	    http_message_header_set(http_message_t *message, char const *key, char const *value);
char const *http_message_header_get(http_message_t *message, char const *key);

bool http_message_set_content(http_message_t *message, char const *content_type,
			      char const *content);

bool  http_message_from_string(http_message_t *message, char const *str);
char *http_message_to_string(http_message_t *message);

#endif
