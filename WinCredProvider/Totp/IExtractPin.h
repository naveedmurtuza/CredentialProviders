#pragma once
#include <windows.h>
class IExtractPin
{
public:
	IExtractPin(){}
	virtual ~IExtractPin(){}
	virtual void GetPin(PCHAR buffer, char* otpCode, size_t size) = 0;
	virtual int GetPinLength() = 0;
};