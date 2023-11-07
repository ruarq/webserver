#include "parser.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "response.h"
#include "message.h"
#include "config.h"
#include "request.h"

#define CURRENT(p)	 ((p)->source[(p)->current])
#define LEXEME(p)	 ((p)->source + (p)->current)
#define IS_EOF(p)	 ((p)->current >= (p)->source_len)
#define IS_DIGIT(c)	 (isalnum(c))
#define CR		 '\r'
#define LF		 '\n'
#define CRLF		 "\r\n"
#define IS_METHOD(c)	 ((c) >= 'A' && (c) <= 'Z')
#define IS_HEADER_KEY(c) (isalpha(c) || (c) == '-')
#define IS_PATH_SEGMENT(c) \
	(isalpha(c) || isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
#define STREQ(s1, s2)	   (strcmp(s1, s2) == 0)
#define STRNEQ(s1, s2, n)  (strncmp(s1, s2, n) == 0)
#define STRNEQL(s1, s2, n) (strncmp(s1, s2, n) == 0 && strlen(s1) == (n))
#define SKIP_WHITESPACE(p)                                  \
	do {                                                \
		char c = (p)->source[(p)->current];         \
		if (c == '\0' || (c != ' ' && c != '\t')) { \
			break;                              \
		}                                           \
		++((p)->current);                           \
	} while (1)

char const *http_strerror(int error)
{
	switch ((http_parser_result_t)error) {
	case http_parser_result_ok:
		return HTTP_PARSER_ERROR_NOERROR_STRING;

	case http_parser_result_crlf:
		return HTTP_PARSER_ERROR_CRLF_STRING;

	case http_parser_result_method:
		return HTTP_PARSER_ERROR_METHOD_STRING;

	case http_parser_result_path:
		return HTTP_PARSER_ERROR_PATH_STRING;

	case http_parser_result_version:
		return HTTP_PARSER_ERROR_VERSION_STRING;

	case http_parser_result_version_mismatch:
		return HTTP_PARSER_ERROR_VERSION_MISMATCH_STRING;

	case http_parser_result_status:
		return HTTP_PARSER_ERROR_STATUS_STRING;

	case http_parser_result_status_invalid:
		return HTTP_PARSER_ERROR_STATUS_INVALID_STRING;

	case http_parser_result_header:
		return HTTP_PARSER_ERROR_HEADER_STRING;

	case http_parser_result_key:
		return HTTP_PARSER_ERROR_KEY_STRING;

	case http_parser_result_value:
		return HTTP_PARSER_ERROR_VALUE_STRING;
	default:
		break;
	}

	return NULL;
}

static http_parser_result_t _http_parser_parse_request(http_parser_t  *parser,
						       http_request_t *request);

static http_parser_result_t _http_parser_parse_response(http_parser_t	*parser,
							http_response_t *response);

static http_parser_result_t http_parser_parse_method(http_parser_t  *parser,
						     http_request_t *request);

static http_parser_result_t http_parser_parse_path(http_parser_t *parser, http_request_t *request);

static http_parser_result_t http_parser_validate_path(http_parser_t *parser, char const *path);

static http_parser_result_t http_parser_parse_version(http_parser_t  *parser,
						      http_message_t *message);

static http_parser_result_t http_parser_parse_status_code(http_parser_t	  *parser,
							  http_response_t *response);

static http_parser_result_t _http_parser_parse_message(http_parser_t  *parser,
						       http_message_t *message);

static http_parser_result_t http_parser_parse_header(http_parser_t  *parser,
						     http_message_t *message);

static http_parser_result_t http_parser_parse_headers(http_parser_t  *parser,
						      http_message_t *message);

static void http_parser_parse_content(http_parser_t *parser, http_message_t *message);

void http_parser_init(http_parser_t *parser)
{
	assert(parser);

	parser->source	   = NULL;
	parser->source_len = 0;
	parser->current	   = 0;
}

static http_parser_result_t _http_parser_parse_request(http_parser_t  *parser,
						       http_request_t *request)
{
	assert(parser && request);

	http_parser_result_t result;

	http_request_init(request);

	SKIP_WHITESPACE(parser);
	result = http_parser_parse_method(parser, request);
	if (result != http_parser_result_ok) {
		return result;
	}

	SKIP_WHITESPACE(parser);
	result = http_parser_parse_path(parser, request);
	if (result != http_parser_result_ok) {
		return result;
	}

	SKIP_WHITESPACE(parser);
	result = http_parser_parse_version(parser, &request->message);
	if (result != http_parser_result_ok) {
		return result;
	}

	SKIP_WHITESPACE(parser);
	if (CURRENT(parser) == LF) {
		++parser->current;
	} else if (STRNEQ(LEXEME(parser), CRLF, strlen(CRLF))) {
		parser->current += 2;
	} else {
		return http_parser_result_crlf;
	}

	result = http_parser_parse_message(parser, request);
	if (result != http_parser_result_ok) {
		return result;
	}

	return http_parser_result_ok;
}

http_parser_result_t http_parser_parse_request(http_parser_t *parser, void *request)
{
	return _http_parser_parse_request(parser, (http_request_t *)request);
}

static http_parser_result_t _http_parser_parse_response(http_parser_t	*parser,
							http_response_t *response)
{
	assert(parser && response);

	http_parser_result_t result;

	http_response_init(response);

	SKIP_WHITESPACE(parser);
	result = http_parser_parse_version(parser, &response->message);
	if (result != http_parser_result_ok) {
		return result;
	}

	SKIP_WHITESPACE(parser);
	result = http_parser_parse_status_code(parser, response);
	if (result != http_parser_result_ok) {
		return result;
	}

	SKIP_WHITESPACE(parser);

	// if there is a status message, we don't care about it
	while (!IS_EOF(parser)) {
		if (CURRENT(parser) == LF) {
			break;
		} else if (STRNEQ(LEXEME(parser), CRLF, strlen(CRLF))) {
			break;
		}

		++parser->current;
	}

	if (IS_EOF(parser)) {
		return http_parser_result_crlf;
	}

	if (CURRENT(parser) == CR) {
		++parser->current;
	}
	++parser->current;

	return http_parser_parse_message(parser, response);
}

http_parser_result_t http_parser_parse_response(http_parser_t *parser, void *response)
{
	return _http_parser_parse_response(parser, (http_response_t *)response);
}

#define MAP_METHOD(s, m)                              \
	do {                                          \
		if (STRNEQL(s, method, method_len)) { \
			request->method = m;          \
			return http_parser_result_ok; \
		}                                     \
	} while (0)

static http_parser_result_t http_parser_parse_method(http_parser_t *parser, http_request_t *request)
{
	assert(parser && request);

	char const *method;
	size_t	    method_len = 0;

	method = LEXEME(parser);
	while (!IS_EOF(parser) && IS_METHOD(CURRENT(parser))) {
		++parser->current;
		++method_len;
	}

	if (method_len == 0) {
		return http_parser_result_method;
	}

	MAP_METHOD(HTTP_METHOD_CONNECT_STRING, http_method_connect);
	MAP_METHOD(HTTP_METHOD_DELETE_STRING, http_method_delete);
	MAP_METHOD(HTTP_METHOD_HEAD_STRING, http_method_head);
	MAP_METHOD(HTTP_METHOD_GET_STRING, http_method_get);
	MAP_METHOD(HTTP_METHOD_OPTIONS_STRING, http_method_options);
	MAP_METHOD(HTTP_METHOD_PATCH_STRING, http_method_patch);
	MAP_METHOD(HTTP_METHOD_POST_STRING, http_method_post);
	MAP_METHOD(HTTP_METHOD_PUT_STRING, http_method_put);
	MAP_METHOD(HTTP_METHOD_TRACE_STRING, http_method_trace);

	return http_parser_result_method;
}

#undef MAP_METHOD

static http_parser_result_t http_parser_parse_path(http_parser_t *parser, http_request_t *request)
{
	assert(parser && request);

	char const *path;
	size_t	    path_len = 0;

	path = LEXEME(parser);
	while (!IS_EOF(parser) && !isspace(CURRENT(parser))) {
		++parser->current;
		++path_len;
	}

	if (path_len == 0) {
		return http_parser_result_path;
	}

	http_request_path_set_n(request, path, path_len);
	return http_parser_validate_path(parser, request->path);
}

static http_parser_result_t http_parser_validate_path(http_parser_t *parser, char const *path)
{
	for (; *path != '\0'; ++path) {
		if (*path != '/') {
			return http_parser_result_path;
		}

		++path;
		for (; *path != '\0'; ++path) {
			if (path[1] == '/') {
				break;
			}

			if (!IS_PATH_SEGMENT(*path)) {
				return http_parser_result_path;
			}
		}
	}

	return http_parser_result_ok;
}

static http_parser_result_t http_parser_parse_version(http_parser_t  *parser,
						      http_message_t *message)
{
	assert(parser && message);

	uint8_t major;
	uint8_t minor;

	if (!STRNEQ(LEXEME(parser), "HTTP/", strlen("HTTP/"))) {
		return http_parser_result_version;
	}

	parser->current += strlen("HTTP/");

	if (!IS_DIGIT(CURRENT(parser))) {
		return http_parser_result_version;
	}

	major = CURRENT(parser) - '0';
	++parser->current;

	if (CURRENT(parser) != '.') {
		return http_parser_result_version;
	}
	++parser->current;

	if (!IS_DIGIT(CURRENT(parser))) {
		return http_parser_result_version;
	}

	minor = CURRENT(parser) - '0';
	++parser->current;

	if (major != HTTP_VERSION_MAJOR || minor != HTTP_VERSION_MINOR) {
		return http_parser_result_version_mismatch;
	}

	http_version_init(&message->version, major, minor);

	return http_parser_result_ok;
}

static http_parser_result_t http_parser_parse_status_code(http_parser_t	  *parser,
							  http_response_t *response)
{
	assert(parser && response);

	size_t i = 0;
	char   status[3 + 1];

	for (i = 0; i < 3 && !IS_EOF(parser); ++i) {
		if (!IS_DIGIT(CURRENT(parser))) {
			return http_parser_result_status;
		}

		status[i] = CURRENT(parser);
		++parser->current;
	}

	status[i] = '\0';

	response->status = atoi(status);

	if (!http_status_to_string(response->status)) {
		return http_parser_result_status_invalid;
	}

	return http_parser_result_ok;
}

static http_parser_result_t _http_parser_parse_message(http_parser_t  *parser,
						       http_message_t *message)
{
	assert(parser && message);

	http_parser_result_t result;

	http_message_init(message);
	result = http_parser_parse_headers(parser, message);
	if (result != http_parser_result_ok) {
		return result;
	}

	http_parser_parse_content(parser, message);

	return http_parser_result_ok;
}

http_parser_result_t http_parser_parse_message(http_parser_t *parser, void *message)
{
	return _http_parser_parse_message(parser, (http_message_t *)message);
}

static http_parser_result_t http_parser_parse_header(http_parser_t *parser, http_message_t *message)
{
	assert(parser && message);

	char const *key;
	size_t	    key_len = 0;
	char const *value;
	size_t	    value_len = 0;

	SKIP_WHITESPACE(parser);

	key = LEXEME(parser);
	while (!IS_EOF(parser) && IS_HEADER_KEY(CURRENT(parser))) {
		++parser->current;
		++key_len;
	}

	SKIP_WHITESPACE(parser);
	if (CURRENT(parser) != ':') {
		return http_parser_result_header;
	}
	++parser->current;
	SKIP_WHITESPACE(parser);

	value = LEXEME(parser);
	while (!IS_EOF(parser)) {
		if (CURRENT(parser) == LF) {
			break;
		} else if (STRNEQ(LEXEME(parser), CRLF, strlen(CRLF))) {
			break;
		}

		++parser->current;
		++value_len;
	}

	if (IS_EOF(parser)) {
		return http_parser_result_crlf;
	}

	if (CURRENT(parser) == CR) {
		++parser->current;
	}
	++parser->current;

	if (key_len == 0) {
		return http_parser_result_key;
	}
	if (value_len == 0) {
		return http_parser_result_value;
	}

	http_message_header_set_n(message, key, key_len, value, value_len);

	return http_parser_result_ok;
}

static http_parser_result_t http_parser_parse_headers(http_parser_t  *parser,
						      http_message_t *message)
{
	assert(parser && message);

	http_parser_result_t result;

	while (!IS_EOF(parser)) {
		if (CURRENT(parser) == LF) {
			break;
		} else if (STRNEQ(LEXEME(parser), CRLF, strlen(CRLF))) {
			break;
		}

		if ((result = http_parser_parse_header(parser, message)) != http_parser_result_ok) {
			return result;
		}
	}

	if (IS_EOF(parser)) {
		return http_parser_result_crlf;
	}

	if (CURRENT(parser) == CR) {
		++parser->current;
	}
	++parser->current;

	return http_parser_result_ok;
}

static void http_parser_parse_content(http_parser_t *parser, http_message_t *message)
{
	assert(parser && message);

	if (IS_EOF(parser)) {
		return;
	}

	http_message_content_set(message, LEXEME(parser));
}
