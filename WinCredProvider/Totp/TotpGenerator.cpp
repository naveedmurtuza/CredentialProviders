#include "stdafx.h"
#include "TotpGenerator.h"


TotpGenerator::TotpGenerator(IMac *mac, IClock *clock, IExtractPin *extractor)
{
	_mac = mac;
	_clock = clock;
	_extractor = extractor;
}


TotpGenerator::~TotpGenerator()
{
	
}
void TotpGenerator::GenerateTimeoutCode(PBYTE password, size_t pwdLen, PCHAR otpCode, size_t otpCodeLen)
{
	GenerateTimeoutCode(password, pwdLen, otpCode, otpCodeLen, 0);
}

void TotpGenerator::GenerateTimeoutCode(PBYTE password, size_t pwdLen, PCHAR otpCode, size_t otpCodeLen, int interval)
{
	char key[256];
	char intervalTime[8];
	char hash[256];
	_mac->DeriveKey(password, pwdLen, (PUCHAR)key, 256);
	_clock->GetIntervalTime(intervalTime, interval);
	_mac->Hash((PBYTE)intervalTime, 8, (PUCHAR)key, 32, (PUCHAR)hash);
	_extractor->GetPin(hash, otpCode, 10);
}

BOOL TotpGenerator::VerifyTimeOutCode(PBYTE password, size_t pwdLen, PCHAR otpCode)
{
	char* otp = new char[_extractor->GetPinLength()];
	for (int interval = 0; interval > 3; interval++)
	{
		this->GenerateTimeoutCode(password, pwdLen, otp, _extractor->GetPinLength());
		if (strcmp(otpCode, otp) == 0)
		{
			return true;
		}
	}
	return false;
}