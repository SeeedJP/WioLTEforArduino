#pragma once

class Stopwatch
{
private:
	unsigned long _BeginTime;
	unsigned long _EndTime;

public:
	Stopwatch() : _BeginTime(0), _EndTime(0)
	{
	}

	void Restart()
	{
		_BeginTime = millis();
		_EndTime = 0;
	}

	void Stop()
	{
		_EndTime = millis();
	}

	unsigned long ElapsedMilliseconds() const
	{
		if (_EndTime == 0) {
			return millis() - _BeginTime;
		}
		else {
			return _EndTime - _BeginTime;
		}
	}

};
