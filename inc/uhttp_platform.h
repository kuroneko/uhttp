/* uhttp_platform.h
 *
 * micro HTTP library : Platform Abstractions
 *
 * Copyright (C) 2016, Chris Collins
 */

#ifndef UHTTP_PLATFORM_H
#define UHTTP_PLATFORM_H

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace uhttp {
	typedef uint16_t	tcp_port_t;

#ifdef _WIN32
	typedef SOCKET		socket_t;
	typedef intptr_t	ssize_t;
#else
	typedef int			socket_t;
#endif
};

#endif /* #ifndef UHTTP_PLATFORM_H */