#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "message.h"

#define HTTP_STATUS_CONTINUE_STRING	       "Continue"
#define HTTP_STATUS_SWITCHING_PROTOCOLS_STRING "Switching Protocols"
#define HTTP_STATUS_PROCESSING_STRING	       "Processing"
#define HTTP_STATUS_EARLY_HINTS_STRING	       "Early Hints"

#define HTTP_STATUS_OK_STRING				 "OK"
#define HTTP_STATUS_CREATED_STRING			 "Created"
#define HTTP_STATUS_ACCEPTED_STRING			 "Accepted"
#define HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION_STRING "Non-Authoritative Information"
#define HTTP_STATUS_NO_CONTENT_STRING			 "No Content"
#define HTTP_STATUS_RESET_CONTENT_STRING		 "Reset Content"
#define HTTP_STATUS_PARTIAL_CONTENT_STRING		 "Partial Content"
#define HTTP_STATUS_MULTI_STATUS_STRING			 "Multi-Status"
#define HTTP_STATUS_ALREADY_REPORTED_STRING		 "Already Reported"
#define HTTP_STATUS_IM_USED_STRING			 "IM Used"

#define HTTP_STATUS_MULTIPLE_CHOICES_STRING   "Multiple Choices"
#define HTTP_STATUS_MOVED_PERMANENTLY_STRING  "Moved Permanently"
#define HTTP_STATUS_FOUND_STRING	      "Found"
#define HTTP_STATUS_SEE_OTHER_STRING	      "See Other"
#define HTTP_STATUS_NOT_MODIFIED_STRING	      "Not Modified"
#define HTTP_STATUS_USE_PROXY_STRING	      "Use Proxy"
#define HTTP_STATUS_UNUSED_STRING	      "Unused"
#define HTTP_STATUS_TEMPORARY_REDIRECT_STRING "Temporary Redirect"
#define HTTP_STATUS_PERMANENT_REDIRECT_STRING "Permanent Redirect"

#define HTTP_STATUS_BAD_REQUEST_STRING			   "Bad Request"
#define HTTP_STATUS_UNAUTHORIZED_STRING			   "Unauthorized"
#define HTTP_STATUS_PAYMENT_REQUIRED_STRING		   "Payment Required"
#define HTTP_STATUS_FORBIDDEN_STRING			   "Forbidden"
#define HTTP_STATUS_NOT_FOUND_STRING			   "Not Found"
#define HTTP_STATUS_METHOD_NOT_ALLOWED_STRING		   "Method Not Allowed"
#define HTTP_STATUS_NOT_ACCEPTABLE_STRING		   "Not Acceptable"
#define HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED_STRING   "Proxy Authentication Required"
#define HTTP_STATUS_REQUEST_TIMEOUT_STRING		   "Request Timeout"
#define HTTP_STATUS_CONFLICT_STRING			   "Conflict"
#define HTTP_STATUS_GONE_STRING				   "Gone"
#define HTTP_STATUS_LENGTH_REQUIRED_STRING		   "Length Required"
#define HTTP_STATUS_PRECONDITION_FAILED_STRING		   "Precondition Failed"
#define HTTP_STATUS_PAYLOAD_TOO_LARGE_STRING		   "Payload Too Large"
#define HTTP_STATUS_URI_TOO_LONG_STRING			   "URI Too Long"
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE_STRING	   "Unsupported Media Type"
#define HTTP_STATUS_RANGE_NOT_SATISFIABLE_STRING	   "Range Not Satisfiable"
#define HTTP_STATUS_EXPECTATION_FAILED_STRING		   "Expectation Failed"
#define HTTP_STATUS_IM_A_TEAPOT_STRING			   "I'm a teapot"
#define HTTP_STATUS_MISDIRECTED_REQUEST_STRING		   "Misdirected Request"
#define HTTP_STATUS_UNPROCESSABLE_CONTENT_STRING	   "Unprocessesable Content"
#define HTTP_STATUS_LOCKED_STRING			   "Locked"
#define HTTP_STATUS_FAILED_DEPENDENCY_STRING		   "Failed Dependency"
#define HTTP_STATUS_TOO_EARLY_STRING			   "Too Early"
#define HTTP_STATUS_UPGRADE_REQUIRED_STRING		   "Upgrade Required"
#define HTTP_STATUS_PRECONDITION_REQUIRED_STRING	   "Precondition Required"
#define HTTP_STATUS_TOO_MANY_REQUESTS_STRING		   "Too Many Requests"
#define HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE_STRING "Request Header Fields Too Large"
#define HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS_STRING   "Unavailable For Legal Reasons"

#define HTTP_STATUS_INTERNAL_SERVER_ERROR_STRING	   "Internal Server Error"
#define HTTP_STATUS_NOT_IMPLEMENTED_STRING		   "Not Implemented"
#define HTTP_STATUS_BAD_GATEWAY_STRING			   "Bad Gateway"
#define HTTP_STATUS_SERVICE_UNAVAILABLE_STRING		   "Service Unavailable"
#define HTTP_STATUS_GATEWAY_TIMEOUT_STRING		   "Gateway Timeout"
#define HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED_STRING	   "HTTP Version Not Supported"
#define HTTP_STATUS_VARIANT_ALSO_NEGOTIATES_STRING	   "Variant Also Negotiates"
#define HTTP_STATUS_INSUFFICIENT_STORAGE_STRING		   "Insufficient Storage"
#define HTTP_STATUS_LOOP_DETECTED_STRING		   "Loop Detected"
#define HTTP_STATUS_NOT_EXTENDED_STRING			   "Not Extended"
#define HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED_STRING "Network Authentication Required"

typedef enum {
	http_status_continue			    = 100,
	http_status_switching_protocols		    = 101,
	http_status_processing			    = 102,
	http_status_early_hints			    = 103,
	http_status_ok				    = 200,
	http_status_created			    = 201,
	http_status_accepted			    = 202,
	http_status_non_authoritative_information   = 203,
	http_status_no_content			    = 204,
	http_status_reset_content		    = 205,
	http_status_partial_content		    = 206,
	http_status_multi_status		    = 207,
	http_status_already_reported		    = 208,
	http_status_im_used			    = 226,
	http_status_multiple_choices		    = 300,
	http_status_moved_permanently		    = 301,
	http_status_found			    = 302,
	http_status_see_other			    = 303,
	http_status_not_modified		    = 304,
	http_status_use_proxy			    = 305, ///< deprecated
	http_status_unused			    = 306, ///< deprecated
	http_status_temporary_redirect		    = 307,
	http_status_permanent_redirect		    = 308,
	http_status_bad_request			    = 400,
	http_status_unauthorized		    = 401,
	http_status_payment_required		    = 402,
	http_status_forbidden			    = 403,
	http_status_not_found			    = 404,
	http_status_method_not_allowed		    = 405,
	http_status_not_acceptable		    = 406,
	http_status_proxy_authentication_required   = 407,
	http_status_request_timeout		    = 408,
	http_status_conflict			    = 409,
	http_status_gone			    = 410,
	http_status_length_required		    = 411,
	http_status_precondition_failed		    = 412,
	http_status_payload_too_large		    = 413,
	http_status_uri_too_long		    = 414,
	http_status_unsupported_media_type	    = 415,
	http_status_range_not_satisfiable	    = 416,
	http_status_expectation_failed		    = 417,
	http_status_im_a_teapot			    = 418,
	http_status_misdirected_request		    = 421,
	http_status_unprocessable_content	    = 422,
	http_status_locked			    = 423,
	http_status_failed_dependency		    = 424,
	http_status_too_early			    = 425,
	http_status_upgrade_required		    = 426,
	http_status_precondition_required	    = 428,
	http_status_too_many_requests		    = 429,
	http_status_request_header_fields_too_large = 431,
	http_status_unavailable_for_legal_reasons   = 451,
	http_status_internal_server_error	    = 500,
	http_status_not_implemented		    = 501,
	http_status_bad_gateway			    = 502,
	http_status_service_unavailable		    = 503,
	http_status_gateway_timeout		    = 504,
	http_status_http_version_not_supported	    = 505,
	http_status_variant_also_negotiates	    = 506,
	http_status_insufficient_storage	    = 507,
	http_status_loop_detected		    = 508,
	http_status_not_extended		    = 510,
	http_status_network_authentication_required = 511,
	http_status_count,
} http_status_t;

char const *http_status_to_string(http_status_t status);

typedef struct {
	http_message_t message;
	http_status_t  status;
} http_response_t;

void http_response_init(http_response_t *response);
void http_response_deinit(http_response_t *response);

int   http_response_from_string(http_response_t *response, char const *str);
char *http_response_to_string(http_response_t *response);

int   http_responses_from_string(http_response_t *responses, size_t *n, char const *str);
char *http_responses_to_string(http_response_t *responses, size_t n);

#endif
