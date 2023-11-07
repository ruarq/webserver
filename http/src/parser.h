#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stddef.h>

#define HTTP_PARSER_ERROR_NOERROR_STRING	  "Something weird happend :-("
#define HTTP_PARSER_ERROR_CRLF_STRING		  "Missing CRLF"
#define HTTP_PARSER_ERROR_METHOD_STRING		  "Invalid method"
#define HTTP_PARSER_ERROR_PATH_STRING		  "Missing path"
#define HTTP_PARSER_ERROR_VERSION_STRING	  "Invalid version"
#define HTTP_PARSER_ERROR_VERSION_MISMATCH_STRING "Version mismatch, try HTTP/1.1"
#define HTTP_PARSER_ERROR_STATUS_STRING		  "Invalid status code"
#define HTTP_PARSER_ERROR_STATUS_INVALID_STRING	  "No such status code"
#define HTTP_PARSER_ERROR_HEADER_STRING		  "Malformed header"
#define HTTP_PARSER_ERROR_KEY_STRING		  "Missing header key"
#define HTTP_PARSER_ERROR_VALUE_STRING		  "Missing header value"

typedef struct {
	char const *source;
	size_t	    source_len;
	size_t	    current;
} http_parser_t;

typedef enum {
	http_parser_result_ok,
	http_parser_result_crlf,
	http_parser_result_method,
	http_parser_result_path,
	http_parser_result_version,
	http_parser_result_version_mismatch,
	http_parser_result_status,
	http_parser_result_status_invalid,
	http_parser_result_header,
	http_parser_result_key,
	http_parser_result_value,
} http_parser_result_t;

void http_parser_init(http_parser_t *parser);

http_parser_result_t http_parser_parse_request(http_parser_t *parser, void *request);
http_parser_result_t http_parser_parse_response(http_parser_t *parser, void *response);
http_parser_result_t http_parser_parse_message(http_parser_t *parser, void *message);

#endif
