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

#include <string>

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
	typedef void(*onOpen)(const std::string& url);
	typedef void(*onClose)(const std::string& url);
	typedef void(*onError)(const std::string& msg, const int errNo);

	UDSSocket(onOpen open, onClose close, onError error);
	virtual ~UDSSocket();

	bool listen(const std::string& pathAndFilename);
	bool listening() const;
	bool open(const std::string& pathAndFilename);
	bool opened() const;
	bool close();

protected:
	virtual bool disconnect();

	onOpen		_onOpen;
	onClose		_onClose;
	onError		_onError;

	socket_t	_server;
	socket_t	_client;
	std::string	_uri;

	bool		_unlinkFile;
};

inline bool UDSSocket::listening() const
{
	return _server != -1;
}

inline bool UDSSocket::opened() const
{
	return _client != -1;
}