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
#include <mutex>
typedef void* Thread;
#else
#include <stdint.h>
typedef pthread_t Thread;
#endif

#include <string>
#include <vector>


#pragma once


#ifndef _WIN32

/// Mutex class

class Mutex
{
public:
	Mutex();
	~Mutex();
	void lock();
	bool tryLock();
	void unlock();

private:
	pthread_mutex_t _mutex;
};

/// ScopedLock class

template <class M>
class ScopedLock
{
public:
	explicit ScopedLock(M& mutex) : _mutex(mutex)
	{
		_mutex.lock();
	}
	~ScopedLock()
	{
		try
		{
			_mutex.unlock();
		}
		catch (...)
		{
			// nothing to do
		}
	}

private:
	M& _mutex;
};

#endif

class UDSSocket
{
public:
	enum WSErrCode {
		ERR_UNKNOWN = 100,
		ERR_SOCKET_HANDLE = 101,
		ERR_SERVER_BIND = 102,
		ERR_SERVER_LISTEN = 103,
		ERR_SERVER_ACCEPT = 104,
		ERR_CLIENT_UNABLE_TO_CONNECT = 105,
		ERR_RECV_DECTECTED_DISCONNECTION = 106,
		ERR_SEND = 107,
		ERR_RECV_THREAD_INVALID = 108,
		ERR_RECV_FRAME_SIZE_ZERO = 109,
	};
	typedef int socket_t;
	typedef std::vector<uint8_t> sockbuff;
	typedef void(*onOpen)(const std::string& url);
	typedef void(*onClose)(const std::string& url);
	typedef void(*onError)(const std::string& msg, const int errNo);
	enum { UDSSOCKET_VERSION = 0x0A010201 }; // reserved.major.minor.revision
	struct UDSFrame
		/// 송수신 프레임을 정의함
	{
		unsigned headerSize;/// 헤더 사이즈
		bool 	 fin;		/// false: 연속 프레임, true: 완전한 프레임
		int 	 N0;		/// 헤더 길이 (uint8_t: 0~126, uint16_t:127, uint64_t:128)
		uint64_t N;			/// 프레임 전체 사이즈
	};

public:
	UDSSocket(onOpen open, onClose close, onError error);

	virtual ~UDSSocket();

public:
	bool listen(const std::string& pathAndFilename);

	bool listening() const;

	bool open(const std::string& pathAndFilename);

	bool opened() const;

	bool close();

protected:
	virtual bool disconnect();

protected:
	onOpen		_onOpen;
	onClose		_onClose;
	onError		_onError;

	socket_t	_server;
	socket_t	_client;
	std::string	_uri;

	bool		_unlinkFile;
};

/// inline

inline bool UDSSocket::listening() const
{
	return _server != -1;
}

inline bool UDSSocket::opened() const
{
	return _client != -1;
}