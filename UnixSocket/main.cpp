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

#include <fstream>
#include <iostream>
#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#include "UnixSocket.h"

void onOpen(const std::string& url)
{
	printf("onOpen: url=%s\n", url.c_str());
}

void onClose(const std::string& url)
{
	printf("onClose: url=%s\n", url.c_str());
}

void onError(const std::string& errMsg, const int errCode)
{
	printf("onError: errCode=%d, errMsg=%s\n", errCode, errMsg.c_str());
}

int main(void)
{
#ifdef _WIN32
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		printf("WSAStartup Failed.\n");
		return 1;
	}
#endif
	UDSSocket server(onOpen, onClose, onError);
	printf("Input command[listen, open, send, close, exit]:\n");
#ifdef WIN32
	std::string uds = "d://UnixSocket";
#else
	std::string uds = "/tmp/UnixSocket.sock";
#endif
	bool prompt = true;
	while (prompt)
	{
		std::string msg;
		std::getline(std::cin, msg);
		if (msg == "listen")
		{
			if (!server.listen(uds.c_str()))
				printf("client: init error\n");
		}
		else if (msg == "open")
		{
			if (!server.open(uds.c_str()))
				printf("server: init error\n");
		}
		else if (msg == "send")
		{
			server.sendTest();
		}
		else if (msg == "close")
		{
			if (server.opened())
				server.close();
		}
		else if (msg == "exit")
		{
			prompt = false;
		}
		else
		{

		}
	}
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}