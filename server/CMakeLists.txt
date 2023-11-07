set(HEADERS
	src/webserver.h
)

set(SOURCES
	src/main.c
	src/webserver.c
)

add_executable(webserver ${HEADERS} ${SOURCES})
target_include_directories(webserver PRIVATE http)
target_link_libraries(webserver PRIVATE http)
