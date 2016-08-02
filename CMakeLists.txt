cmake_minimum_required(VERSION 3.0.2)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(UHTTP_PLATFORM_SRC src/socket_posix.cpp)	
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(UHTTP_PLATFORM_SRC src/socket_win32.cpp)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(UHTTP_PLATFORM_SRC src/socket_posix.cpp)	
else()
	message(FATAL_ERROR "I couldn't recognise the platform ${CMAKE_SYSTEM_NAME}")
endif()

add_library(uhttp STATIC 
	src/uri.cpp
	${UHTTP_PLATFORM_SRC})
target_include_dirs(uhttp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
