#pragma once

#include <Arduino.h>

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
	long _Timeout;

private:
	void DiscardRead();
	void SetTimeout(long timeout);
	bool ReadLine(char* data, int dataSize);

public:
	void Write(const char* str);
	void WriteCommand(const char* command);
	bool WaitForResponse(const char* response);
	bool WaitForResponse(const char* response, long timeout);
	bool WriteCommandAndWaitForResponse(const char* command, const char* response);
	bool WriteCommandAndWaitForResponse(const char* command, const char* response, long timeout);

public:
	WioLTE();
	void Init();
	void PowerSupplyLTE(bool on);
	void PowerSupplyGNSS(bool on);
	void PowerSupplyGrove(bool on);
	bool Reset();
	bool IsBusy() const;
	bool TurnOn();

	bool SendSMS(const char* dialNumber, const char* message);

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
