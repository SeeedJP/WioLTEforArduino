#include "../WioLTEConfig.h"
#include "ArgumentParser.h"

ArgumentParser::ArgumentParser()
{
}

void ArgumentParser::Parse(const char* str)
{
	_Arguments.clear();

	// Search comma position.
	std::vector<const char*> commaList;
	bool inString = false;
	for (const char* ptr = str; *ptr != '\0'; ptr++) {
		if (!inString) {
			switch (*ptr) {
			case ',':
				commaList.push_back(ptr);
				break;
			case '"':
				inString = true;
				break;
			}
		}
		else {
			switch (*ptr) {
			case '"':
				inString = false;
				break;
			}
		}
	}

	// Build arguments.
	_Arguments.resize(commaList.size() + 1);
	for (int i = 0; i < _Arguments.size(); i++) {
		const char* begin = i == 0 ? str : commaList[i - 1] + 1;
		const char* end = i < commaList.size() ? commaList[i] : str + strlen(str);

		if (end - begin >= 2 && *begin == '"' && *(end - 1) == '"') {
			begin++;
			end--;
		}

		_Arguments[i].resize(end - begin + 1);
		memcpy(&_Arguments[i][0], begin, end - begin);
		_Arguments[i][end - begin] = '\0';
	}
}

int ArgumentParser::Size() const
{
	return _Arguments.size();
}

const char* ArgumentParser::operator[](int index) const
{
	return &_Arguments[index][0];
}
