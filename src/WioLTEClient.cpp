#include "WioLTEConfig.h"
#include "WioLTEClient.h"

#define RECEIVE_MAX_LENGTH	(1500)

#define CONNECT_SUCCESS				(1)
#define CONNECT_TIMED_OUT			(-1)
#define CONNECT_INVALID_SERVER		(-2)
#define CONNECT_TRUNCATED			(-3)
#define CONNECT_INVALID_RESPONSE	(-4)

WioLTEClient::WioLTEClient(WioLTE* wio)
{
	_Wio = wio;
	_ConnectId = -1;
	_ReceiveBuffer = new byte[RECEIVE_MAX_LENGTH];
}

WioLTEClient::~WioLTEClient()
{
	delete [] _ReceiveBuffer;
}

int WioLTEClient::connect(IPAddress ip, uint16_t port)
{
	if (connected()) return CONNECT_INVALID_RESPONSE;	// Already connected.

	String ipStr = String(ip[0]);
	ipStr += ".";
	ipStr += String(ip[1]);
	ipStr += ".";
	ipStr += String(ip[2]);
	ipStr += ".";
	ipStr += String(ip[3]);
	int connectId = _Wio->SocketOpen(ipStr.c_str(), port, WioLTE::SOCKET_TCP);
	if (connectId < 0) return CONNECT_INVALID_SERVER;
	_ConnectId = connectId;

	return CONNECT_SUCCESS;
}

int WioLTEClient::connect(const char* host, uint16_t port)
{
	if (connected()) return CONNECT_INVALID_RESPONSE;	// Already connected.

	int connectId = _Wio->SocketOpen(host, port, WioLTE::SOCKET_TCP);
	if (connectId < 0) return CONNECT_INVALID_SERVER;
	_ConnectId = connectId;

	return CONNECT_SUCCESS;
}

size_t WioLTEClient::write(uint8_t data)
{
	if (!connected()) return 0;

	if (!_Wio->SocketSend(_ConnectId, &data, 1)) return 0;

	return 1;
}

size_t WioLTEClient::write(const uint8_t* buf, size_t size)
{
	if (!connected()) return 0;

	if (!_Wio->SocketSend(_ConnectId, buf, size)) return 0;

	return size;
}

int WioLTEClient::available()
{
	if (!connected()) return 0;

	int receiveSize = _Wio->SocketReceive(_ConnectId, _ReceiveBuffer, RECEIVE_MAX_LENGTH);
	for (int i = 0; i < receiveSize; i++) _ReceiveQueue.push(_ReceiveBuffer[i]);

	return _ReceiveQueue.size();
}

int WioLTEClient::read()
{
	if (!connected()) return -1;

	int actualSize = available();
	if (actualSize <= 0) return -1;	// None is available.

	byte data = _ReceiveQueue.front();
	_ReceiveQueue.pop();

	return data;
}

int WioLTEClient::read(uint8_t* buf, size_t size)
{
	if (!connected()) return 0;

	int actualSize = available();
	if (actualSize <= 0) return 0;	// None is available.

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
	return _ConnectId >= 0 ? true : false;
}
