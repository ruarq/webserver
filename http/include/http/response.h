#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "message.h"

#define HTTP_STATUS_OK_STRING	       "OK"
#define HTTP_STATUS_CREATED_STRING     "Created"
#define HTTP_STATUS_NO_CONTENT_STRING  "No Content"
#define HTTP_STATUS_BAD_REQUEST_STRING "Bad Request"
#define HTTP_STATUS_FORBIDDEN_STRING   "Forbidden"
#define HTTP_STATUS_NOT_FOUND_STRING   "Not Found"

#define HTTP_RESPONSE_BUFFER_SIZE 8192

typedef enum {
	// request valid
	http_status_ok = 200,

	// resource has been created successfully
	http_status_created = 201,

	// resource has been overwritten
	http_status_no_content = 204,

	// request invalid
	http_status_bad_request = 400,

	// not enough rights to modify resource
	http_status_forbidden = 403,

	// requested resource doesn't exist
	http_status_not_found = 404,
} http_status_t;

char const *http_status_to_string(http_status_t status);

typedef struct {
	http_message_t message;
	char	      *http_version;
	http_status_t  status;
} http_response_t;

void http_response_init(http_response_t *response);
void http_response_deinit(http_response_t *response);

void http_response_set_http_version(http_response_t *response, char const *http_version);

bool  http_response_from_string(http_response_t *response, char const *str);
char *http_response_to_string(http_response_t *response);

#endif
