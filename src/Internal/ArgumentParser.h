#pragma once

#include <vector>

class ArgumentParser
{
private:
	std::vector< std::vector<char> > _Arguments;

public:
	ArgumentParser();
	void Parse(const char* str);
	int Size() const;
	const char* operator[](int index) const;

};
