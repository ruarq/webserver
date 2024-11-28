#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <stddef.h>

#define HTTP_TEXT_PLAIN "text/plain"
#define HTTP_TEXT_HTML	"text/html"
#define HTTP_CLOSE	"close"

#define HTTP_CONTENT_TYPE   "Content-Type"
#define HTTP_CONTENT_LENGTH "Content-Length"
#define HTTP_HOST	    "Host"
#define HTTP_SERVER	    "Server"
#define HTTP_CONNECTION	    "Connection"

typedef struct {
	char *key;
	char *value;
} http_header_t;


void http_header_init(http_header_t *header);

void http_header_deinit(http_header_t const *header);

void http_header_set(http_header_t *header, char const *key, char const *value);

void http_header_set_n(http_header_t *header, char const *key, size_t key_n, char const *value,
                       size_t value_n);

void http_header_key_set(http_header_t *header, char const *key);

void http_header_key_set_n(http_header_t *header, char const *key, size_t n);

void http_header_value_set(http_header_t *header, char const *value);

void http_header_value_set_n(http_header_t *header, char const *value, size_t n);

#endif
