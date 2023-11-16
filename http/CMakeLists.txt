set(HEADERS
	include/http/http.h
	include/http/common.h
	include/http/config.h
	include/http/header.h
	include/http/message.h
	include/http/request.h
	include/http/response.h
	include/http/version.h
	include/http/parser.h
)

set(SOURCES
	src/header.c
	src/message.c
	src/request.c
	src/response.c
	src/version.c
	src/parser.c
)

add_library(http ${HEADERS} ${SOURCES})
target_include_directories(http PUBLIC include PRIVATE include/http)
