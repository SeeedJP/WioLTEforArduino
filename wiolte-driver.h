#pragma once

#include <Arduino.h>

#define WIOLTE_TCP	(WioLTE::SOCKET_TCP)
#define WIOLTE_UDP	(WioLTE::SOCKET_UDP)

class WioLTE
{
private:
	static const int MODULE_PWR_PIN = 18; // PB5
	static const int ANT_PWR_PIN = 28; // PB12
	static const int ENABLE_VCCB_PIN = 26; // PB10    

	static const int PWR_KEY_PIN = 36; // PC4 
	static const int WAKEUP_IN_PIN = 32; // PC0
	static const int AP_READY_PIN = 33; // PC1
	static const int WAKEUP_DISABLE_PIN = 34; // PC2
	static const int RESET_MODULE_PIN = 35; // PC3
	static const int STATUS_PIN = 31; // PB15
	static const int RGB_LED_PIN = 17; // PB1

private:
	HardwareSerial* _Serial;

private:
	void DiscardRead();
	bool ReadLine(char* data, int dataSize, long timeout);

public:
	enum SocketType {
		SOCKET_TCP,
		SOCKET_UDP,
	};

public:
	void Write(const char* str);
	void WriteCommand(const char* command);
	bool WaitForResponse(const char* response, long timeout);
	bool WaitForResponse(const char* response, char* parameter, int parameterSize, long timeout);
	bool WriteCommandAndWaitForResponse(const char* command, const char* response, long timeout);

	bool Reset();
	bool TurnOn();

public:
	WioLTE();
	void Init();
	void PowerSupplyLTE(bool on);
	void PowerSupplyGNSS(bool on);
	void PowerSupplyGrove(bool on);
	bool IsBusy() const;
	bool TurnOnOrReset();

	bool SendSMS(const char* dialNumber, const char* message);

	bool Activate(const char* accessPointName, const char* userName, const char* password);

	int SocketOpen(const char* host, int port, SocketType type);
	bool SocketSend(int connectId, const char* data);
	int SocketReceive(int connectId, byte* data, int dataSize);
	int SocketReceive(int connectId, char* data, int dataSize);
	bool SocketClose(int connectId);

public:
	class Stopwatch
	{
	private:
		unsigned long _BeginTime;
		unsigned long _EndTime;

	public:
		Stopwatch() : _BeginTime(0), _EndTime(0)
		{
		}

		void Start()
		{
			_BeginTime = millis();
			_EndTime = 0;
		}

		void Stop()
		{
			_EndTime = millis();
		}

		unsigned long ElapsedMilliseconds()
		{
			if (_EndTime == 0) {
				return millis() - _BeginTime;
			}
			else {
				return _EndTime - _BeginTime;
			}
		}
	};

};
