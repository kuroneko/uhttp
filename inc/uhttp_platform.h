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
	/// A type suitable for holding TCP ports.
	typedef uint16_t	tcp_port_t;

#ifdef _WIN32
	/// Define the type used to represent sockets by the local TCP implementation
	typedef SOCKET		socket_t;

	/// Win32 is missing the ssize_t integer type, so we substitute intptr_t for it.
	typedef intptr_t	ssize_t;
#else
	/// Define the type used to represent sockets by the local TCP implementation
	typedef int			socket_t;
#endif
};

#endif /* #ifndef UHTTP_PLATFORM_H */