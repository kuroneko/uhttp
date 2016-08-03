/* socket_win32.cpp
*
* Socket Abstraction for Win32 (Winsock) hosts.
*/

#include "uhttp.h"

#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;
using namespace uhttp;

TcpStream::TcpStream(const std::string &address, tcp_port_t port) :
	tcp_no_delay(false),
	hostname(address),
	port(port),
	socket_fd(-1),
	errState(SocketError::OK)
{
	// ensure winsock is initialized
	WSADATA	wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

TcpStream::~TcpStream()
{
	if (!closed()) {
		close();
	}
	WSACleanup();
}

bool
TcpStream::connect()
{
	// stringify the port number for use in the getaddrinfo call
	char	serviceName[16];
	_snprintf_s(serviceName, 16, _TRUNCATE, "%d", port);

	// populate the getaddrinfo request structure, and perform the lookup
	struct addrinfo addrHints;
	memset(&addrHints, 0, sizeof(addrHints));
	addrHints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
	addrHints.ai_family = AF_INET;
	addrHints.ai_socktype = SOCK_STREAM;

	struct addrinfo *results = NULL;
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
		setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&nodelay), sizeof(nodelay));
	}

	if (!::connect(socket_fd, results->ai_addr, static_cast<int>(results->ai_addrlen))) {
		errState = SocketError::OK;
		return true;
	}
	::closesocket(socket_fd);
	socket_fd = -1;
	errState = SocketError::CONNECT_FAILURE;
	return false;
}

void
TcpStream::close()
{
	if (socket_fd >= 0) {
		::closesocket(socket_fd);
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
	int errCode = WSAGetLastError();
	switch (errCode) {
	case WSAEINTR:
	case WSA_OPERATION_ABORTED:
		// an interrupted or aborted operation is actually no-error, we've been forced
		// to stop by explicit action on our behalf.
		errState = SocketError::OK;
		break;
	case WSAENETDOWN:
		errState = SocketError::NETWORK_DOWN;
		break;
	case WSAENETUNREACH:
		errState = SocketError::NETWORK_UNREACHABLE;
		break;
	case WSAECONNABORTED:
		errState = SocketError::CONNECTION_ABORTED;
		break;
	case WSAENETRESET:
		errState = SocketError::NETWORK_RESET;
		break;
	case WSAENOTCONN:
		errState = SocketError::NOT_CONNECTED;
		break;
	case WSAECONNRESET:
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

	opLen = recv(socket_fd, reinterpret_cast<char *>(buf), static_cast<int>(len), 0);
	if (opLen != SOCKET_ERROR) {
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

	opLen = recv(socket_fd, reinterpret_cast<char *>(buf), static_cast<int>(len), MSG_PEEK);
	if (opLen != SOCKET_ERROR) {
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

	opLen = send(socket_fd, reinterpret_cast<const char *>(buf), static_cast<int>(len), 0);
	if (opLen != SOCKET_ERROR) {
		errState = SocketError::OK;
		return opLen;
	}
	setErrorState();
	return -1;
}
