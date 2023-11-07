#ifndef HTTP_VERSION_H
#define HTTP_VERSION_H

#include <stdint.h>

#define HTTP_VERSION_STRING_LENGTH 8

typedef struct {
	uint8_t major;
	uint8_t minor;
} http_version_t;

void http_version_init(http_version_t *version, uint8_t mahor, uint8_t minor);

char *http_version_to_string(http_version_t *version);

#endif
