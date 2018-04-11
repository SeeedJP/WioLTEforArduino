#pragma once

class SerialAPI
{
private:
	HardwareSerial* _Serial;

public:
	SerialAPI(HardwareSerial* serial) : _Serial(serial) {}
	void Begin(int baud) { _Serial->begin(baud); }
	void Write(byte data) { _Serial->write(data); }
	bool Available() const { return _Serial->available() >= 1 ? true : false; }
	byte Read() { return _Serial->read(); }

};
