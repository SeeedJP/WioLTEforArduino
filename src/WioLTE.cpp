#include "WioLTEConfig.h"
#include "WioLTE.h"

#include "Internal/Debug.h"
#include "Internal/StringBuilder.h"
#include "Internal/ArgumentParser.h"
#include "Internal/CMSIS/cmsis_gcc.h"
#include "Internal/CMSIS/core_cm4.h"
#include <stdio.h>
#include <limits.h>

#define RET_OK(val)					(ReturnOk(val))
#define RET_ERR(val,err)			(ReturnError(__LINE__, val, err))

#define CONNECT_ID_NUM				(12)
#define POLLING_INTERVAL			(100)

#define HTTP_POST_USER_AGENT		"QUECTEL_MODULE"
#define HTTP_POST_CONTENT_TYPE		"application/json"

#define LINEAR_SCALE(val, inMin, inMax, outMin, outMax)	(((val) - (inMin)) / ((inMax) - (inMin)) * ((outMax) - (outMin)) + (outMin))

////////////////////////////////////////////////////////////////////////////////////////
// Helper functions

static void PinModeAndDefault(int pin, WiringPinMode mode)
{
	pinMode(pin, mode);
}

static void PinModeAndDefault(int pin, WiringPinMode mode, int value)
{
	pinMode(pin, mode);
	if (mode == OUTPUT) digitalWrite(pin, value);
}

static int HexToInt(char c)
{
	if ('0' <= c && c <= '9') return c - '0';
	else if ('a' <= c && c <= 'f') return c - 'a' + 10;
	else if ('A' <= c && c <= 'F') return c - 'A' + 10;
	else return -1;
}

static bool ConvertHexToBytes(const char* hex, byte* data, int dataSize)
{
	int high;
	int low;

	for (int i = 0; i < dataSize; i++) {
		high = HexToInt(hex[i * 2]);
		low = HexToInt(hex[i * 2 + 1]);
		if (high < 0 || low < 0) return false;
		data[i] = high * 16 + low;
	}

	return true;
}

static int NumberOfDigits(int value)
{
	int digits = 0;

	if (value < 0) {
		digits++;
		value *= -1;
	}

	do {
		digits++;
		value /= 10;
	} while (value > 0);

	return digits;
}

static bool SplitUrl(const char* url, const char** host, int* hostLength, const char** uri, int* uriLength)
{
	if (strncmp(url, "http://", 7) == 0) {
		*host = &url[7];
	}
	else if (strncmp(url, "https://", 8) == 0) {
		*host = &url[8];
	}
	else {
		return false;
	}

	const char* ptr;
	for (ptr = *host; *ptr != '\0'; ptr++) {
		if (*ptr == '/') break;
	}
	*hostLength = ptr - *host;
	*uri = ptr;
	*uriLength = strlen(ptr);

	return true;
}

static bool SmAddressFieldToString(const byte* addressField, char* str, int strSize)
{
	byte addressLength = addressField[0];
	byte typeOfAddress = addressField[1];
	const byte* addressValue = &addressField[2];

	if (addressLength + 1 > strSize) return false;

	for (int i = 0; i < addressLength; i++)
	{
		if (i % 2 == 0)
		{
			str[i] = '0' + (addressValue[i / 2] & 0x0f);
		}
		else
		{
			str[i] = '0' + (addressValue[i / 2] >> 4);
		}
	}
	str[addressLength] = '\0';

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
// WioLTE

bool WioLTE::ReturnError(int lineNumber, bool value, WioLTE::ErrorCodeType errorCode)
{
	_LastErrorCode = errorCode;

	char str[100];
	sprintf(str, "%d", lineNumber);
	DEBUG_PRINT("ERROR! ");
	DEBUG_PRINTLN(str);

	return value;
}

int WioLTE::ReturnError(int lineNumber, int value, WioLTE::ErrorCodeType errorCode)
{
	_LastErrorCode = errorCode;

	char str[100];
	sprintf(str, "%d", lineNumber);
	DEBUG_PRINT("ERROR! ");
	DEBUG_PRINTLN(str);

	return value;
}

bool WioLTE::IsBusy() const
{
	return digitalRead(STATUS_PIN) ? true : false;
}

bool WioLTE::Reset()
{
	digitalWrite(RESET_MODULE_PIN, LOW);
	delay(200);
	digitalWrite(RESET_MODULE_PIN, HIGH);
	delay(300);

	Stopwatch sw;
	sw.Restart();
	while (!_AtSerial.ReadResponse("^RDY$", 100, NULL)) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
}

bool WioLTE::TurnOn()
{
	delay(100);
	digitalWrite(PWR_KEY_PIN, HIGH);
	delay(200);
	digitalWrite(PWR_KEY_PIN, LOW);

	Stopwatch sw;
	sw.Restart();
	while (IsBusy()) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 5000) return false;
		delay(100);
	}
	DEBUG_PRINTLN("");

	sw.Restart();
	while (!_AtSerial.ReadResponse("^RDY$", 100, NULL)) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
}

int WioLTE::GetFirstIndexOfReceivedSMS()
{
	std::string response;
	ArgumentParser parser;

	if (!_AtSerial.WriteCommandAndReadResponse("AT+CMGF=0", "^OK$", 500, NULL)) return -1;

	_AtSerial.WriteCommand("AT+CMGL=4");	// ALL

	int messageIndex = -1;
	while (true) {
		if (!_AtSerial.ReadResponse("^(OK|\\+CMGL: .*)$", 500, &response)) return -1;
		if (response == "OK") break;
		if (messageIndex < 0) {
			parser.Parse(&response.c_str()[7]);
			if (parser.Size() != 4) return -1;
			messageIndex = atoi(parser[0]);
		}

		if (!_AtSerial.ReadResponse("^.*$", 500, NULL)) return -1;
	}

	return messageIndex < 0 ? -2 : messageIndex;
}

bool WioLTE::HttpSetUrl(const char* url)
{
	StringBuilder str;
	if (!str.WriteFormat("AT+QHTTPURL=%d", strlen(url))) return false;
	_AtSerial.WriteCommand(str.GetString());
	if (!_AtSerial.ReadResponse("^CONNECT$", 500, NULL)) return false;

	_AtSerial.WriteBinary((const byte*)url, strlen(url));
	if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return false;

	return true;
}

bool WioLTE::ReadResponseCallback(const char* response)
{
	return false;

	if (strncmp(response, "+CGREG: ", 8) == 0) {
		DEBUG_PRINT("### Response Callback +CGREG ### ");
		DEBUG_PRINTLN(response);

		//ArgumentParser parser;
		//parser.Parse(&response[8]);
		//int stat;
		//if (parser.Size() == 2) {
		//	stat = atoi(parser[1]);
		//}
		//else if (parser.Size() >= 1) {
		//	stat = atoi(parser[0]);
		//}
		//else {
		//	stat = -1;
		//}

		//switch (stat)
		//{
		//case 0:	// Not registered. MT is not currently searching
		//case 2:	// Not registered, but MT is currently trying to attach or searching
		//case 3:	// Registration denied.
		//	_PacketGprsNetworkRegistration = false;
		//	DEBUG_PRINTLN("NETWORK OFF");
		//	break;
		//case 1:	// Registered, home network.
		//case 5:	// Registered, roaming
		//	_PacketGprsNetworkRegistration = true;
		//	DEBUG_PRINTLN("NETWORK ON");
		//	break;
		//}

		return true;
	}
	else if (strncmp(response, "+CEREG: ", 8) == 0) {
		DEBUG_PRINT("### Response Callback +CEREG ### ");
		DEBUG_PRINTLN(response);

		//ArgumentParser parser;
		//parser.Parse(&response[8]);
		//int stat;
		//if (parser.Size() == 2) {
		//	stat = atoi(parser[1]);
		//}
		//else if (parser.Size() >= 1) {
		//	stat = atoi(parser[0]);
		//}
		//else {
		//	stat = -1;
		//}

		//switch (stat)
		//{
		//case 0:	// Not registered. MT is not currently searching
		//case 2:	// Not registered, but MT is currently trying to attach or searching
		//case 3:	// Registration denied
		//	_PacketEpsNetworkRegistration = false;
		//	DEBUG_PRINTLN("NETWORK OFF");
		//	break;
		//case 1:	// Registered, home network
		//case 5:	// Registered, roaming
		//	_PacketEpsNetworkRegistration = true;
		//	DEBUG_PRINTLN("NETWORK ON");
		//	break;
		//}

		return true;
	}

	return false;
}

WioLTE::WioLTE() : _SerialAPI(&Serial1), _AtSerial(&_SerialAPI, this), _Led(1, RGB_LED_PIN), _LastErrorCode(E_OK)
{
}

WioLTE::ErrorCodeType WioLTE::GetLastError() const
{
	return _LastErrorCode;
}

void WioLTE::Init()
{
	// Power supply
	PinModeAndDefault(MODULE_PWR_PIN, OUTPUT, LOW);
	PinModeAndDefault(ANT_PWR_PIN, OUTPUT, LOW);
	PinModeAndDefault(ENABLE_VCCB_PIN, OUTPUT, LOW);
#if defined WIOLTE_SCHEMATIC_B
	PinModeAndDefault(RGB_LED_PWR_PIN, OUTPUT, HIGH);
	PinModeAndDefault(SD_POWR_PIN, OUTPUT, LOW);
#endif // WIOLTE_SCHEMATIC_B

	// Turn on/off Pins
	PinModeAndDefault(PWR_KEY_PIN, OUTPUT, LOW);
	PinModeAndDefault(RESET_MODULE_PIN, OUTPUT, HIGH);

	// Status Indication Pins
	PinModeAndDefault(STATUS_PIN, INPUT);

	// UART Interface
	PinModeAndDefault(DTR_PIN, OUTPUT, LOW);

	// GPIO Pins
	PinModeAndDefault(WAKEUP_IN_PIN, OUTPUT, LOW);
	PinModeAndDefault(W_DISABLE_PIN, OUTPUT, HIGH);
	//PinModeAndDefault(AP_READY_PIN, OUTPUT);  // NOT use
  
	_SerialAPI.Begin(115200);
	_Led.begin();
	_LastErrorCode = E_OK;

	_PacketGprsNetworkRegistration = false;
	_PacketEpsNetworkRegistration = false;
}

void WioLTE::PowerSupplyLTE(bool on)
{
	digitalWrite(MODULE_PWR_PIN, on ? HIGH : LOW);
	_LastErrorCode = E_OK;
}

void WioLTE::PowerSupplyCellular(bool on)
{
	PowerSupplyLTE(on);
}

void WioLTE::PowerSupplyGNSS(bool on)
{
	digitalWrite(ANT_PWR_PIN, on ? HIGH : LOW);
	_LastErrorCode = E_OK;
}

void WioLTE::PowerSupplyLed(bool on)
{
#if defined WIOLTE_SCHEMATIC_B
	digitalWrite(RGB_LED_PWR_PIN, on ? HIGH : LOW);
#endif // WIOLTE_SCHEMATIC_B
	_LastErrorCode = E_OK;
}

void WioLTE::PowerSupplyGrove(bool on)
{
	digitalWrite(ENABLE_VCCB_PIN, on ? HIGH : LOW);
	_LastErrorCode = E_OK;
}

void WioLTE::PowerSupplySD(bool on)
{
#if defined WIOLTE_SCHEMATIC_B
	digitalWrite(SD_POWR_PIN, on ? HIGH : LOW);
#endif // WIOLTE_SCHEMATIC_B
	_LastErrorCode = E_OK;
}

void WioLTE::LedSetRGB(byte red, byte green, byte blue)
{
	_Led.WS2812SetRGB(0, red, green, blue);
	_Led.WS2812Send();
	_LastErrorCode = E_OK;
}

bool WioLTE::TurnOnOrReset()
{
	std::string response;

	if (!IsBusy()) {
		DEBUG_PRINTLN("Reset()");
		if (!Reset()) return RET_ERR(false, E_UNKNOWN);
	}
	else {
		DEBUG_PRINTLN("TurnOn()");
		if (!TurnOn()) return RET_ERR(false, E_UNKNOWN);
	}

	Stopwatch sw;
	sw.Restart();
	while (!_AtSerial.WriteCommandAndReadResponse("AT", "^OK$", 500, NULL)) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return RET_ERR(false, E_UNKNOWN);
	}
	DEBUG_PRINTLN("");

	if (!_AtSerial.WriteCommandAndReadResponse("ATE0", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse("AT+QURCCFG=\"urcport\",\"uart1\"", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse("AT+QSCLK=1", "^(OK|ERROR)$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	// TODO ReadResponseCallback
	//if (!_AtSerial.WriteCommandAndReadResponse("AT+CGREG=2", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	//if (!_AtSerial.WriteCommandAndReadResponse("AT+CEREG=2", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	//if (!_AtSerial.WriteCommandAndReadResponse("AT+CGREG?", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	//if (!_AtSerial.WriteCommandAndReadResponse("AT+CEREG?", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	sw.Restart();
	while (true) {
		if (!_AtSerial.WriteCommandAndReadResponse("AT+CPIN?", "^(OK|\\+CME ERROR: .*)$", 5000, &response)) return RET_ERR(false, E_UNKNOWN);
		if (response == "OK") break;
		if (sw.ElapsedMilliseconds() >= 10000) return RET_ERR(false, E_UNKNOWN);
		delay(POLLING_INTERVAL);
	}

	return RET_OK(true);
}

bool WioLTE::TurnOff()
{
	if (!_AtSerial.WriteCommandAndReadResponse("AT+QPOWD", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.ReadResponse("^POWERED DOWN$", 60000, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

bool WioLTE::Sleep()
{
	digitalWrite(DTR_PIN, HIGH);

	return RET_OK(true);
}

bool WioLTE::Wakeup()
{
	digitalWrite(DTR_PIN, LOW);

	Stopwatch sw;
	sw.Restart();
	while (!_AtSerial.WriteCommandAndReadResponse("AT", "^OK$", 500, NULL)) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 2000) return RET_ERR(false, E_UNKNOWN);
	}
	DEBUG_PRINTLN("");

	return RET_OK(true);
}

int WioLTE::GetIMEI(char* imei, int imeiSize)
{
	std::string response;
	std::string imeiStr;

	_AtSerial.WriteCommand("AT+GSN");
	while (true) {
		if (!_AtSerial.ReadResponse("^(OK|[0-9]+)$", 500, &response)) return RET_ERR(-1, E_UNKNOWN);
		if (response == "OK") break;
		imeiStr = response;
	}

	if ((int)imeiStr.size() + 1 > imeiSize) return RET_ERR(-1, E_UNKNOWN);
	strcpy(imei, imeiStr.c_str());

	return RET_OK((int)strlen(imei));
}

int WioLTE::GetIMSI(char* imsi, int imsiSize)
{
	std::string response;
	std::string imsiStr;

	_AtSerial.WriteCommand("AT+CIMI");
	while (true) {
		if (!_AtSerial.ReadResponse("^(OK|[0-9]+)$", 500, &response)) return RET_ERR(-1, E_UNKNOWN);
		if (response == "OK") break;
		imsiStr = response;
	}

	if ((int)imsiStr.size() + 1 > imsiSize) return RET_ERR(-1, E_UNKNOWN);
	strcpy(imsi, imsiStr.c_str());

	return RET_OK((int)strlen(imsi));
}

int WioLTE::GetPhoneNumber(char* number, int numberSize)
{
	std::string response;
	ArgumentParser parser;
	std::string numberStr;

	_AtSerial.WriteCommand("AT+CNUM");
	while (true) {
		if (!_AtSerial.ReadResponse("^(OK|\\+CNUM: .*)$", 500, &response)) return RET_ERR(-1, E_UNKNOWN);
		if (response == "OK") break;

		if (numberStr.size() >= 1) continue;

		parser.Parse(response.c_str());
		if (parser.Size() < 2) return RET_ERR(-1, E_UNKNOWN);
		numberStr = parser[1];
	}

	if ((int)numberStr.size() + 1 > numberSize) return RET_ERR(-1, E_UNKNOWN);
	strcpy(number, numberStr.c_str());

	return RET_OK((int)strlen(number));
}

int WioLTE::GetReceivedSignalStrength()
{
	std::string response;
	ArgumentParser parser;

	_AtSerial.WriteCommand("AT+CSQ");
	if (!_AtSerial.ReadResponse("^\\+CSQ: (.*)$", 500, &response)) return RET_ERR(INT_MIN, E_UNKNOWN);

	parser.Parse(response.c_str());
	if (parser.Size() != 2) return RET_ERR(INT_MIN, E_UNKNOWN);
	int rssi = atoi(parser[0]);

	if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(INT_MIN, E_UNKNOWN);

	if (rssi == 0) return RET_OK(-113);
	else if (rssi == 1) return RET_OK(-111);
	else if (2 <= rssi && rssi <= 30) return RET_OK((int)LINEAR_SCALE((double)rssi, 2, 30, -109, -53));
	else if (rssi == 31) return RET_OK(-51);
	else if (rssi == 99) return RET_OK(-999);
	else if (rssi == 100) return RET_OK(-116);
	else if (rssi == 101) return RET_OK(-115);
	else if (102 <= rssi && rssi <= 190) return RET_OK((int)LINEAR_SCALE((double)rssi, 102, 190, -114, -26));
	else if (rssi == 191) return RET_OK(-25);
	else if (rssi == 199) return RET_OK(-999);
	
	return RET_OK(-999);
}

bool WioLTE::GetTime(struct tm* tim)
{
	std::string response;

	_AtSerial.WriteCommand("AT+CCLK?");
	if (!_AtSerial.ReadResponse("^\\+CCLK: (.*)$", 500, &response)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	if (strlen(response.c_str()) != 22) return RET_ERR(false, E_UNKNOWN);
	const char* parameter = response.c_str();

	if (parameter[0] != '"') return RET_ERR(false, E_UNKNOWN);
	if (parameter[3] != '/') return RET_ERR(false, E_UNKNOWN);
	if (parameter[6] != '/') return RET_ERR(false, E_UNKNOWN);
	if (parameter[9] != ',') return RET_ERR(false, E_UNKNOWN);
	if (parameter[12] != ':') return RET_ERR(false, E_UNKNOWN);
	if (parameter[15] != ':') return RET_ERR(false, E_UNKNOWN);
	if (parameter[21] != '"') return RET_ERR(false, E_UNKNOWN);

	int yearOffset = atoi(&parameter[1]);
	tim->tm_year = (yearOffset >= 80 ? 1900 : 2000) + yearOffset - 1900;
	tim->tm_mon = atoi(&parameter[4]) - 1;
	tim->tm_mday = atoi(&parameter[7]);
	tim->tm_hour = atoi(&parameter[10]);
	tim->tm_min = atoi(&parameter[13]);
	tim->tm_sec = atoi(&parameter[16]);
	tim->tm_wday = 0;
	tim->tm_yday = 0;
	tim->tm_isdst = -1;

	return RET_OK(true);
}

bool WioLTE::SendSMS(const char* dialNumber, const char* message)
{
	if (!_AtSerial.WriteCommandAndReadResponse("AT+CMGF=1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+CMGS=\"%s\"", dialNumber)) return RET_ERR(false, E_UNKNOWN);
	_AtSerial.WriteCommand(str.GetString());
	if (!_AtSerial.ReadResponse("^> ", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	_AtSerial.WriteBinary((const byte*)message, strlen(message));
	_AtSerial.WriteBinary((const byte*)"\x1a", 1);
	if (!_AtSerial.ReadResponse("^OK$", 120000, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

int WioLTE::ReceiveSMS(char* message, int messageSize, char* dialNumber, int dialNumberSize)
{
	int messageIndex = GetFirstIndexOfReceivedSMS();
	if (messageIndex == -2) return RET_OK(0);
	if (messageIndex < 0) return RET_ERR(-1, E_UNKNOWN);

	if (!_AtSerial.WriteCommandAndReadResponse("AT+CMGF=0", "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+CMGR=%d", messageIndex)) return RET_ERR(-1, E_UNKNOWN);
	_AtSerial.WriteCommand(str.GetString());

	if (!_AtSerial.ReadResponse("^\\+CMGR: .*$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);

	std::string response;
	if (!_AtSerial.ReadResponse("^(.*)$", 500, &response)) return RET_ERR(-1, E_UNKNOWN);
	const char* hex = response.c_str();

	int hexSize = strlen(hex);
	if (hexSize % 2 != 0) return RET_ERR(-1, E_UNKNOWN);
	int dataSize = hexSize / 2;
	byte* data = (byte*)alloca(dataSize);
	if (!ConvertHexToBytes(hex, data, dataSize)) return RET_ERR(-1, E_UNKNOWN);
	byte* dataEnd = &data[dataSize];

	// http://www.etsi.org/deliver/etsi_gts/03/0340/05.03.00_60/gsmts_0340v050300p.pdf
	// http://www.etsi.org/deliver/etsi_gts/03/0338/05.00.00_60/gsmts_0338v050000p.pdf
	byte* smscInfoSize = data;
	byte* tpMti = smscInfoSize + 1 + *smscInfoSize;
	if (tpMti >= dataEnd) return RET_ERR(-1, E_UNKNOWN);
	if ((*tpMti & 0xc0) != 0) return RET_ERR(-1, E_UNKNOWN);	// SMS-DELIVER
	byte* tpOaSize = tpMti + 1;
	if (tpOaSize >= dataEnd) return RET_ERR(-1, E_UNKNOWN);
	byte* tpPid = tpOaSize + 2 + *tpOaSize / 2 + *tpOaSize % 2;
	if (tpPid >= dataEnd) return RET_ERR(-1, E_UNKNOWN);
	byte* tpDcs = tpPid + 1;
	if (tpDcs >= dataEnd) return RET_ERR(-1, E_UNKNOWN);
	byte* tpScts = tpDcs + 1;
	if (tpScts >= dataEnd) return RET_ERR(-1, E_UNKNOWN);
	byte* tpUdl = tpScts + 7;
	if (tpUdl >= dataEnd) return RET_ERR(-1, E_UNKNOWN);
	byte* tpUd = tpUdl + 1;
	if (tpUd >= dataEnd) return RET_ERR(-1, E_UNKNOWN);

	if (dialNumber != NULL && dialNumberSize >= 1)
	{
		if (!SmAddressFieldToString(tpOaSize, dialNumber, dialNumberSize)) return RET_ERR(-1, E_UNKNOWN);
	}

	if (messageSize < *tpUdl + 1) return RET_ERR(-1, E_UNKNOWN);
	for (int i = 0; i < *tpUdl; i++) {
		int offset = i - i / 8;
		int shift = i % 8;
		if (shift == 0) {
			message[i] = tpUd[offset] & 0x7f;
		}
		else {
			message[i] = tpUd[offset] * 256 + tpUd[offset - 1] << shift >> 8 & 0x7f;
		}
	}
	message[*tpUdl] = '\0';

	if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);

	return RET_OK(*tpUdl);
}

bool WioLTE::DeleteReceivedSMS()
{
	int messageIndex = GetFirstIndexOfReceivedSMS();
	if (messageIndex == -2) return RET_ERR(false, E_UNKNOWN);
	if (messageIndex < 0) return RET_ERR(false, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+CMGD=%d", messageIndex)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

bool WioLTE::Activate(const char* accessPointName, const char* userName, const char* password)
{
	std::string response;
	ArgumentParser parser;

	Stopwatch sw;
	sw.Restart();
	while (true) {
		//int resultCode;
		int status;

		_AtSerial.WriteCommand("AT+CGREG?");
		if (!_AtSerial.ReadResponse("^\\+CGREG: (.*)$", 500, &response)) return RET_ERR(false, E_UNKNOWN);
		parser.Parse(response.c_str());
		if (parser.Size() < 2) return RET_ERR(false, E_UNKNOWN);
		//resultCode = atoi(parser[0]);
		status = atoi(parser[1]);
		if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
		if (status == 0) return RET_ERR(false, E_UNKNOWN);
		if (status == 1 || status == 5) break;

		_AtSerial.WriteCommand("AT+CEREG?");
		if (!_AtSerial.ReadResponse("^\\+CEREG: (.*)$", 500, &response)) return RET_ERR(false, E_UNKNOWN);
		parser.Parse(response.c_str());
		if (parser.Size() < 2) return RET_ERR(false, E_UNKNOWN);
		//resultCode = atoi(parser[0]);
		status = atoi(parser[1]);
		if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
		if (status == 0) return RET_ERR(false, E_UNKNOWN);
		if (status == 1 || status == 5) break;

		if (sw.ElapsedMilliseconds() >= 120000) return RET_ERR(false, E_UNKNOWN);
	}

	// for debug.
#ifdef WIOLTE_DEBUG
	_AtSerial.WriteCommandAndReadResponse("AT+CREG?", "^OK$", 500, NULL);
	_AtSerial.WriteCommandAndReadResponse("AT+CGREG?", "^OK$", 500, NULL);
	_AtSerial.WriteCommandAndReadResponse("AT+CEREG?", "^OK$", 500, NULL);
#endif // WIOLTE_DEBUG

	StringBuilder str;
	if (!str.WriteFormat("AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", accessPointName, userName, password)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	sw.Restart();
	while (true) {
		_AtSerial.WriteCommand("AT+QIACT=1");
		if (!_AtSerial.ReadResponse("^(OK|ERROR)$", 150000, &response)) return RET_ERR(false, E_UNKNOWN);
		if (response == "OK") break;
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QIGETERROR", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
		if (sw.ElapsedMilliseconds() >= 150000) return RET_ERR(false, E_UNKNOWN);
		delay(POLLING_INTERVAL);
	}

	// for debug.
#ifdef WIOLTE_DEBUG
	if (!_AtSerial.WriteCommandAndReadResponse("AT+QIACT?", "^OK$", 150000, NULL)) return RET_ERR(false, E_UNKNOWN);
#endif // WIOLTE_DEBUG

	return RET_OK(true);
}

bool WioLTE::Deactivate()
{
	if (!_AtSerial.WriteCommandAndReadResponse("AT+QIDEACT=1", "^OK$", 40000, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

bool WioLTE::SyncTime(const char* host)
{
	StringBuilder str;
	if (!str.WriteFormat("AT+QNTP=1,\"%s\"", host)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.ReadResponse("^\\+QNTP: (.*)$", 125000, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

bool WioLTE::GetLocation(double* longitude, double* latitude)
{
	std::string response;
	ArgumentParser parser;

	if (!_AtSerial.WriteCommandAndReadResponse("AT+QLOCCFG=\"contextid\",1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	_AtSerial.WriteCommand("AT+QCELLLOC");
	if (!_AtSerial.ReadResponse("^\\+QCELLLOC: (.*)$", 60000, &response)) return RET_ERR(false, E_UNKNOWN);
	parser.Parse(response.c_str());
	if (parser.Size() != 2) return RET_ERR(false, E_UNKNOWN);
	*longitude = atof(parser[0]);
	*latitude = atof(parser[1]);
	if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

int WioLTE::SocketOpen(const char* host, int port, SocketType type)
{
	std::string response;
	ArgumentParser parser;

	if (host == NULL || host[0] == '\0') return RET_ERR(-1, E_UNKNOWN);
	if (port < 0 || 65535 < port) return RET_ERR(-1, E_UNKNOWN);

	const char* typeStr;
	switch (type) {
	case SOCKET_TCP:
		typeStr = "TCP";
		break;
	case SOCKET_UDP:
		typeStr = "UDP";
		break;
	default:
		return RET_ERR(-1, E_UNKNOWN);
	}

	bool connectIdUsed[CONNECT_ID_NUM];
	for (int i = 0; i < CONNECT_ID_NUM; i++) connectIdUsed[i] = false;

	_AtSerial.WriteCommand("AT+QISTATE?");
	do {
		if (!_AtSerial.ReadResponse("^(OK|\\+QISTATE: .*)$", 10000, &response)) return RET_ERR(-1, E_UNKNOWN);
		if (strncmp(response.c_str(), "+QISTATE: ", 10) == 0) {
			parser.Parse(&response.c_str()[10]);
			if (parser.Size() >= 1) {
				int connectId = atoi(parser[0]);
				if (connectId < 0 || CONNECT_ID_NUM <= connectId) return RET_ERR(-1, E_UNKNOWN);
				connectIdUsed[connectId] = true;
			}
		}
	} while (response != "OK");

	int connectId;
	for (connectId = 0; connectId < CONNECT_ID_NUM; connectId++) {
		if (!connectIdUsed[connectId]) break;
	}
	if (connectId >= CONNECT_ID_NUM) return RET_ERR(-1, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+QIOPEN=1,%d,\"%s\",\"%s\",%d", connectId, typeStr, host, port)) return RET_ERR(-1, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", 150000, NULL)) return RET_ERR(-1, E_UNKNOWN);
	str.Clear();
	if (!str.WriteFormat("^\\+QIOPEN: %d,0$", connectId)) return RET_ERR(-1, E_UNKNOWN);
	if (!_AtSerial.ReadResponse(str.GetString(), 150000, NULL)) return RET_ERR(-1, E_UNKNOWN);

	return RET_OK(connectId);
}

bool WioLTE::SocketSend(int connectId, const byte* data, int dataSize)
{
	if (connectId >= CONNECT_ID_NUM) return RET_ERR(false, E_UNKNOWN);
	if (dataSize > 1460) return RET_ERR(false, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+QISEND=%d,%d", connectId, dataSize)) return RET_ERR(false, E_UNKNOWN);
	_AtSerial.WriteCommand(str.GetString());
	if (!_AtSerial.ReadResponse("^>", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	_AtSerial.WriteBinary(data, dataSize);
	if (!_AtSerial.ReadResponse("^SEND OK$", 5000, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

bool WioLTE::SocketSend(int connectId, const char* data)
{
	return SocketSend(connectId, (const byte*)data, strlen(data));
}

int WioLTE::SocketReceive(int connectId, byte* data, int dataSize)
{
	std::string response;

	if (connectId >= CONNECT_ID_NUM) return RET_ERR(-1, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+QIRD=%d", connectId)) return RET_ERR(-1, E_UNKNOWN);
	_AtSerial.WriteCommand(str.GetString());
	if (!_AtSerial.ReadResponse("^\\+QIRD: (.*)$", 500, &response)) return RET_ERR(-1, E_UNKNOWN);
	int dataLength = atoi(response.c_str());
	if (dataLength >= 1) {
		if (dataLength > dataSize) return RET_ERR(-1, E_UNKNOWN);
		if (!_AtSerial.ReadBinary(data, dataLength, 500)) return RET_ERR(-1, E_UNKNOWN);
	}
	if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);

	return RET_OK(dataLength);
}

int WioLTE::SocketReceive(int connectId, char* data, int dataSize)
{
	int dataLength = SocketReceive(connectId, (byte*)data, dataSize - 1);
	if (dataLength >= 0) data[dataLength] = '\0';

	return dataLength;
}

int WioLTE::SocketReceive(int connectId, byte* data, int dataSize, long timeout)
{
	Stopwatch sw;
	sw.Restart();
	int dataLength;
	while ((dataLength = SocketReceive(connectId, data, dataSize)) == 0) {
		if (sw.ElapsedMilliseconds() >= timeout) return 0;
		delay(POLLING_INTERVAL);
	}
	return dataLength;
}

int WioLTE::SocketReceive(int connectId, char* data, int dataSize, long timeout)
{
	Stopwatch sw;
	sw.Restart();
	int dataLength;
	while ((dataLength = SocketReceive(connectId, data, dataSize)) == 0) {
		if (sw.ElapsedMilliseconds() >= timeout) return 0;
		delay(POLLING_INTERVAL);
	}
	return dataLength;
}

bool WioLTE::SocketClose(int connectId)
{
	if (connectId >= CONNECT_ID_NUM) return RET_ERR(false, E_UNKNOWN);

	StringBuilder str;
	if (!str.WriteFormat("AT+QICLOSE=%d", connectId)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", 10000, NULL)) return RET_ERR(false, E_UNKNOWN);

	return RET_OK(true);
}

int WioLTE::HttpGet(const char* url, char* data, int dataSize)
{
	std::string response;
	ArgumentParser parser;

	if (strncmp(url, "https:", 6) == 0) {
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QHTTPCFG=\"sslctxid\",1"         , "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QSSLCFG=\"sslversion\",1,4"      , "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QSSLCFG=\"ciphersuite\",1,0XFFFF", "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QSSLCFG=\"seclevel\",1,0"        , "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);
	}

	if (!_AtSerial.WriteCommandAndReadResponse("AT+QHTTPCFG=\"requestheader\",0", "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);

	if (!HttpSetUrl(url)) return RET_ERR(-1, E_UNKNOWN);

	if (!_AtSerial.WriteCommandAndReadResponse("AT+QHTTPGET", "^OK$", 500, NULL)) return RET_ERR(-1, E_UNKNOWN);
	if (!_AtSerial.ReadResponse("^\\+QHTTPGET: (.*)$", 60000, &response)) return RET_ERR(-1, E_UNKNOWN);

	parser.Parse(response.c_str());
	if (parser.Size() < 1) return RET_ERR(-1, E_UNKNOWN);
	if (strcmp(parser[0], "0") != 0) return RET_ERR(-1, E_UNKNOWN);
	int contentLength = parser.Size() >= 3 ? atoi(parser[2]) : -1;

	_AtSerial.WriteCommand("AT+QHTTPREAD");
	if (!_AtSerial.ReadResponse("^CONNECT$", 1000, NULL)) return RET_ERR(-1, E_UNKNOWN);
	if (contentLength >= 0) {
		if (contentLength + 1 > dataSize) return RET_ERR(-1, E_UNKNOWN);
		if (!_AtSerial.ReadBinary((byte*)data, contentLength, 60000)) return RET_ERR(-1, E_UNKNOWN);
		data[contentLength] = '\0';

		if (!_AtSerial.ReadResponse("^OK$", 1000, NULL)) return RET_ERR(-1, E_UNKNOWN);
	}
	else {
		if (!_AtSerial.ReadResponseQHTTPREAD(data, dataSize, 60000)) return RET_ERR(-1, E_UNKNOWN);
		contentLength = strlen(data);
	}
	if (!_AtSerial.ReadResponse("^\\+QHTTPREAD: 0$", 1000, NULL)) return RET_ERR(-1, E_UNKNOWN);

	return RET_OK(contentLength);
}

bool WioLTE::HttpPost(const char* url, const char* data, int* responseCode)
{
	std::string response;
	ArgumentParser parser;

	if (strncmp(url, "https:", 6) == 0) {
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QHTTPCFG=\"sslctxid\",1"         , "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QSSLCFG=\"sslversion\",1,4"      , "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QSSLCFG=\"ciphersuite\",1,0XFFFF", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
		if (!_AtSerial.WriteCommandAndReadResponse("AT+QSSLCFG=\"seclevel\",1,0"        , "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
	}

	if (!_AtSerial.WriteCommandAndReadResponse("AT+QHTTPCFG=\"requestheader\",1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);

	if (!HttpSetUrl(url)) return RET_ERR(false, E_UNKNOWN);

	const char* host;
	int hostLength;
	const char* uri;
	int uriLength;
	if (!SplitUrl(url, &host, &hostLength, &uri, &uriLength)) return RET_ERR(false, E_UNKNOWN);


	StringBuilder header;
	header.Write("POST ");
	if (uriLength <= 0) {
		header.Write("/");
	}
	else {
		header.Write(uri, uriLength);
	}
	header.Write(" HTTP/1.1\r\n");
	header.Write("Host: ");
	header.Write(host, hostLength);
	header.Write("\r\n");
	header.Write("Accept: */*\r\n");
	header.Write("User-Agent: "HTTP_POST_USER_AGENT"\r\n");
	header.Write("Connection: Keep-Alive\r\n");
	header.Write("Content-Type: "HTTP_POST_CONTENT_TYPE"\r\n");
	if (!header.WriteFormat("Content-Length: %d\r\n", strlen(data))) return RET_ERR(false, E_UNKNOWN);
	header.Write("\r\n");

	StringBuilder str;
	if (!str.WriteFormat("AT+QHTTPPOST=%d", header.Length() + strlen(data))) return RET_ERR(false, E_UNKNOWN);
	_AtSerial.WriteCommand(str.GetString());
	if (!_AtSerial.ReadResponse("^CONNECT$", 60000, NULL)) return RET_ERR(false, E_UNKNOWN);
	const char* headerStr = header.GetString();
	_AtSerial.WriteBinary((const byte*)headerStr, strlen(headerStr));
	_AtSerial.WriteBinary((const byte*)data, strlen(data));
	if (!_AtSerial.ReadResponse("^OK$", 1000, NULL)) return RET_ERR(false, E_UNKNOWN);
	if (!_AtSerial.ReadResponse("^\\+QHTTPPOST: (.*)$", 60000, &response)) return RET_ERR(false, E_UNKNOWN);
	parser.Parse(response.c_str());
	if (parser.Size() < 1) return RET_ERR(false, E_UNKNOWN);
	if (strcmp(parser[0], "0") != 0) return RET_ERR(false, E_UNKNOWN);
	if (parser.Size() < 2) {
		*responseCode = -1;
	}
	else {
		*responseCode = atoi(parser[1]);
	}

	return RET_OK(true);
}

void WioLTE::SystemReset()
{
	NVIC_SystemReset();
}

////////////////////////////////////////////////////////////////////////////////////////
