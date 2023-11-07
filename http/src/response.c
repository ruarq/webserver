#include "response.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>

#include "parser.h"

static char const *http_status_to_string_map[] = {
	[http_status_continue]		  = HTTP_STATUS_CONTINUE_STRING,
	[http_status_switching_protocols] = HTTP_STATUS_SWITCHING_PROTOCOLS_STRING,
	[http_status_processing]	  = HTTP_STATUS_PROCESSING_STRING,
	[http_status_early_hints]	  = HTTP_STATUS_EARLY_HINTS_STRING,
	[http_status_ok]		  = HTTP_STATUS_OK_STRING,
	[http_status_created]		  = HTTP_STATUS_CREATED_STRING,
	[http_status_accepted]		  = HTTP_STATUS_ACCEPTED_STRING,
	[http_status_non_authoritative_information] =
		HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION_STRING,
	[http_status_no_content]	 = HTTP_STATUS_NO_CONTENT_STRING,
	[http_status_reset_content]	 = HTTP_STATUS_RESET_CONTENT_STRING,
	[http_status_partial_content]	 = HTTP_STATUS_PARTIAL_CONTENT_STRING,
	[http_status_multi_status]	 = HTTP_STATUS_MULTI_STATUS_STRING,
	[http_status_already_reported]	 = HTTP_STATUS_ALREADY_REPORTED_STRING,
	[http_status_im_used]		 = HTTP_STATUS_IM_USED_STRING,
	[http_status_multiple_choices]	 = HTTP_STATUS_MULTIPLE_CHOICES_STRING,
	[http_status_moved_permanently]	 = HTTP_STATUS_MOVED_PERMANENTLY_STRING,
	[http_status_found]		 = HTTP_STATUS_FOUND_STRING,
	[http_status_see_other]		 = HTTP_STATUS_SEE_OTHER_STRING,
	[http_status_not_modified]	 = HTTP_STATUS_NOT_MODIFIED_STRING,
	[http_status_use_proxy]		 = HTTP_STATUS_USE_PROXY_STRING,
	[http_status_unused]		 = HTTP_STATUS_UNUSED_STRING,
	[http_status_temporary_redirect] = HTTP_STATUS_TEMPORARY_REDIRECT_STRING,
	[http_status_permanent_redirect] = HTTP_STATUS_PERMANENT_REDIRECT_STRING,
	[http_status_bad_request]	 = HTTP_STATUS_BAD_REQUEST_STRING,
	[http_status_unauthorized]	 = HTTP_STATUS_UNAUTHORIZED_STRING,
	[http_status_payment_required]	 = HTTP_STATUS_PAYMENT_REQUIRED_STRING,
	[http_status_forbidden]		 = HTTP_STATUS_FORBIDDEN_STRING,
	[http_status_not_found]		 = HTTP_STATUS_NOT_FOUND_STRING,
	[http_status_method_not_allowed] = HTTP_STATUS_METHOD_NOT_ALLOWED_STRING,
	[http_status_not_acceptable]	 = HTTP_STATUS_NOT_ACCEPTABLE_STRING,
	[http_status_proxy_authentication_required] =
		HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED_STRING,
	[http_status_request_timeout]	     = HTTP_STATUS_REQUEST_TIMEOUT_STRING,
	[http_status_conflict]		     = HTTP_STATUS_CONFLICT_STRING,
	[http_status_gone]		     = HTTP_STATUS_GONE_STRING,
	[http_status_length_required]	     = HTTP_STATUS_LENGTH_REQUIRED_STRING,
	[http_status_precondition_failed]    = HTTP_STATUS_PRECONDITION_FAILED_STRING,
	[http_status_payload_too_large]	     = HTTP_STATUS_PAYLOAD_TOO_LARGE_STRING,
	[http_status_uri_too_long]	     = HTTP_STATUS_URI_TOO_LONG_STRING,
	[http_status_unsupported_media_type] = HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE_STRING,
	[http_status_range_not_satisfiable]  = HTTP_STATUS_RANGE_NOT_SATISFIABLE_STRING,
	[http_status_expectation_failed]     = HTTP_STATUS_EXPECTATION_FAILED_STRING,
	[http_status_im_a_teapot]	     = HTTP_STATUS_IM_A_TEAPOT_STRING,
	[http_status_misdirected_request]    = HTTP_STATUS_MISDIRECTED_REQUEST_STRING,
	[http_status_unprocessable_content]  = HTTP_STATUS_UNPROCESSABLE_CONTENT_STRING,
	[http_status_locked]		     = HTTP_STATUS_LOCKED_STRING,
	[http_status_failed_dependency]	     = HTTP_STATUS_FAILED_DEPENDENCY_STRING,
	[http_status_too_early]		     = HTTP_STATUS_TOO_EARLY_STRING,
	[http_status_upgrade_required]	     = HTTP_STATUS_UPGRADE_REQUIRED_STRING,
	[http_status_precondition_required]  = HTTP_STATUS_PRECONDITION_REQUIRED_STRING,
	[http_status_too_many_requests]	     = HTTP_STATUS_TOO_MANY_REQUESTS_STRING,
	[http_status_request_header_fields_too_large] =
		HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE_STRING,
	[http_status_unavailable_for_legal_reasons] =
		HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS_STRING,
	[http_status_internal_server_error]	 = HTTP_STATUS_INTERNAL_SERVER_ERROR_STRING,
	[http_status_not_implemented]		 = HTTP_STATUS_NOT_IMPLEMENTED_STRING,
	[http_status_bad_gateway]		 = HTTP_STATUS_BAD_GATEWAY_STRING,
	[http_status_service_unavailable]	 = HTTP_STATUS_SERVICE_UNAVAILABLE_STRING,
	[http_status_gateway_timeout]		 = HTTP_STATUS_GATEWAY_TIMEOUT_STRING,
	[http_status_http_version_not_supported] = HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED_STRING,
	[http_status_variant_also_negotiates]	 = HTTP_STATUS_VARIANT_ALSO_NEGOTIATES_STRING,
	[http_status_insufficient_storage]	 = HTTP_STATUS_INSUFFICIENT_STORAGE_STRING,
	[http_status_loop_detected]		 = HTTP_STATUS_LOOP_DETECTED_STRING,
	[http_status_not_extended]		 = HTTP_STATUS_NOT_EXTENDED_STRING,
	[http_status_network_authentication_required] =
		HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED_STRING,
};

char const *http_status_to_string(http_status_t status)
{
	if (status >= http_status_count || status < 0) {
		return NULL;
	}

	return http_status_to_string_map[status];
}

void http_response_init(http_response_t *response)
{
	assert(response);

	http_message_init(response);
}

void http_response_deinit(http_response_t *response)
{
	assert(response);

	http_message_deinit(response);
}

int http_response_from_string(http_response_t *response, char const *str)
{
	http_parser_t	     parser;
	http_parser_result_t result;

	http_parser_init(&parser);
	parser.source	  = str;
	parser.source_len = strlen(str);
	result		  = http_parser_parse_response(&parser, response);

	return result;
}

char *http_response_to_string(http_response_t *response)
{
	assert(response);

	char	   *message_string = http_message_to_string(&response->message);
	char	   *version_string = http_version_to_string(&response->message.version);
	char const *status_string  = http_status_to_string(response->status);
	char	    status_num_string[3 + 1];
	size_t	    required_length;
	char	   *buf;

	snprintf(status_num_string, 3 + 1, "%d", response->status);

	required_length = strlen(version_string) + 1 + strlen(status_num_string) + 1 +
			  strlen(status_string) + 2 + strlen(message_string);

	buf = calloc(required_length + 1, sizeof(char));
	snprintf(buf, required_length + 1, "%s %s %s\r\n%s", version_string, status_num_string,
		 status_string, message_string);

	free(message_string);
	free(version_string);

	return buf;
}
