#include "version.h"

#include <stdio.h>
#include <stdlib.h>

void http_version_init(http_version_t *version, uint8_t major, uint8_t minor)
{
	version->major = major;
	version->minor = minor;
}

char *http_version_to_string(http_version_t *version)
{
	// HTTP/X.X
	char *buf = calloc(HTTP_VERSION_STRING_LENGTH + 1, sizeof(char));
	snprintf(buf, HTTP_VERSION_STRING_LENGTH + 1, "HTTP/%d.%d", version->major, version->minor);
	return buf;
}
