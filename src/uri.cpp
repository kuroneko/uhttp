/* uri.cpp
 *
 * URI parsing
 *
 * Copyright (C) 2016, Christopher Collins
 *
 * This intentionally doesn't support any authentication schemes, breakdown of the
 * path/query, or other complicated stuff.  Please use a more featureful HTTP client
 * if you need that stuff.
*/

#include "uhttp.h"
#include <string>
#include <cstdint>

using namespace std;
using namespace uhttp;

URI::URI() :
	rawURI(),
	didParse(false)
{
}

URI::URI(const std::string &uri) :
	rawURI(uri),
	didParse(false)
{
}

URI::URI(const URI &src) :
	rawURI(src.rawURI),
	didParse(false)
{
}

void
URI::set(const std::string &uri)
{
	rawURI = uri;
	didParse = false;
}

std::string
URI::get() const
{
	return rawURI;
}

void
URI::parse()
{
	// identify the protocol
	size_t protDelim = rawURI.find("://");
	size_t hostOffset = 0;

	if (protDelim == string::npos) {
		// no protocol, assume http.
		_protocol = "http";		
	} else {
		hostOffset = protDelim+3;
		_protocol = rawURI.substr(0, protDelim);
	}

	// now, look for the / that ends the address section.
	size_t pathOffset = rawURI.find("/", hostOffset);
	if (pathOffset == string::npos) {
		_host = rawURI.substr(hostOffset);
		_path = "/";
	} else {
		_host = rawURI.substr(hostOffset, pathOffset-hostOffset);
		_path = rawURI.substr(pathOffset);
	}

	// now, try to break down address.
	size_t portOffset = _host.find(":");
	if (portOffset == string::npos) {
		// default port.  use -1 to indicate
		_port = -1;
	} else {
		std::string portStr = _host.substr(portOffset+1);
		_port = stoi(portStr);
		_host = _host.substr(0, portOffset);
	}

	didParse = true;
	return;
}

std::string
URI::protocol()
{
	if (!didParse) {
		parse();
	}
	return _protocol;
}

std::string
URI::address()
{
	if (!didParse) {
		parse();
	}
	return _host;	
}

int
URI::port()
{
	if (!didParse) {
		parse();
	}
	return _port;
}

std::string
URI::path()
{
	if (!didParse) {
		parse();
	}
	return _path;
}
