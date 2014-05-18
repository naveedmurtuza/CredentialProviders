#pragma once
#include "IExtractPin.h"
class SimplePinExtractor :
	public IExtractPin
{
private:
	const int PIN_LENGTH = 6;
	const int PIN_MODULO = 1000000;
public:
	SimplePinExtractor();
	~SimplePinExtractor();
	void GetPin(PCHAR buffer, char* otpCode, size_t size);
	int GetPinLength();
};

