#pragma once

#include <vector>

class StringBuilder
{
private:
	std::vector<char> _Buffer;

public:
	StringBuilder();
	void Clear();
	int Length() const;
	const char* GetString() const;
	void Write(const char* str);
	void Write(const char* str, int length);
	bool WriteFormat(const char* format, ...);

};
