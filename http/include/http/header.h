#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#define HTTP_HEADER_SIZE 8192

#define HTTP_CONTENT_TYPE   "Content-Type"
#define HTTP_CONTENT_LENGTH "Content-Length"

#define HTTP_TEXT_HTML	"text/html"
#define HTTP_TEXT_PLAIN "text/plain"

typedef struct {
	char *key;
	char *value;
} http_header_t;

void http_header_init(http_header_t *header);
void http_header_deinit(http_header_t *header);

void http_header_set_key(http_header_t *header, char const *key);
void http_header_set_value(http_header_t *header, char const *value);
void http_header_set(http_header_t *header, char const *key, char const *value);

#endif
