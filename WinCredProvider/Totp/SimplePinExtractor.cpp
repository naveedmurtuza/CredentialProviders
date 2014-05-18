#include "stdafx.h"
#include "SimplePinExtractor.h"
#include <string>

SimplePinExtractor::SimplePinExtractor()
{
}


SimplePinExtractor::~SimplePinExtractor()
{
}

void SimplePinExtractor::GetPin(PCHAR buffer, char* otpCode, size_t size)
{
	int offset = buffer[20 - 1] & 0xf;
	int binary
		= ((buffer[offset] & 0x7f) << 24)
		| ((buffer[offset + 1] & 0xff) << 16)
		| ((buffer[offset + 2] & 0xff) << 8)
		| (buffer[offset + 3] & 0xff);
	int otp = binary % PIN_MODULO;
	//_itoa_s(otp, otpCode, size, 10);
	sprintf_s(otpCode, size, "%06d", otp);
}

int SimplePinExtractor::GetPinLength()
{
	return PIN_LENGTH;
}