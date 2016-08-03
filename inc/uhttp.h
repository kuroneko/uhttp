/* uhttp.h
 *
 * micro HTTP library
 *
 * Copyright (C) 2016, Chris Collins
 */
#ifndef _UHTTP_H
#define _UHTTP_H

#include <string>
#include <cstdint>

#include "uhttp_platform.h"

namespace uhttp {
	enum class SocketError: int {
		OK = 0,
		NOT_OPEN,
		OTHER_ERROR,
		HOSTNAME_NOT_FOUND,
		INITIALISATION_FAILURE,
		CONNECT_FAILURE,
		NOT_CONNECTED,
		CONNECTION_RESET,
		CONNECTION_ABORTED,
		NETWORK_DOWN,
		NETWORK_UNREACHABLE,
		NETWORK_RESET,
		CONNECTION_REFUSED,
		TIMED_OUT,
	};

	class Socket {
	public:
		virtual bool connect() = 0;
		virtual void close() = 0;
		virtual bool closed() const = 0;
		virtual SocketError error() const = 0;
		virtual ssize_t read(void *buf, size_t len) = 0;
		virtual ssize_t peek(void *buf, size_t len) = 0;
		virtual ssize_t write(void *buf, size_t len) = 0;
	};

	class TcpSocket : public Socket {
	public:
		// if set true before connect(), this will inform connect to attempt to disable any
		// write coalescing/delay in the OS's sockets layer
		bool			tcp_no_delay;

		TcpSocket(const std::string &address, tcp_port_t port);
		virtual ~TcpSocket();

		virtual bool connect();
		virtual void close();
		virtual bool closed();
		virtual SocketError error() const;
		virtual ssize_t read(void *buf, size_t len);
		virtual ssize_t peek(void *buf, size_t len);
		virtual ssize_t write(void *buf, size_t len);

	protected:
		std::string		hostname;
		tcp_port_t		port;

	private:
		socket_t 		socket_fd;
		SocketError		errState;
		void			setErrorState();
	};

	class URI {
	public:
		URI();
		URI(const std::string &uri);
		URI(const URI &src);
		~URI();

		//- Raw URI get/set
		void set(const std::string &uri);
		std::string get() const;

		//- parsed getters.
		std::string protocol();
		std::string address();
		int 		port();
		std::string path();
	protected:
		std::string rawURI;
	private:
		void 		parse();
		bool		didParse;
		std::string _protocol, _host, _path;
		int 		_port;
	};

	class Client {
	public:
		Client();
		virtual ~Client();
	};
}

#endif
