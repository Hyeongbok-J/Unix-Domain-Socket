/*
	MIT License

	Copyright (c) 2018 Hyeongbok-Jeon

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifdef _WIN32
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS // _CRT_SECURE_NO_WARNINGS for sscanf errors in MSVC2013 Express
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <fcntl.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment( lib, "ws2_32" )
#include <afunix.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <io.h>
#ifndef _SSIZE_T_DEFINED
typedef int ssize_t;
#define _SSIZE_T_DEFINED
#endif
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef snprintf
#define snprintf _snprintf_s
#endif
#if _MSC_VER >=1600
// vs2010 or later
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#define socketpoll	WSAPoll
#define socketerrno WSAGetLastError()
#define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#define TO_STRING(x) std::to_string(x)
#define SLEEP Sleep
#define UNLINK _unlink
#else
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdint.h>
#ifndef _SOCKET_T_DEFINED
typedef int socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR   (-1)
#endif
#define closesocket(s) ::close(s)
#define socketpoll	::poll
#include <errno.h>
#define socketerrno errno
#define SOCKET_EAGAIN_EINPROGRESS EAGAIN
#define SOCKET_EWOULDBLOCK EWOULDBLOCK
#define TO_STRING(x) static_cast< std::ostringstream & >( \
			( std::ostringstream() << std::dec << x ) ).str()
#define SLEEP Sleep
#define UNLINK unlink
#endif

#include <stdexcept>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>

#include "UnixSocket.h"

class Exception : public std::exception
{
public:
	Exception(const std::string& msg, int code) : _msg(msg), _code(code) {}
	Exception(const Exception& exc) : std::exception(exc), _msg(exc._msg), _code(exc._code) {}
	Exception& operator = (const Exception& exc) {
		if (&exc != this) {
			_msg = exc._msg;
			_code = exc._code;
		}
		return *this;
	}
	~Exception() throw() {}

public:
	virtual Exception* clone() const { return new Exception(*this); }
	virtual void rethrow() const { throw *this; }
	virtual const char* name() const throw() { return "Exception"; }
#ifdef _WIN32
	virtual const char* className() const throw() { return typeid(*this).name(); }
#endif
	virtual const char* what() const throw() { return name(); }
	const std::string& message() const { return _msg; }
	int code() const { return _code; }
	std::string displayText() const
	{
		std::string txt = name();
		if (!_msg.empty())
		{
			txt.append(": ");
			txt.append(_msg);
		}
		return txt;
	}

protected:
	Exception(int code = 0) : _code(code) {}
	void message(const std::string& msg) { _msg = msg; }

private:
	std::string _msg;
	int			_code;
};

#ifndef _WIN32

/// Mutex class

Mutex::Mutex()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	if (pthread_mutex_init(&_mutex, &attr))
	{
		pthread_mutexattr_destroy(&attr);
		throw Exception("cannot create mutex", 0);
	}
	pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&_mutex);
}

void Mutex::lock()
{
	if (pthread_mutex_lock(&_mutex))
		throw Exception("cannot lock mutex", 0);
}

bool Mutex::tryLock()
{
	int rc = pthread_mutex_trylock(&_mutex);
	if (rc == 0)
		return true;
	else if (rc == EBUSY)
		return false;
	else
		throw Exception("cannot lock mutex", 0);
}

void Mutex::unlock()
{
	if (pthread_mutex_unlock(&_mutex))
		throw Exception("cannot unlock mutex", 0);
}

#endif

static bool setBlocking(socket_t sockfd, bool blocking)
{
	if (sockfd == -1)
		return false;
	try
	{
#ifdef _WIN32
		u_long on = !blocking;
		ioctlsocket(sockfd, FIONBIO, &on);
#else
		int old = fcntl(sockfd, F_GETFL);
		int flag = 0;
		if (blocking)
			flag = old & ~O_NONBLOCK;
		else
			flag = old | O_NONBLOCK;
		fcntl(sockfd, F_SETFL, flag);
#endif
		return true;
	}
	catch (...) {
		// nothing to do
	}
	return false;
}

UDSSocket::UDSSocket(onOpen open, onClose close, onError error)
	: _onOpen(open)
	, _onClose(close)
	, _onError(error)
	, _client(-1)
	, _server(-1)
	, _recvThread(0)
	, _unlinkFile(false)
{

}

UDSSocket::~UDSSocket()
{

}

bool UDSSocket::listen(const std::string& pathAndFilename)
{
	if (listening())
		return false;
	try
	{
		_uri = pathAndFilename;
		UNLINK(_uri.c_str());

		socket_t fd = (socket_t)::socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd < 0)
		{
			throw Exception("ERROR: Failed to socket handle", ERR_SOCKET_HANDLE);
		}
		_server = fd;
		printf("socket: _server=%d, path=%s\n", _server, pathAndFilename.c_str());

		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, pathAndFilename.c_str());

		int binded = ::bind(_server, (struct sockaddr *) &addr, sizeof(addr));
		if (binded != 0)
		{
			throw Exception("ERROR: Failed to bind", ERR_SERVER_BIND);
		}
		printf("bind: succeed\n");

		const int UDS_MAX_BACKLOG = 1;
		int listened = ::listen(_server, UDS_MAX_BACKLOG);
		if (listened != 0)
		{
			throw Exception("ERROR: Failed to listen", ERR_SERVER_LISTEN);
		}
		printf("listen: succeed\n");

		int sockfd = (socket_t)accept(_server, NULL, NULL);
		if (sockfd < 0)
		{
			throw Exception("ERROR: unable to accept", ERR_SERVER_ACCEPT);
		}
		_client = sockfd;
		printf("accept: _client=%d\n", _client);

#ifdef _WIN32
		_recvThread = CreateThread(NULL, 0, UDSSocket::recv, (void*)this, 0, NULL);
		if (_recvThread == 0) {
			throw Exception("ERROR: Failed to created thread", ERR_RECV_THREAD_INVALID);
		}
#else
		int handle = pthread_create(&_recvThread, NULL, &UDSSocket::recv, this);
		if (handle != 0) {
			throw Exception("ERROR: Failed to created thread", ERR_RECV_THREAD_INVALID);
		}
#endif
		_unlinkFile = true;

		if (listening() && _onOpen)
			_onOpen(_uri);

		return listening();
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(std::string("listen: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(std::string("listen: unknown error"), (int)ERR_UNKNOWN);
	}
	close();
	return false;
}

bool UDSSocket::open(const std::string& pathAndFilename)
{
	if (opened())
		return false;
	try
	{
		_uri = pathAndFilename;

		socket_t sockfd = (socket_t)::socket(AF_UNIX, SOCK_STREAM, 0);
		if (sockfd < 0) {
			throw Exception("ERROR: Failed to socket handle", ERR_SOCKET_HANDLE);
		}
		_client = sockfd;
		printf("socket: sockfd=%d, path=%s\n", _client, pathAndFilename.c_str());

		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		strcpy(addr.sun_path, pathAndFilename.c_str());
		addr.sun_family = AF_UNIX;

		int connected = connect(_client, (struct sockaddr *)&addr, sizeof(addr));
		if (connected != 0) {
			closesocket(sockfd);
			throw Exception(
				"Unable to connect to " + pathAndFilename,
				ERR_CLIENT_UNABLE_TO_CONNECT);
		}

#ifdef _WIN32
		_recvThread = CreateThread(NULL, 0, UDSSocket::recv, (void*)this, 0, NULL);
		if (_recvThread == 0) {
			throw Exception("ERROR: Failed to created thread", ERR_RECV_THREAD_INVALID);
		}
#else
		int handle = pthread_create(&_recvThread, NULL, &UDSSocket::recv, this);
		if (handle != 0) {
			throw Exception("ERROR: Failed to created thread", ERR_RECV_THREAD_INVALID);
		}
#endif
		if (opened() && _onOpen)
			_onOpen(_uri);

		return opened();
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(std::string("open: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(std::string("open: unknown error"), (int)ERR_UNKNOWN);
	}
	close();
	return true;
}

bool UDSSocket::close()
{
	try
	{
		if (opened() || listening())
			disconnect();

		if (_recvThread) {
#ifdef _WIN32
			WaitForSingleObject(_recvThread, INFINITE);
#else
			pthread_join(_recvThread, NULL);
#endif
			_recvThread = 0;
		}
		else {
			if (_onClose)
				_onClose(_uri);
		}

		if (_unlinkFile) {
			UNLINK(_uri.c_str());
			printf("unlink: _uri=%s\n", _uri.c_str());
		}
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(std::string("close: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(std::string("close: unknown error"), (int)ERR_UNKNOWN);
	}
	_uri.clear();
	_unlinkFile = false;
	return true;
}

bool UDSSocket::disconnect()
{
	bool disconnected = false;
	try
	{
		if (opened())
		{
			printf("disconnect: client=%d\n", _client);
#ifndef _WIN32
			::shutdown(_client, SHUT_RDWR);
#endif
			closesocket(_client);
			_client = -1;
			disconnected = true;
		}
		if (listening())
		{
			printf("disconnect: server=%d\n", _server);
#ifndef _WIN32
			::shutdown(_server, SHUT_RDWR);
#endif
			closesocket(_server);
			_server = -1;
			disconnected = true;
		}
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(std::string("disconnect: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(std::string("disconnect: unknown error"), (int)ERR_UNKNOWN);
	}
	return disconnected;
}

#ifdef _WIN32
unsigned long UDSSocket::recv(void* thisPtr)
#else
void* UDSSocket::recv(void* thisPtr)
#endif
{
	UDSSocket* uds = (UDSSocket*)thisPtr;
	if (uds)
	{
		try
		{
			if (uds->opened()) { uds->recvTest(); }
		}
		catch (Exception& exc) {
			if (uds->_onError)
				uds->_onError(std::string("recv thread: ") + exc.displayText(), exc.code());
		}
		catch (...) {
			if (uds->_onError)
				uds->_onError(std::string("recv thread: unknown error"), (int)ERR_UNKNOWN);
		}
		if (uds->_onClose)
			uds->_onClose(uds->_uri);
		printf("recv thread: quit\n");
	}
	return 0;
}

void UDSSocket::recvTest() {

	char buf[4096];

	while (true)
	{
		memset(buf, 0, 4096);

		int bytesReceived = ::recv(_client, buf, 4096, 0);

		if (bytesReceived == SOCKET_ERROR)
		{
			std::cerr << "Error in recv(). Quitting" << std::endl;
			break;
		}

		if (bytesReceived == 0)
		{
			std::cout << "Client disconnected " << std::endl;
			break;
		}

		::send(_client, buf, bytesReceived + 1, 0);
	}
}

void UDSSocket::sendTest() {

	char buf[4096];
	std::string userInput;

	do
	{
		std::cout << "> ";
		getline(std::cin, userInput);

		if (userInput.size() > 0)
		{
			int sendResult = ::send(_client, userInput.c_str(), userInput.size() + 1, 0);

			if (sendResult != SOCKET_ERROR)
			{
				memset(buf, 0, 4096);

				int bytesReceived = ::recv(_client, buf, 4096, 0);
				if (bytesReceived > 0)
				{
					std::cout << "SERVER >> " << std::string(buf, 0, bytesReceived) << std::endl;
				}
			}
		}
	} while (userInput.size() > 0);
}