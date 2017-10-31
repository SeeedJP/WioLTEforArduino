#include <Arduino.h>
#include "WioLTEClient.h"

WioLTEClient::WioLTEClient(WioLTE* wio)
{
	_Wio = wio;
	_ConnectId = -1;
}

int WioLTEClient::connect(IPAddress ip, uint16_t port)
{
	return -2;	// INVALID_SERVER
}

int WioLTEClient::connect(const char *host, uint16_t port)
{
	if (connected()) return -4;	// INVALID_RESPONSE

	int connectId = _Wio->SocketOpen(host, port, WioLTE::SOCKET_TCP);
	if (connectId < 0) return -2;	// INVALID_SERVER

	_ConnectId = connectId;

	return 1;	// SUCCESS
}

size_t WioLTEClient::write(uint8_t data)
{
	if (!connected()) return 0;

	if (!_Wio->SocketSend(_ConnectId, &data, 1)) return 0;

	return 1;
}

size_t WioLTEClient::write(const uint8_t *buf, size_t size)
{
	if (!connected()) return 0;

	if (!_Wio->SocketSend(_ConnectId, buf, size)) return 0;

	return size;
}

int WioLTEClient::available()
{
	if (!connected()) return 0;

	byte data[1500];
	int dataSize = _Wio->SocketReceive(_ConnectId, data, sizeof (data));
	for (int i = 0; i < dataSize; i++) _ReceiveQueue.push(data[i]);

	return _ReceiveQueue.size();
}

int WioLTEClient::read()
{
	if (!connected()) return 0;

	int actualSize = available();
	if (actualSize <= 0) return -1;	// None is available.

	byte data = _ReceiveQueue.front();
	_ReceiveQueue.pop();

	return data;
}

int WioLTEClient::read(uint8_t *buf, size_t size)
{
	if (!connected()) return 0;

	int actualSize = available();
	if (actualSize <= 0) return -1;	// None is available.

	int popSize = actualSize <= size ? actualSize : size;
	for (int i = 0; i < popSize; i++) {
		buf[i] = _ReceiveQueue.front();
		_ReceiveQueue.pop();
	}

	return popSize;
}

int WioLTEClient::peek()
{
	if (!connected()) return 0;

	int actualSize = available();
	if (actualSize <= 0) return -1;	// None is available.

	return _ReceiveQueue.front();
}

void WioLTEClient::flush()
{
	// Nothing to do.
}

void WioLTEClient::stop()
{
	if (!connected()) return;

	_Wio->SocketClose(_ConnectId);
	_ConnectId = -1;
	while (!_ReceiveQueue.empty()) _ReceiveQueue.pop();
}

uint8_t WioLTEClient::connected()
{
	return _ConnectId >= 0 ? true : false;
}

WioLTEClient::operator bool()
{
	// TODO
	return false;
}
