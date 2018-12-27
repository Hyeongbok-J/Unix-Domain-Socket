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

#include <fstream>
#include <iterator>
#include <iostream>

#include "LT_EOL_Func.h"
#include "Def_Communication.h"
#include "Def_EOLMessage.h"

#include "UDSSocket.h"


static std::string format(const std::string fmt, ...)
	/// 문자열 포맷함
{
	int size = ((int)fmt.size()) * 2 + 50;
	std::string str;
	va_list ap;
	while (1) {				// Maximum two passes on a POSIX system...
		str.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
		va_end(ap);
		if (n > -1 && n < size) {  // Everything worked
			str.resize(n);
			return str;
		}
		if (n > -1)			// Needed size returned
			size = n + 1;   // For null char
		else
			size *= 2;      // Guess at a larger size (OS specific)
	}
	return str;
}

std::size_t load(std::vector<uint8_t>& binary, const char* filename)
	/// 파일을 버퍼를 로드함
{
	std::size_t loadedSize = 0;
	try
	{
		// 파일을 오픈함
		std::ifstream file(filename, std::ios::binary);

		// 바이너리 모드에서 뉴 라인을 중지함
		file.unsetf(std::ios::skipws);

		// 파일 크기를 구함
		std::streampos fileSize;

		file.seekg(0, std::ios::end);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		// 버퍼 크기를 지정함
		binary.reserve(fileSize);

		// 파일을 읽음
		binary.insert(binary.begin(),
			std::istream_iterator<uint8_t>(file),
			std::istream_iterator<uint8_t>());

		loadedSize = binary.size();
	}
	catch (std::exception& exc) {
		printf("load: %s\n", exc.what());
	}
	catch (...) {
		printf("load: unknown error\n");
	}
	return loadedSize;
}

struct UserData
	/// 사용자 데이터를 정의함
{
	int			imageWidth;
	int			imageHeight;
	UDSSocket* 	udsSocket;
};

void onOpen(void* user, const std::string& url)
{
	UserData* userData = (UserData*)user;
	std::string msg = format("onOpen: url=%s\n", url.c_str());
	printf(msg.c_str());
}

void onClose(void* user, const std::string& url)
{
	UserData* userData = (UserData*)user;
	std::string msg = format("onClose: url=%s\n", url.c_str());
	printf(msg.c_str());
}

std::size_t sendUart(const std::vector<uint8_t>& binary)
{
	// FixMe@181123: UART로 보내는 구현이 필요함
	ST_EOL_Command_Opt* stMsg = (ST_EOL_Command_Opt*)binary.data();
	printf("sendUart: id1=0x%X, id2=0x%X, size=%d\n",
		stMsg->id1, stMsg->id2, (int)binary.size());
	return binary.size();
}

bool sendUartMultiparty(uint8_t id1, uint8_t id2, const void* data=0, const std::size_t size=0)
{
	const int MAX_HDR = 5; // id1(1), id2(1), len(3), reserved(1)
	const int MAX_MULTIPARTY = 256 - MAX_HDR; // 250

	bool multiparty = false;
	std::vector<uint8_t> txbuf;
	txbuf.insert(txbuf.end(), (char*)data, (char*)data + size);
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
		binary.insert(binary.end(), (const char*)&id1, (const char*)&id1 + sizeof(id1));
		binary.insert(binary.end(), (const char*)&id2, (const char*)&id2 + sizeof(id2));
		binary.insert(binary.end(), len.begin(), len.end());
		binary.insert(binary.end(), txbuf.begin(), txbuf.begin() + messageSize);

		// UART로 바이너리를 전송함
		std::size_t bytesSent = sendUart(binary);
		if (bytesSent != binary.size()) {
			return false;
		}

		// 보낸 메시지 만큼 송신버퍼에서 삭제함
		if (txbuf.size())
			txbuf.erase(txbuf.begin(), txbuf.begin() + messageSize);
	} while (txbuf.size());

	return true;
}

bool onImage(void* user, const void* binary, const int size);
bool onEOLMode(void* user, const void* binary, const int size);
bool onIRLED(void* user, const void* binary, const int size);
bool onReadEEPROM(void* user, const void* binary, const int size);
bool onWriteEEPROM(void* user, const void* binary, const int size);
void onBinary(void* user, const std::vector<uint8_t>& binary)
{
	UserData* userData = (UserData*)user;
	printf("onBinary=%d\n", (int)binary.size());

	ST_EOL_Command_Opt* stMsg = (ST_EOL_Command_Opt*)binary.data();
	if (stMsg->id1 == EOL_ID1_CMD) 
	{
		if (false) {}
		else if (stMsg->id2 == EOL_ID2_CAPTURE_IMG) {
			onImage(user, stMsg->data, 0);
		}
		else if (stMsg->id2 == EOL_ID2_EOL_SWITCH) {
			onEOLMode(user, stMsg->data, 0);
		}
		else if (stMsg->id2 == EOL_ID2_IRLED_SWITCH) {
			onIRLED(user, stMsg->data, 0);
		}
		else if (stMsg->id2 == EOL_ID2_READ_EEPROM) {
			onReadEEPROM(user, stMsg->data, 0);
		}
		else if (stMsg->id2 == EOL_ID2_WRITE_EEPROM) {
			onWriteEEPROM(user, stMsg->data, 0);
		}
	}
	else if (stMsg->id1 == EOL_ID1_UART_CMD || 
			 stMsg->id1 == EOL_ID1_UART_ACK || 
			 stMsg->id1 == EOL_ID1_UART_NCK) 
	{
		// UART로 릴레이 전송함
		const char* payload = (char*)binary.data() + sizeof(ST_EOL_Command_Opt);
		const std::size_t payloadSize = binary.size() - sizeof(ST_EOL_Command_Opt);
		bool sent = sendUartMultiparty(stMsg->id1, stMsg->id2, payload, payloadSize);
		printf("relayed to Uart: sent=%d, id1=0x%X, id2=0x%X, size=%d\n",
			(int)sent, stMsg->id1, stMsg->id2, (int)binary.size());
	}
}

void onError(void* user, const std::string& errMsg, const int errCode)
{
	std::string msg = format("onError: errCode=%d, errMsg=%s\n", errCode, errMsg.c_str());
	UserData* userData = (UserData*)user;
	printf(msg.c_str());
}

int main(void)
{
#ifdef _WIN32 // 윈도우 소켓을 사용할 것임
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		printf("WSAStartup Failed.\n");
		return 1;
	}
#endif

	UserData userData;
	userData.imageWidth = 1280;
	userData.imageHeight = 964;

	void* user = &userData;
	UDSSocket server(user, onOpen, onClose, onBinary, onError);
	userData.udsSocket = &server;

	printf("Input command[exit, listen, large, small, write, capture, close]:\n");

#ifdef WIN32
	std::string uds = "d:\\uds.1234";
#else
	std::string uds = "/tmp/uds.1234";
#endif
	bool prompt = true;
	while (prompt)
	{
		std::string msg;
		std::getline(std::cin, msg);
		if (msg == "exit") {
			prompt = false;
		} else if (msg == "listen") {
			if (!server.listen(uds.c_str())) {
		        printf("client: init error\n");
			}
		} else if (msg == "open") {
			if (!server.open(uds.c_str())) {
				printf("server: init error\n");
			}
#if false
		} else if (msg == "large") {
			printf("[UDS: Send Large Test]\n");
			const std::string filename = "./image/StereoCal/left/Step_02.bmp";
			std::vector<uint8_t> image;
			std::size_t size = load(image, filename.c_str());
			printf("send: file=%s, size=%d\n", filename.c_str(), (int)image.size());
			if (size && server.opened()) {
				bool sent = server.send(EOL_ID1_CMD, EOL_ID2_UART, image);
				printf("sent=%d\n", (int)sent);
			}
		}
		else if (msg == "small") {
			printf("[UDS: Send Small Test]\n");
			const std::string filename = "./image/NewChart.raw";
			std::vector<uint8_t> image;
			std::size_t size = load(image, filename.c_str());
			printf("send: file=%s, size=%d\n", filename.c_str(), (int)image.size());
			if (size && server.opened()) {
				bool sent = server.send(EOL_ID1_CMD, EOL_ID2_UART, image);
				printf("sent=%d\n", (int)sent);
			}
#endif
		} else if (msg == "write") {
			EOL_EEPROMCalib_Opt calibration;
			calibration.imageX = 1280.f;
			calibration.imageY = 964.f;
			calibration.master.focalX = -1497.054077f;
			calibration.master.focalY = -1497.054077f;
			calibration.master.principleX = 642.791687f;
			calibration.master.principleY = 466.003723f;
			calibration.master.k1 = -0.104099f;
			calibration.master.k2 = -0.253960f;
			calibration.master.k3 = 0.0f;
			calibration.master.k4 = 0.0f;
			calibration.master.k5 = 0.0f;
			calibration.slave.focalX = -1488.354614f;
			calibration.slave.focalY = -1488.354614f;
			calibration.slave.principleX = 647.019470f;
			calibration.slave.principleY = 470.398376f;
			calibration.slave.k1 = -0.102794f;
			calibration.slave.k2 = -0.290721f;
			calibration.slave.k3 = 0.0f;
			calibration.slave.k4 = 0.0f;
			calibration.slave.k5 = 0.0f;
			calibration.center.rotX = -0.396321f;
			calibration.center.rotY = 0.357470f;
			calibration.center.rotZ = 0.107100f;
			calibration.center.transX = -0.091872f;
			calibration.center.transY = 0.000565f;
			calibration.center.transZ = -0.002537f;

			ST_EOL_EEPROMWrite_Opt stMsg;
			stMsg.cmd = EOL_EEPROM_CALIBRATION;
			stMsg.dataLength = sizeof(calibration);

			std::vector<uint8_t> payload;
			payload.insert(payload.end(), (const char*)&stMsg, (const char*)&stMsg + sizeof(stMsg));
			payload.insert(payload.end(), (const char*)&calibration, (const char*)&calibration + sizeof(calibration));
			bool sent = server.send(EOL_ID1_CMD, EOL_ID2_WRITE_EEPROM, payload);
			printf("sent=%d\n", (int)sent);
		} else if (msg == "close") {
			if (server.opened())
				server.close();
		}
		else if (msg == "capture") {
			ST_EOL_ReqCapImg_Opt stMsg;
			stMsg.cmd = EOL_IMG_CAPTURE;
			bool sent = server.send(EOL_ID1_UART_CMD, EOL_ID2_CAPTURE_IMG, &stMsg, sizeof(stMsg));
		} else {

		}
	}

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}


/// capture image functions

bool onImageCapture()
	/// 이미지를 캡처함
{
	printf("onImageCapture: ACK returned...\n");
	return true;
}

bool onImageDelete()
	/// 이미지를 삭제함
{
	printf("onImageDelete: NCK returned..\n");
	return false;
}

bool onImage(void* user, const void* binary, const int size)
	/// 이미지 캡처/삭제를 요청함
{
	if (!binary)
		return false;

	UserData* userData = (UserData*)user;
	int width = userData->imageWidth;
	int height = userData->imageHeight;

	bool succeed = false;
	ST_EOL_ReqCapImg_Opt* opt = (ST_EOL_ReqCapImg_Opt*)binary;
	switch (opt->cmd) {
	case EOL_IMG_CAPTURE:	succeed = onImageCapture();	break;
	case EOL_IMG_DELETE:	succeed = onImageDelete();	break;
	}

	if (userData) {
		std::vector<uint8_t> binary;
		if (succeed) {
			ST_EOL_ResCapImgAck_Opt stRes;
			stRes.res = EOL_RES_LGE;

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_ACK, EOL_ID2_CAPTURE_IMG, &stRes, sizeof(stRes));
				printf("[UnixSocket] onImage: sent=%d\n", (int)sent);
			}
		}
		else {
			ST_EOL_NCK_Opt stRes;
			stRes.code = 0;

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_NCK, EOL_ID2_CAPTURE_IMG, &stRes, sizeof(stRes));
				printf("[UnixSocket] onImage: sent=%d\n", (int)sent);
			}
		}
	}
	else {
		printf("onImage: No connected\n");
	}
	return succeed;
}


/// EOL Mode functions

bool onEOLModeOn()
	/// EOLMode ON
{
	printf("onEOLModeOn: ACK returned...\n");
	return true;
}

bool onEOLModeOff()
	/// EOLMode OFF
{
	printf("onEOLModeOff: NCK returned..\n");
	return false;
}

bool onEOLModeExt(int cmd)
	/// EOLMode Extension Cmd
{
	printf("onEOLModeExt: ACK returned...\n");
	return true;
}

bool onEOLMode(void* user, const void* binary, const int size)
	/// EOL Mode ON/OFF 요청함
{
	if (!binary)
		return false;

	UserData* userData = (UserData*)user;
	int width = userData->imageWidth;
	int height = userData->imageHeight;

	bool succeed = false;
	ST_EOL_ReqMode_Opt* opt = (ST_EOL_ReqMode_Opt*)binary;
	switch (opt->cmd) {
	case EOL_MODE_ON:	succeed = onEOLModeOn();	break;
	case EOL_MODE_OFF:	succeed = onEOLModeOff();	break;
	default: succeed = onEOLModeExt((int)opt->cmd); break;
	}

	if (userData) {
		std::vector<uint8_t> binary;
		if (succeed) {
			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_ACK, EOL_ID2_EOL_SWITCH);
				printf("[UnixSocket] onEOLMode: sent=%d\n", (int)sent);
			}
		}
		else {
			ST_EOL_NCK_Opt stRes;
			stRes.code = 0;

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_NCK, EOL_ID2_EOL_SWITCH, &stRes, sizeof(stRes));
				printf("[UnixSocket] onEOLMode: sent=%d\n", (int)sent);
			}
		}
	}
	else {
		printf("onEOLMode: No connected\n");
	}
	return succeed;
}


/// IRLED control functions

bool onIRLEDControl(int cmd)
	/// IRLED control
{
	printf("onIRLED: cmd=%d, ACK returned...\n", cmd);
	return true;
}

bool onIRLED(void* user, const void* binary, const int size)
	/// EOL Mode ON/OFF 요청함
{
	if (!binary)
		return false;

	UserData* userData = (UserData*)user;
	int width = userData->imageWidth;
	int height = userData->imageHeight;

	bool succeed = false;
	ST_EOL_IRLED_Opt* opt = (ST_EOL_IRLED_Opt*)binary;
	succeed = onIRLEDControl((int)opt->cmd);;

	if (userData) {
		std::vector<uint8_t> binary;
		if (succeed) {
			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_ACK, EOL_ID2_IRLED_SWITCH);
				printf("[UnixSocket] onIRLED: sent=%d\n", (int)sent);
			}
		}
		else {
			ST_EOL_NCK_Opt stRes;
			stRes.code = 0;

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_NCK, EOL_ID2_IRLED_SWITCH, &stRes, sizeof(stRes));
				printf("[UnixSocket] onIRLED: sent=%d\n", (int)sent);
			}
		}
	}
	else {
		printf("onIRLED: No connected\n");
	}
	return succeed;
}


/// EEPROM read functions

bool onReadAddress(uint8_t cmd, std::vector<uint8_t>& data)
	/// EEPROM 읽기
{
	bool read = false;
	if (cmd == EOL_EEPROM_CALIBRATION) {
		EOL_EEPROMCalib_Opt calibration;
		calibration.imageX = 1280.f;
		calibration.imageY = 964.f;
		calibration.master.focalX = -1497.054077f;
		calibration.master.focalY = -1497.054077f;
		calibration.master.principleX = 642.791687f;
		calibration.master.principleY = 466.003723f;
		calibration.master.k1 = -0.104099f;
		calibration.master.k2 = -0.253960f;
		calibration.master.k3 = 0.0f;
		calibration.master.k4 = 0.0f;
		calibration.master.k5 = 0.0f;
		calibration.slave.focalX = -1488.354614f;
		calibration.slave.focalY = -1488.354614f;
		calibration.slave.principleX = 647.019470f;
		calibration.slave.principleY = 470.398376f;
		calibration.slave.k1 = -0.102794f;
		calibration.slave.k2 = -0.290721f;
		calibration.slave.k3 = 0.0f;
		calibration.slave.k4 = 0.0f;
		calibration.slave.k5 = 0.0f;
		calibration.center.rotX = -0.396321f;
		calibration.center.rotY = 0.357470f;
		calibration.center.rotZ = 0.107100f;
		calibration.center.transX = -0.091872f;
		calibration.center.transY = 0.000565f;
		calibration.center.transZ = -0.002537f;
		data.insert(data.end(), (const char*)&calibration, (const char*)&calibration + sizeof(calibration));
		read = true;
	}
	else if (cmd == EOL_EEPROM_DISPLAY_CENTER) {
		// not implements
	}
	else if (cmd == EOL_EEPROM_BARCODE) {
		// not implements
	}
	printf("onReadAddress: cmd=%d, dataLength=%d, ACK returned...\n", cmd, (int)data.size());
	return read;
}

bool onReadEEPROM(void* user, const void* binary, const int size)
	/// EEPROM 읽기를 요청함
{
	if (!binary)
		return false;

	UserData* userData = (UserData*)user;
	int width = userData->imageWidth;
	int height = userData->imageHeight;

	bool succeed = false;
	std::vector<uint8_t> data;
	ST_EOL_EEPROMRead_Opt* opt = (ST_EOL_EEPROMRead_Opt*)binary;
	succeed = onReadAddress(opt->cmd, data);

	if (userData) {
		if (succeed) {
			ST_EOL_ResEEPROMReadAck_Opt stRes;
			stRes.cmd = opt->cmd;
			stRes.dataLength = (uint16_t)data.size();

			std::vector<uint8_t> payload;
			payload.insert(payload.end(), (const char*)&stRes, (const char*)&stRes + sizeof(stRes));
			payload.insert(payload.end(), data.begin(), data.end());

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_ACK, EOL_ID2_READ_EEPROM, payload);
				printf("[UnixSocket] onReadEEPROM: sent=%d\n", (int)sent);
			}
		}
		else {
			ST_EOL_NCK_Opt stRes;
			stRes.code = 0;

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_NCK, EOL_ID2_READ_EEPROM, &stRes, sizeof(stRes));
				printf("[UnixSocket] onReadEEPROM: sent=%d\n", (int)sent);
			}
		}
	}
	else {
		printf("onReadEEPROM: No connected\n");
	}
	return succeed;
}


/// EEPROM write functions

bool onWriteAddress(uint8_t cmd, uint16_t dataLength, uint8_t* data)
	/// EEPROM에 쓰기
{
	printf("onWriteAddress: cmd=%d, dataLength=%d, ACK returned...\n", cmd, dataLength);
	return true;
}

bool onWriteEEPROM(void* user, const void* binary, const int size)
	/// EEPROM 쓰기를 요청함
{
	if (!binary)
		return false;

	UserData* userData = (UserData*)user;
	int width = userData->imageWidth;
	int height = userData->imageHeight;

	bool succeed = false;
	ST_EOL_EEPROMWrite_Opt* opt = (ST_EOL_EEPROMWrite_Opt*)binary;
	succeed = onWriteAddress(opt->cmd, opt->dataLength, opt->data);

	if (userData) {
		if (succeed) {
			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_ACK, EOL_ID2_WRITE_EEPROM);
				printf("[UnixSocket] onWriteEEPROM: sent=%d\n", (int)sent);
			}
		}
		else {
			ST_EOL_NCK_Opt stRes;
			stRes.code = 0;

			if (userData->udsSocket && userData->udsSocket->opened()) {
				bool sent = userData->udsSocket->send(EOL_ID1_NCK, EOL_ID2_WRITE_EEPROM, &stRes, sizeof(stRes));
				printf("[UnixSocket] onWriteEEPROM: sent=%d\n", (int)sent);
			}
		}
	}
	else {
		printf("onWriteEEPROM: No connected\n");
	}
	return succeed;
}
