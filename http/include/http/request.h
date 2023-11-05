#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdbool.h>

#include "message.h"

#define HTTP_METHOD_GET_STRING	  "GET"
#define HTTP_METHOD_PUT_STRING	  "PUT"
#define HTTP_METHOD_DELETE_STRING "DELETE"

#define HTTP_REQUEST_BUFFER_SIZE 8192

typedef enum {
	http_method_get,
	http_method_put,
	http_method_delete,
	http_method_undef,
} http_method_t;

http_method_t http_method_from_string(char const *str);
char const   *http_method_to_string(http_method_t method);

typedef struct {
	http_message_t message;
	http_method_t  method;
	char	      *path;
	char	      *http_version;
} http_request_t;

void http_request_init(http_request_t *request);
void http_request_deinit(http_request_t *request);

void http_request_set_path(http_request_t *request, char const *path);
void http_request_set_http_version(http_request_t *request, char const *http_version);

bool  http_request_from_string(http_request_t *request, char const *str);
char *http_request_to_string(http_request_t *request);

#endif
