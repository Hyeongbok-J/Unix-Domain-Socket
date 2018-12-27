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
	#include <mutex>
	typedef void* Thread;
#else
    #include <stdint.h>
	pthread_t Thread;
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
	explicit ScopedLock(M& mutex): _mutex(mutex)
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

/// UDSSocket class

class UDSSocket
	/// UDSSocket은 유닉스를 위한 Unix Domain Socket을 구현한 것으로
	/// listen/listening을 통해 서버를 실행하고 실행유무를 확인할 수 있고
	/// open/opened를 통해 클라이언트를 실행하고 실행유무를 확인할 수 있음
	/// 송신을 위해 send를 사용하고 수신을 위해 onBinary 콜백을 사용함
	/// 17063빌드부터 윈도우에서 AF_UNIX를 지원함 (December 19, 2017)
{
public:
	enum WSErrCode {
		ERR_UNKNOWN = 100,
		ERR_SOCKET_HANDLE = 101,
		ERR_SERVER_BIND = 102,
		ERR_SERVER_LISTEN = 103,
		ERR_SERVER_ACCEPT= 104,
		ERR_CLIENT_UNABLE_TO_CONNECT = 105,
		ERR_RECV_DECTECTED_DISCONNECTION = 106,
		ERR_SEND = 107,
		ERR_RECV_THREAD_INVALID = 108,
		ERR_RECV_FRAME_SIZE_ZERO = 109,
	};
	typedef int socket_t;
	typedef std::vector<uint8_t> sockbuff;
	typedef void(*onOpen)(void* user, const std::string& url);
	typedef void(*onClose)(void* user, const std::string& url);
	typedef void(*onBinary)(void* user, const std::vector<uint8_t>& binary);
	typedef void(*onError)(void* user, const std::string& msg, const int errNo);
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
	UDSSocket(void* user, onOpen open, onClose close, onBinary onBinary, onError error);
		/// 생성함
	
	virtual ~UDSSocket();
		/// 소멸함

public:
	bool listen(const std::string& pathAndFilename);
		/// 지정한 경로의 파일을 리스닝함 (서버로 실행)

	bool listening() const;
		/// 리스닝 상태인지 확인함

	bool open(const std::string& pathAndFilename);
		/// 지정한 경로의 파일에 접속함 (클라이언트로 실행)

	bool opened() const;
		/// 연결한 상태인지 확인함

	bool close();
		/// 종료함

	virtual bool send(uint8_t id1, uint8_t id2, const void* data=0, const std::size_t size=0);
		/// 바이너리를 전송함 (멀티파티 전송모델 지원)

	virtual bool send(uint8_t id1, uint8_t id2, const std::vector<uint8_t>& payload);
		/// 바이너리를 전송함 (멀티파티 전송모델 지원)

	virtual bool send(const std::vector<uint8_t>& hdr, const std::vector<uint8_t>& payload);
		/// 바이너리를 전송함 (멀티파티 전송모델 지원)

	virtual std::size_t sendRaw(const std::vector<uint8_t>& binary);
		/// 바이너리를 그대로 전송함

	static unsigned depacketizeMultipartyLength(UDSFrame& depacketized, const void* frame);
		/// 패킷을 파싱함 (멀티파티 전송모델 지원)

	static unsigned packetizeMultipartyLength(std::vector<uint8_t>& packetized, const std::size_t size, bool multiparty=true);
		/// 패킷을 생성함 (멀티파티 전송모델 지원)

protected:
	virtual bool disconnect();
		/// 연결을 종료함

	template<class Iterator>
	std::size_t  sendFrame(uint64_t messageSize, Iterator messageBegin, Iterator messageEnd);
		/// 프레임을 전송함
	
	virtual void recvFrame();
		/// 프레임을 수신함

#ifdef _WIN32
	static unsigned long __stdcall recv(void* thisPtr);
#else
	static void* recv(void* thisPtr);
#endif
		/// 스레드를 활용해 비동기로 프레임을 수신함

protected:
    void*		_user;				/// 유저 데이터
	onOpen		_onOpen;			/// 연결한 경우 호출하는 콜백
	onClose		_onClose;			/// 종료한 경우 호출하는 콜백
	onBinary	_onBinary;			/// 프레임 수신한 경우 호출하는 콜백
	onError		_onError;			/// 에러가 발생한 경우 호출하는 콜백

	socket_t	_server;			/// 서버 소켓
	socket_t	_client;			/// 클라이언트 소켓
	sockbuff	_rxbuf;				/// 수신 버퍼
	sockbuff	_binary;			/// 수신 메시지
	std::string	_uri;				/// 연결한 경로와 파일명
#ifdef _WIN32
	std::mutex	_mutex;				/// 멀티 스레드 환경에서 전송할 때 동기화
#else
	Mutex		_mutex;
#endif
	Thread		_recvThread;		/// 비동기 수신을 위한 스레드 핸들
	bool		_unlinkFile;		/// 서버 소켓인 경우 종료할 때 연결한 경로의 파일을 삭제함

	int			_reduceWindowSize;	/// 한 번에 전송하는 사이즈를 지정한 만큼 줄임
	bool		_calledCallback;	/// _onBinary 콜백을 호출한 상태임
	bool		_closedInCallback;	/// _onBinary 콜백 안에서 close()를 호출하였음
	bool		_calledClose;		/// close()를 명시적으로 호출한 경우 recv()에 대한 onError를 발생하지 않음
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

