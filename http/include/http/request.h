#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "message.h"

#define HTTP_METHOD_CONNECT_STRING "CONNECT"
#define HTTP_METHOD_DELETE_STRING  "DELETE"
#define HTTP_METHOD_HEAD_STRING	   "HEAD"
#define HTTP_METHOD_GET_STRING	   "GET"
#define HTTP_METHOD_OPTIONS_STRING "OPTIONS"
#define HTTP_METHOD_PATCH_STRING   "PATCH"
#define HTTP_METHOD_POST_STRING	   "POST"
#define HTTP_METHOD_PUT_STRING	   "PUT"
#define HTTP_METHOD_TRACE_STRING   "TRACE"

typedef enum {
	http_method_connect,
	http_method_delete,
	http_method_head,
	http_method_get,
	http_method_options,
	http_method_patch,
	http_method_post,
	http_method_put,
	http_method_trace,
	http_method_count,
} http_method_t;

char const *http_method_to_string(http_method_t method);

typedef struct {
	http_message_t message;
	http_method_t  method;
	char	      *path;
} http_request_t;

void http_request_init(http_request_t *request);
void http_request_deinit(http_request_t *request);

void http_request_path_set(http_request_t *request, char const *path);
void http_request_path_set_n(http_request_t *request, char const *path, size_t n);

int   http_request_from_string(http_request_t *request, char const *str);
char *http_request_to_string(http_request_t *request);

int http_requests_from_string(http_request_t *requests, size_t *n, char const **str);

#endif
