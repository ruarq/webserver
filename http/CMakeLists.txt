set(HEADERS
	include/http/http.h
	include/http/config.h
	include/http/header.h
	include/http/message.h
)

set(SOURCES
	src/header.c
	src/message.c
	src/request.c
)

add_library(http ${HEADERS} ${SOURCES})
target_include_directories(http PUBLIC include PRIVATE include/http)
