set(HEADERS
	include/http/http.h
)

set(SOURCES
	src/request.c
)

add_library(http ${HEADERS} ${SOURCES})
target_include_directories(http PUBLIC include PRIVATE include/http)
