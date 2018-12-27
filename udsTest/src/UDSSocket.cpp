/*
	LURITECH Confidential Proprietary
	Copyright (C) 2003-2008 LURITECH Company.
	All rights are reserved by LURITECH Company.

	본 파일은 루리텍 회사의 비밀 정보를 포함하고 있습니다. 루리텍 회사의
	사전 서면 승인 없이 어떠한 형태나 방법으로도 본 비밀 정보를 제3자에게
	누설하거나, 전달하거나, 사용할 수 없습니다.

	This any file contain information that is confidential and proprietary
	to LURITECH company. No part of this information may be disclosed, used,
	copied, or transmitted in any form or by any means without prior
	written permission from LURITECH company.

	Confidential Proprietary

	space 4 tabs.
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

#include "UDSSocket.h"


/// Exception class

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
	std::string displayText() const {
		std::string txt = name();
		if (!_msg.empty()) {
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


/// ScopedState class

class ScopedState
{
public:
	ScopedState(bool& state) : _state(state) { _state = true; }
	virtual ~ScopedState() { _state = false; }

private:
	bool& _state;
};


/// Filescope functions

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

static uint32_t getTickCount() {
#ifdef _WIN32
	return (uint32_t)GetTickCount();
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}

#ifndef _WIN32
static void Sleep(long msec) {
	timeval delay = { msec / 1000, msec % 1000 * 1000 };
	int rc = ::select(0, NULL, NULL, NULL, &delay);
	if (-1 == rc) {
		// Handle signals by continuing to sleep or return immediately.
	}
}
#endif


/// UDSSocket class

UDSSocket::UDSSocket(void* user, onOpen open, onClose close, onBinary onBinary, onError error)
	: _user(user)
	, _onOpen(open)
	, _onClose(close) 
	, _onBinary(onBinary)
	, _onError(error)
	, _client(-1)
	, _server(-1)
	, _recvThread(0)
	, _unlinkFile(false)
	, _reduceWindowSize(100)
	, _calledCallback(false)
	, _closedInCallback(false)
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
		// UDS소켓은 서버와 클라이언트 통신을 위해 파일을 사용함 (TCP소켓은 IP:PORT을 사용함)
		_uri = pathAndFilename;
		UNLINK(_uri.c_str());

		// UDS 클라이언트 소켓을 생성함
		socket_t fd = (socket_t)::socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd < 0) {
			throw Exception("ERROR: Failed to socket handle", ERR_SOCKET_HANDLE);
		}
		_server = fd;
		printf("socket: _server=%d, path=%s\n", _server, pathAndFilename.c_str());

		// UDS 서버소켓에 파일을 바인딩함
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, pathAndFilename.c_str());

		int binded = ::bind(_server, (struct sockaddr *) &addr, sizeof(addr));
		if (binded != 0) {
			throw Exception("ERROR: Failed to bind", ERR_SERVER_BIND);
		}
		printf("bind: succeed\n");

		// UDS 서버소켓을 리스닝함 (클라이언트를 대기함)
		// - BACKLOG는 동시에 서버가 여러 개 들어온 경우 몇 개까지 큐에서 대기하는지를 지정한 것임
		const int UDS_MAX_BACKLOG = 1;
		int listened = ::listen(_server, UDS_MAX_BACKLOG);
		if (listened != 0) {
			throw Exception("ERROR: Failed to listen", ERR_SERVER_LISTEN);
		}
		printf("listen: succeed\n");

		// UDS 서버소켓으로 클라이언트가 접속하였음
		// - UDSSocket 클래스는 서버와 클라이언트가 1:1로 접속하는 싱글 모델로 구현하였음
		//   (즉 한 개의 UDSSocket 서버에 여러 개의 클라이언트를 연결할 수 없음)
		int sockfd = (socket_t)accept(_server, NULL, NULL);
		if (sockfd < 0) {
			throw Exception("ERROR: unable to accept", ERR_SERVER_ACCEPT);
		}
		_client = sockfd;
		printf("accept: _client=%d\n", _client);

		// UDS 서버소켓이 클라이언트가 송신한 프레임을 비동기로(thread) 수신함
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
			_onOpen(_user, _uri);

		return listening();
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(_user, std::string("listen: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, std::string("listen: unknown error"), (int)ERR_UNKNOWN);
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
		// UDS소켓은 서버와 클라이언트 통신을 위해 파일을 사용함 (TCP소켓은 IP:PORT을 사용함)
		_uri = pathAndFilename;

		// UDS 클라이언트 소켓을 생성함
		socket_t sockfd = (socket_t)::socket(AF_UNIX, SOCK_STREAM, 0);
		if (sockfd < 0) {
			throw Exception("ERROR: Failed to socket handle", ERR_SOCKET_HANDLE);
		}
		_client = sockfd;
		printf("socket: sockfd=%d, path=%s\n", _server, pathAndFilename.c_str());

		// UDS 클라이언트 소켓이 접속할 파일을 지정함
		// - UDS 서버가 미리 실행하고 반드시 리스닝하고 있지 않으면 예외가 발생함
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		strcpy(addr.sun_path, pathAndFilename.c_str());
		addr.sun_family = AF_UNIX;

		// UDS 클라이언트 소켓으로 UDS소켓 서버에 접속함
		int connected = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
		if (connected != 0) {
			closesocket(sockfd);
			throw Exception(
				"Unable to connect to " + pathAndFilename,
				ERR_CLIENT_UNABLE_TO_CONNECT);
		}
		setBlocking(sockfd, false);

		// UDS 클라이언트 소켓이 서버가 송신한 프레임을 비동기로(thread) 수신함
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
			_onOpen(_user, _uri);

		return opened();
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(_user, std::string("open: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, std::string("open: unknown error"), (int)ERR_UNKNOWN);
	}
	close();
	return true;
}

bool UDSSocket::close()
{
	try
	{
	    // close를 명시적으로 호출한 경우에 발생한 ERR_DECTECTED_DISCONNECTION_RECV 에러는
	    // onError() 콜백을 호출하지 않음
	    _calledClose = true;
    
	    // 콜백을 호출한 상태이면 close를 연기함
	    if (_calledCallback) {
		    _closedInCallback = true;
		    return false;
	    }

		// 연결을 종료함
		if (opened() || listening())
			disconnect();

		// 수신 쓰레드를 종료함
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
			    _onClose(_user, _uri);
	    }

		// 파일을 삭제함
		if (_unlinkFile) {
			UNLINK(_uri.c_str());
			printf("unlink: _uri=%s\n", _uri.c_str());
		}
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(_user, std::string("close: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, std::string("close: unknown error"), (int)ERR_UNKNOWN);
	}

	// 수신버퍼를 삭제하고 URI를 비움
	_uri.clear();
	_rxbuf.clear();
	_unlinkFile = false;
	_calledClose = false;
	return true;
}

bool UDSSocket::disconnect()
{
	bool disconnected = false;
	try
	{
		// 클라이언트인 경우 연결을 끊음
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
		// 서버인 경우 LISTEN을 종료
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
			_onError(_user, std::string("disconnect: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, std::string("disconnect: unknown error"), (int)ERR_UNKNOWN);
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
			if (uds->opened()) { uds->recvFrame(); }
		}
		catch (Exception& exc) {
			if (uds->_onError)
				uds->_onError(uds->_user, std::string("recv thread: ") + exc.displayText(), exc.code());
		}
		catch (...) {
			if (uds->_onError)
				uds->_onError(uds->_user, std::string("recv thread: unknown error"), (int)ERR_UNKNOWN);
		}
		if (uds->_onClose)
			uds->_onClose(uds->_user, uds->_uri);
		printf("recv thread: quit\n");
	}
	return 0;
}

void UDSSocket::recvFrame()
{
	try {
		// 시스템에 최적화된 버퍼사이즈를 구함
		enum { RECV_BUF_SIZE = 1500 };
		int size = RECV_BUF_SIZE;
		if (opened()) {
			socklen_t optlen = sizeof(size);
			getsockopt(_client, SOL_SOCKET, SO_RCVBUF, (char*)&size, &optlen);
			size -= _reduceWindowSize;
		}

		while (opened())
		{
			int N = (int)_rxbuf.size();

			// 소켓에 읽을 데이터가 있을 때까지 무한대기함
			struct pollfd fds[1];
			fds[0].fd = _client;
			fds[0].events = POLLIN;
			int polled = socketpoll(fds, 1, 1);
			if (polled < 0) { printf("polled stop\n"); disconnect(); }

			// 소켓 프레임을 반복 수신함
			if (polled) {
				ssize_t ret;
				_rxbuf.resize(N + size);
				ret = ::recv(_client, (char*)&_rxbuf[0] + N, size, 0);
				if (ret < 0 && (errno == EWOULDBLOCK || errno == EAGAIN /*|| errno == 0*/)) {
					_rxbuf.resize(N);
				}
				else if (ret <= 0) {
					_rxbuf.resize(N);
					if (ret == 0 && _onError && !_calledClose) {
						_onError(_user,
							std::string("detected disconnection: recv=") + TO_STRING(ret),
							ERR_RECV_DECTECTED_DISCONNECTION);
					}
					disconnect();
					break;
				}
				else {
					_rxbuf.resize(N + ret);
				}
			}

			UDSFrame frame;
			// 기본 소켓 헤더는 적어도 3바이트 이상이어야 함: id1(1)+id2(1)+len(1)
			if (_rxbuf.size() < 3) { continue; }
			// 수신한 소켓 헤더와 프레임을 파싱함
			depacketizeMultipartyLength(frame, _rxbuf.data());
			// 소켓 헤더만큼 읽지 못했다면 소켓에서 다시 읽음
			if (_rxbuf.size() < frame.headerSize) { continue; }
			// 소켓 프레임 전체를 수신하지 못한 경우 소켓에서 다시 읽음
			if (_rxbuf.size() < frame.headerSize + frame.N) { continue; }

			// 소켓 프레임 전체를 수신함
			_binary.insert(_binary.end(), _rxbuf.begin() + frame.headerSize, _rxbuf.begin() + frame.headerSize + (size_t)frame.N);
			if (frame.fin) {
				// 메시지 전체를 수신한 경우 _onBinary 콜백을 호출함
				if (_onBinary) {
					// 앞에 id1(1)+id2(1) 추가함
					_binary.insert(_binary.begin(), _rxbuf.begin(), _rxbuf.begin() + 2);
					if (_binary.size()) {
						ScopedState state(_calledCallback);
						_onBinary(_user, _binary);
						std::vector<uint8_t>().swap(_binary);
					}
					else if (_onError) {
						_onError(_user,
							std::string("warning: onBinary not called because frame is empty"),
							ERR_RECV_FRAME_SIZE_ZERO);
					}
				}
				// _onBinary 콜백 내부에서 close를 호출한 경우
				// 콜백에서 나온 후 연결을 종료함
				if (_closedInCallback) {
					_closedInCallback = false;
					disconnect();
					break;
				}
			}

			// 수신한 전체 버퍼에서 콜백으로 처리한 만큼을 삭제함
			if (_rxbuf.size())
				_rxbuf.erase(_rxbuf.begin(), _rxbuf.begin() + frame.headerSize + (size_t)frame.N);
		}
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(_user, std::string("recvFrame: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, std::string("recvFrame: unknown error"), (int)ERR_UNKNOWN);
	}
}

unsigned UDSSocket::depacketizeMultipartyLength(UDSFrame& depacketized, const void* frame)
{
	if (!frame)
		return 0;
	try
	{
		// 기본 소켓 헤더를 읽어옴
		const uint8_t* data = (uint8_t *)frame + 2;
		depacketized.fin = (data[0] & 0x80) != 0x80;
		depacketized.N0 = (data[0] & 0x7f);
		depacketized.headerSize = 2 + 1 + (depacketized.N0 == 126 ? 2 : 0);
		// 프레임 길이가 0~125이면 헤더를 1바이트 사용함
		if (depacketized.N0 < 126) {
			depacketized.N = depacketized.N0;
		}
		// 프레임 길이가 126~65535이면 헤더를 3바이트 사용함
		else if (depacketized.N0 == 126) {
			depacketized.N = 0;
			depacketized.N |= ((uint64_t)data[1]) << 8;
			depacketized.N |= ((uint64_t)data[2]) << 0;
		}
		return depacketized.headerSize;
	}
	catch (...) {
		// depacketize standard error
	}
	return 0;
}

unsigned UDSSocket::packetizeMultipartyLength(std::vector<uint8_t>& packetized, const std::size_t size, bool multiparty)
{
	try
	{
		// 전송하려는 메시지 사이즈는 적어도 1바이트 이상임
		packetized.assign(1 + (size >= 126 ? 2 : 0), 0);
		// 멀티파티 연속전송이면 FIN을 0으로, 마지막 전송이면 FIN을 1로 설정함
		if (multiparty) packetized[0] = 0x80; else packetized[0] = 0x00;
		// 메시지 사이즈가 0~125이면 헤더를 1바이트 사용함
		if (size < 126) {
			packetized[0] |= (size & 0xff);
		}
		// 메시지 사이즈가 126~65535이면 헤더를 3바이트 사용함
		else if (size < 65536) {
			packetized[0] |= 126;
			packetized[1] = (size >> 8) & 0xff;
			packetized[2] = (size >> 0) & 0xff;
		}
		return (unsigned)packetized.size();
	}
	catch (...) {
		// depacketize standard error
	}
	return 0;
}

bool UDSSocket::send(uint8_t id1, uint8_t id2, const void* data, const std::size_t size)
{
	std::vector<uint8_t> hdr;
	hdr.insert(hdr.end(), (const char*)&id1, (const char*)&id1 + sizeof(id1));
	hdr.insert(hdr.end(), (const char*)&id2, (const char*)&id2 + sizeof(id2));
	std::vector<uint8_t> payload;
	if (data && size) payload.insert(payload.end(), (const char*)data, ((const char*)data) + size);
	return send(hdr, payload);
}

bool UDSSocket::send(uint8_t id1, uint8_t id2, const std::vector<uint8_t>& payload)
{
	std::vector<uint8_t> hdr;
	hdr.insert(hdr.end(), (const char*)&id1, (const char*)&id1 + sizeof(id1));
	hdr.insert(hdr.end(), (const char*)&id2, (const char*)&id2 + sizeof(id2));
	return send(hdr, payload);
}

bool UDSSocket::send(const std::vector<uint8_t>& hdr, const std::vector<uint8_t>& payload)
{
	try
	{
		const int MAX_HDR = 5; // id1(1), id2(1), len(3), reserved(1)
		const int MAX_MULTIPARTY = 256 - MAX_HDR; // 250

		bool multiparty = false;
		std::vector<uint8_t> txbuf;
		txbuf.insert(txbuf.end(), payload.begin(), payload.end());
		// payload가 0인 경우에도 적어도 1회 읽을 필요가 있음
		do
		{
			// 멀티파티 전송여부를 확인함
			uint64_t messageSize = txbuf.size();
			if (messageSize > MAX_MULTIPARTY) {
				messageSize = MAX_MULTIPARTY;
				multiparty = true;
			}

			// 전송하려는 메시지 사이즈를 패킷타이즈
			std::vector<uint8_t> len;
			UDSSocket::packetizeMultipartyLength(len, messageSize, multiparty && messageSize == MAX_MULTIPARTY);

			// 보내려는 바이너리를 멀티파티 전송모델로 패킷타이즈함
			// 멀티파티 전송모델: id1(1), id2(1), len(1~3), data(len)
			std::vector<uint8_t> binary;
			binary.insert(binary.end(), hdr.begin(), hdr.end());
			binary.insert(binary.end(), len.begin(), len.end());
			binary.insert(binary.end(), txbuf.begin(), txbuf.begin() + messageSize);

			// 바이너리를 전송함
			std::size_t bytesSent = sendRaw(binary);
			if (bytesSent != binary.size()) {
				throw Exception("sent=" + TO_STRING(bytesSent) + ", size=" + TO_STRING(binary.size()), ERR_SEND);
			}

			// 보낸 메시지 만큼 송신버퍼에서 삭제함
			if (txbuf.size())
				txbuf.erase(txbuf.begin(), txbuf.begin() + messageSize);
		} while (txbuf.size());

		return true;
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(_user, std::string("send: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, "send: unknown error", (int)ERR_UNKNOWN);
	}
	return false;
}

std::size_t UDSSocket::sendRaw(const std::vector<uint8_t>& binary)
{
	return sendFrame(binary.size(), binary.begin(), binary.end());
}

template<class Iterator>
std::size_t UDSSocket::sendFrame(uint64_t messageSize, Iterator messageBegin, Iterator messageEnd)
{
	std::size_t bytesSent = 0;
	try
	{
		std::vector<uint8_t> txbuf;
		txbuf.insert(txbuf.end(), messageBegin, messageEnd);

		if (opened()) {
			// 시스템에 최적화된 버퍼사이즈를 구함
			enum { SEND_BUF_SIZE = 1500 };
			int size = SEND_BUF_SIZE;
			socklen_t optlen = sizeof(size);
			getsockopt(_client, SOL_SOCKET, SO_SNDBUF, (char*)&size, &optlen);
			size -= _reduceWindowSize;

#ifdef _WIN32
			std::lock_guard<std::mutex> lock(_mutex);
#else
			ScopedLock<Mutex> lock(_mutex);
#endif
			while (txbuf.size()) {
				// 소켓에 데이터를 쓸 수 있을 때까지 무한대기함
				struct pollfd fds[1];
				fds[0].fd = _client;
				fds[0].events = POLLOUT;
				int polled = socketpoll(fds, 1, -1);
				if (polled < 0) { printf("polled stop\n"); break; }

				// 소켓 프레임을 모두 보낼 때까지 반복 송신함
				int tx = (int)(txbuf.size() > size ? size : txbuf.size());
				int sent = ::send(_client, (char*)&txbuf[0], tx, 0);
				if (sent <= 0) {
					throw Exception("sent=" + TO_STRING(sent) + ", errno=" + TO_STRING(socketerrno), ERR_SEND);
				}

				// 보낸 만큼 송신버퍼에서 삭제함
				bytesSent += sent;
				if (txbuf.size())
					txbuf.erase(txbuf.begin(), txbuf.begin() + sent);
			}
		}
	}
	catch (Exception& exc) {
		if (_onError)
			_onError(_user, std::string("sendFrame: ") + exc.displayText(), exc.code());
	}
	catch (...) {
		if (_onError)
			_onError(_user, "sendFrame: unknown error", (int)ERR_UNKNOWN);
	}
	return bytesSent;
}