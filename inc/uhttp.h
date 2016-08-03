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
	/**
	 * @brief Error codes that can be produced by implementations of Socket
	 *
	 * Note: Not all platforms produce all errors.
	 */
	enum class SocketError: int {
		/// No error condition
		OK = 0,
		/// Attempted an operation that requires the socket to be open, when 
		/// it's not.
		NOT_OPEN, 
		/// Connection is already open.
		ALREADY_OPEN,
		/// An error that wasn't identified as one of the standard ones.
		OTHER_ERROR,
		/// Hostname couldn't be resolved
		HOSTNAME_NOT_FOUND,
		/// Other initialisation failure during connect()
		INITIALISATION_FAILURE,
		/// Failed to connect to host.
		CONNECT_FAILURE,
		/// Client is not connected to host.
		NOT_CONNECTED,
		/// Connection was reset (remote-initiated)
		CONNECTION_RESET,
		/// Connection was lost (local-initiated)
		CONNECTION_ABORTED,
		/// Network is Down (local)
		NETWORK_DOWN,
		/// Destination Network is unreachable (remote)
		NETWORK_UNREACHABLE,
		/// Network was reset (local)
		NETWORK_RESET,
		/// Connection was refused by the remote host
		CONNECTION_REFUSED,
		/// Something timed out.
		TIMED_OUT,
	};

	/**
	 * @brief Stream is a basic abstraction of a bidirectional stream
	 *
	 * The fundamental goal of Stream is to seperate specifics such as stream
	 * encryption or compression from the actual socket, making it possible to
	 * extend the implementation further in the future.
	 *
	 * The expected behaviour of each method in Stream is given here.
	 *
	 * Implementations should acquire any details necessary for the connection
	 * via sub-classes or implementation specific methods, and implement the
	 * basic methods documented against this abstract class for use by
	 * the rest of the library.
	 *
	 * It's generally assumed that instantiation will not connect the stream,
	 * but \p connect() will.
	 */
	class Stream {
	public:
		/**
		 * @brief Connect the stream to the endpoint.
		 *
		 * If a failure is indicated, then \p error() will return an error state
		 * related to the cause of failure.
		 *
		 * If the stream is already connected, this method should fail and
		 * \p error() should return \p ErrorState::ALREADY_OPEN.
		 *
		 * @return \c true on success, \c false on failure.
		 */
		virtual bool connect() = 0;

		/**
		 * @brief Close the stream, disconnecting from the endpoint.
		 *
		 * Close the stream.  No-op if the stream is not open.
		 */
		virtual void close() = 0;

		/**
		 * @brief Indicate if the stream is closed or not.
		 *
		 * @return \c true if the stream is closed.  \c false otherwise.
		 */
		virtual bool closed() const = 0;

		/**
		 * @brief Return the error state from the last failure.
		 *
		 * @return a \p SocketError representing the last error encountered.
		 */
		virtual SocketError error() const = 0;

		/**
		 * @brief Read data from the stream
		 *
		 * Reads data from the stream, removing that data from the stream 
		 * buffer.
		 *
		 * @param buf Memory location to read to
		 * @param len Maximum number of bytes of data to read to buf
		 * @return number of bytes read, or \c -1 if an error is encountered.
		 */
		virtual ssize_t read(void *buf, size_t len) = 0;

		/**
		 * @brief Peeks at data in the stream
		 *
		 * Reads data from the stream, leaving that data in the stream 
		 * buffer to be returned by a subsequent read call.
		 *
		 * @param buf Memory location to read to
		 * @param len Maximum number of bytes of data to read to buf
		 * @return number of bytes read, or -1 if an error is encountered.
		 */
		virtual ssize_t peek(void *buf, size_t len) = 0;

		/**
		 * @brief Write data to the stream
		 *
		 * Writes len bytes of data from buf to the stream.
		 *
		 * @param buf pointer to data to write
		 * @param len number of bytes ot send.
		 * @return number of bytes writter, or -1 if an error is encountered.
		 */
		virtual ssize_t write(void *buf, size_t len) = 0;
	};

	/**
	 * @brief TcpStream provides a platform neutral abstraction of a normal 
	 *        TCP socket.
	 */
	class TcpStream : public Stream {
	public:
		/// if set true before invoking connect, the connection will be made
		/// with the platform's TCP/stream write coalescing/delay disabled.
		bool			tcp_no_delay;

		/**
		 * Create a TcpStream for connecting to the nominated host and port.
		 *
		 * The resulting TcpStream will not be connected to the service until
		 * connect is invoked.
		 *
		 * On Windows platforms, this will also take care of starting up
		 * winsock if it has not already been initialised.
		 *
		 * @param address Hostname or IP Address of the host to conenct to
		 * @param port Port to connect to in host byte order.
		*/
		TcpStream(const std::string &address, tcp_port_t port);

		/**
		 * Destroy a TcpStream, closing it if necessary first. 
		 *
		 * On Windows platforms, this will also take care of tearing down
		 * winsock if uhttp is the last consumer.
		 */
		virtual ~TcpStream();

		virtual bool connect();
		virtual void close();
		virtual bool closed() const;
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

	/** 
	 * @brief URI abstracts a Uniform Resource Identifier
	 *
	 * This abstraction only provides basic parsing facitilies and is very
	 * forviging of poorly constructed URIs.
	 *
	 * The actual scheme used is:
	 * <tt>[scheme://]address[:port][/path]</tt>
	 *
	 * If \c scheme is omitted, it is assumed to be http.
	 * 
	 * If \c port is omitted, it is assumed to be 80.
	 *
	 * If \c path is omitted, it is assumed to be /.
	 *
	 * Internally, the parse result is cached, with the result being discarded
	 * when a new value is set via \p set()
	 */
	class URI {
	public:
		/// @brief construct an empty URI
		URI();
		/// @brief construct a URI containing \c uri
		URI(const std::string &uri);
		/// @brief construct a copy of \c src
		URI(const URI &src);
		~URI();

		/**
		 * @brief Set a new value for the URI
		 *
		 * @param uri a string containing the new URI
		 */
		void set(const std::string &uri);

		/**
		 * @brief Get the current value of the URI
		 *
		 * The value returned by this method is the untouched URI as set by
		 * \p set() or via constructor.
		 *
		 * @return the URI as a string
		 */
		std::string get() const;

		/// @brief Returns the protocol specified in the URI
		std::string protocol();

		/// @brief Returns the address (hostname) specified in the URI
		std::string address();

		/// @brief Returns the port specified in the URI
		int 		port();

		/// @brief Returns the path specified in the URI
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
