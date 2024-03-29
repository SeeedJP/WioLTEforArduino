#pragma once

#include "WioLTEConfig.h"
#include "Internal/AtSerial.h"
#if defined ARDUINO_ARCH_STM32F4
#include <Seeed_ws2812.h>
#elif defined ARDUINO_ARCH_STM32
#include "Internal/WioSK6812.h"
#endif
#include <time.h>
#include <functional>
#include "WioLTEHttpHeader.h"

#define WIOLTE_TCP	(WioLTE::SOCKET_TCP)
#define WIOLTE_UDP	(WioLTE::SOCKET_UDP)

#define WIOLTE_D38	(WioLTE::D38)
#define WIOLTE_D39	(WioLTE::D39)
#define WIOLTE_D20	(WioLTE::D20)
#define WIOLTE_D19	(WioLTE::D19)
#define WIOLTE_A6	(WioLTE::A6)
#define WIOLTE_A7	(WioLTE::A7)
#define WIOLTE_A4	(WioLTE::A4)
#define WIOLTE_A5	(WioLTE::A5)

#define WIO_TCP		(WioLTE::SOCKET_TCP)
#define WIO_UDP		(WioLTE::SOCKET_UDP)

#define WIO_D38		(WioLTE::D38)
#define WIO_D39		(WioLTE::D39)
#define WIO_D20		(WioLTE::D20)
#define WIO_D19		(WioLTE::D19)
#define WIO_A6		(WioLTE::A6)
#define WIO_A7		(WioLTE::A7)
#define WIO_A4		(WioLTE::A4)
#define WIO_A5		(WioLTE::A5)

class WioLTE
{
public:
	enum ErrorCodeType {
		E_OK = 0,
		E_UNKNOWN,
		E_TIMEOUT,
		E_GNSS_NOT_FIXED,
		E_UNABLE_TO_RECEIVE_RDY,
		E_NO_AT_RESPONSE,
		E_PIN,
		E_UNABLE_TO_RECEIVE_POWERED_DOWN,
		E_RESPONSE_TOO_LONG,
		E_NOT_ENOUGH_SIZE,
		E_INVALID_RESPONSE,
		E_INVALID_ARGUMENT,
		E_NOT_ENOUGH_RESOURCE,
	};

	enum SocketType {
		SOCKET_TCP,
		SOCKET_UDP,
	};

public:
	// D38 connector
	static const int D38 = 38;
	static const int D39 = 39;

	// D20 connector
	static const int D20 = 20;
	static const int D19 = 19;

	// A6 connector
	static const int A6 = 6;
	static const int A7 = 7;

	// A4 connector
	static const int A4 = 4;
	static const int A5 = 5;

private:
	SerialAPI _SerialAPI;
	AtSerial _AtSerial;
#if defined ARDUINO_ARCH_STM32F4
	WS2812 _Led;
#elif defined ARDUINO_ARCH_STM32
	WioSK6812 _Led;
#endif
	ErrorCodeType _LastErrorCode;
	std::function<void(int)>_Delay;

	bool _PacketGprsNetworkRegistration;
	bool _PacketEpsNetworkRegistration;

private:
	bool ReturnOk(bool value)
	{
		_LastErrorCode = E_OK;
		return value;
	}
	int ReturnOk(int value)
	{
		_LastErrorCode = E_OK;
		return value;
	}
	bool ReturnError(int lineNumber, bool value, ErrorCodeType errorCode);
	int ReturnError(int lineNumber, int value, ErrorCodeType errorCode);

	bool IsRespond();
	bool Reset(long timeout);
	bool TurnOn(long timeout);

	int GetFirstIndexOfReceivedSMS();

	bool HttpSetUrl(const char* url);

public:
	bool ReadResponseCallback(const char* response);	// Internal use only.

public:
	WioLTE();
	ErrorCodeType GetLastError() const;
	void SetDelayFunction(std::function<void(int)> func);
	void SetDoWorkInWaitForAvailableFunction(std::function<void()> func);
	void Init();
	void PowerSupplyLTE(bool on);						// Keep compatibility
	void PowerSupplyCellular(bool on);
	void PowerSupplyGNSS(bool on);
	void PowerSupplyLed(bool on);
	void PowerSupplyGrove(bool on);
	void PowerSupplySD(bool on);
	void LedSetRGB(byte red, byte green, byte blue);
	bool TurnOnOrReset(long timeout = 12000);
	bool TurnOff(long timeout = 60000);
	bool Sleep();
	bool Wakeup();

	int GetRevision(char* revision, int revisionSize);
	int GetIMEI(char* imei, int imeiSize);
	int GetIMSI(char* imsi, int imsiSize);
	int GetICCID(char* iccid, int iccidSize);
	int GetPhoneNumber(char* number, int numberSize);
	int GetReceivedSignalStrength();
	bool GetTime(struct tm* tim);

	//bool Call(const char* dialNumber);
	//void IsRinging();
	//void Answer();
	//bool HangUp();

	bool SendSMS(const char* dialNumber, const char* message);
	int ReceiveSMS(char* message, int messageSize, char* dialNumber = NULL, int dialNumberSize = 0);
	bool DeleteReceivedSMS();

	bool WaitForCSRegistration(long timeout = 120000);
	bool WaitForPSRegistration(long timeout = 120000);
	bool Activate(const char* accessPointName, const char* userName, const char* password, long waitForRegistTimeout = 120000);
	bool Deactivate();

	bool SyncTime(const char* host);
	bool GetLocation(double* longitude, double* latitude);

	int SocketOpen(const char* host, int port, SocketType type);
	bool SocketSend(int connectId, const byte* data, int dataSize);
	bool SocketSend(int connectId, const char* data);
	int SocketReceive(int connectId, byte* data, int dataSize);
	int SocketReceive(int connectId, char* data, int dataSize);
	int SocketReceive(int connectId, byte* data, int dataSize, long timeout);
	int SocketReceive(int connectId, char* data, int dataSize, long timeout);
	bool SocketClose(int connectId);

	int HttpGet(const char* url, char* data, int dataSize, long timeout = 60000);
	int HttpGet(const char* url, char* data, int dataSize, const WioLTEHttpHeader& header, long timeout = 60000);
	bool HttpPost(const char* url, const char* data, int* responseCode, long timeout = 60000);
	bool HttpPost(const char* url, const char* data, int* responseCode, const WioLTEHttpHeader& header, long timeout = 60000);

	// GNSS functionality (may not work on JP boards)
	bool EnableGNSS(long timeout = 60000);
	bool DisableGNSS();
	bool GetGNSSLocation(double* longitude, double* latitude, double* altitude = NULL, struct tm* tim = NULL);

public:
	static void SystemReset();

};

typedef WioLTE WioCellular;
