#pragma once

#include "WioLTE.h"
#include <Client.h>
#include <queue>

class WioLTEClient : public Client {

protected:
	WioLTE* _Wio;
	int _ConnectId;
	std::queue<byte> _ReceiveQueue;
	byte* _ReceiveBuffer;

public:
	WioLTEClient(WioLTE* wio);
	virtual ~WioLTEClient();

	virtual int connect(IPAddress ip, uint16_t port);
	virtual int connect(const char* host, uint16_t port);
	virtual size_t write(uint8_t data);
	virtual size_t write(const uint8_t* buf, size_t size);
	virtual int available();
	virtual int read();
	virtual int read(uint8_t* buf, size_t size);
	virtual int peek();
	virtual void flush();
	virtual void stop();
	virtual uint8_t connected();
	virtual operator bool();

};
