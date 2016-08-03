/* socket_posix.cpp
 *
 * Socket Abstraction for Posix-like hosts.
*/

#include "uhttp.h"

#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>


using namespace std;
using namespace uhttp;

TcpStream::TcpStream(const std::string &address, tcp_port_t port) :
	tcp_no_delay(false),
	hostname(address),
	port(port),
	socket_fd(-1),
	errState(SocketError::OK)
{
}

TcpStream::~TcpStream()
{
	if (!closed()) {
		close();
	}
}

bool
TcpStream::connect()
{
	// stringify the port number for use in the getaddrinfo call
	char				serviceName[16];

	snprintf(serviceName, 16, "%d", port);
	serviceName[15] = '\0';

	// populate the getaddrinfo request structure, and perform the lookup
	struct addrinfo		addrHints;

	memset(&addrHints, 0, sizeof(addrHints));
	addrHints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
	addrHints.ai_family = AF_INET;
	addrHints.ai_socktype = SOCK_STREAM;

	struct addrinfo 	*results = NULL;

	if (getaddrinfo(hostname.c_str(), serviceName, &addrHints, &results)) {
		errState = SocketError::HOSTNAME_NOT_FOUND;
		return false;
	}

	socket_fd = socket(results->ai_family, results->ai_socktype, 0);
	if (-1 == socket_fd) {
		errState = SocketError::INITIALISATION_FAILURE;
		return false;
	}

	// disable NAGLE if requested.
	if (tcp_no_delay) {
		int nodelay = 1;
		setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, 
			reinterpret_cast<void *>(&nodelay), sizeof(nodelay));
	}

	if (!::connect(socket_fd, results->ai_addr, results->ai_addrlen)) {
		errState = SocketError::OK;
		return true;
	}
	setErrorState();
	::close(socket_fd);
	socket_fd = -1;
	return false;
}

void
TcpStream::close()
{
	if (socket_fd >= 0) {
		::close(socket_fd);
		socket_fd = -1;
	}
}

bool
TcpStream::closed() const
{
	return (socket_fd == -1);
}

SocketError
TcpStream::error() const
{
	return errState;
}

void
TcpStream::setErrorState()
{
	switch (errno) {
	case EINTR:
		// an interrupted or aborted operation is actually no-error, we've been 
		// forced to stop by explicit action on our behalf.
		errState = SocketError::OK;
		break;
	case ECONNREFUSED:
		errState = SocketError::CONNECTION_REFUSED;
		break;
	case ETIMEDOUT:
		errState = SocketError::TIMED_OUT;
		break;
	case ENETUNREACH:
		errState = SocketError::NETWORK_UNREACHABLE;
		break;
	case ENOTCONN:
		errState = SocketError::NOT_CONNECTED;
		break;
	case ECONNRESET:
		errState = SocketError::CONNECTION_RESET;
		break;
	default:
		errState = SocketError::OTHER_ERROR;
		break;
	}
}

ssize_t
TcpStream::read(void *buf, size_t len)
{
	if (closed()) {
		errState = SocketError::NOT_OPEN;
		return -1;
	}
	ssize_t opLen;

	opLen = recv(socket_fd, reinterpret_cast<char *>(buf), len, 0);
	if (opLen >= 0) {
		errState = SocketError::OK;
		return opLen;
	}
	setErrorState();
	return -1;
}

ssize_t
TcpStream::peek(void *buf, size_t len)
{
	if (closed()) {
		errState = SocketError::NOT_OPEN;
		return -1;
	}
	ssize_t opLen;

	opLen = recv(socket_fd, reinterpret_cast<char *>(buf), len, MSG_PEEK);
	if (opLen >= 0) {
		errState = SocketError::OK;
		return opLen;
	}
	setErrorState();
	return -1;
}

ssize_t
TcpStream::write(void *buf, size_t len)
{
	if (closed()) {
		errState = SocketError::NOT_OPEN;
		return -1;
	}
	ssize_t opLen;

	opLen = send(socket_fd, reinterpret_cast<const char *>(buf), len, 0);
	if (opLen >= 0) {
		errState = SocketError::OK;
		return opLen;
	}
	setErrorState();
	return -1;
}
