
#include "TotpGenerator.h"

#include "log.h"

using namespace Logging;

extern Logger *l;
TotpGenerator::TotpGenerator(IMac *mac, IClock *clock, IExtractPin *extractor)
{
	_mac = mac;
	_clock = clock;
	_extractor = extractor;
}


TotpGenerator::~TotpGenerator()
{
	
}
NTSTATUS TotpGenerator::GenerateTimeoutCode(PBYTE password, int pwdLen, PCHAR otpCode, int otpCodeLen)
{
	return GenerateTimeoutCode(password, pwdLen, otpCode, otpCodeLen, 0);
}

NTSTATUS TotpGenerator::GenerateTimeoutCode(PBYTE password, int pwdLen, PCHAR otpCode, int otpCodeLen, int interval)
{
	NTSTATUS status = 0;
	char key[256];
	char intervalTime[18];
	char hash[256];

	if (!NT_SUCCESS(status = _mac->DeriveKey(password, pwdLen, (PUCHAR)key, 32)))
	{
		return status;
	}

	_clock->GetIntervalTime(intervalTime, interval);

	if (!NT_SUCCESS(status = _mac->Hash((PBYTE)intervalTime, 8, (PUCHAR)key, 32, (PUCHAR)hash)))
	{
		l->LogS(INFO, "GenerateTimeoutCode -> Hash -> Error %d \n", status);
		return status;
	}
	;
	_extractor->GetPin(hash, otpCode, otpCodeLen);
	l->LogS(INFO, "All DONE.... %d \n",status);
	return status;
}

NTSTATUS TotpGenerator::VerifyTimeOutCode(PBYTE password, int pwdLen, PCHAR otpCode,BOOL *valid)
{
	PCHAR otp = new char[50];
	NTSTATUS status;
	for (int interval = 0; interval < 2; interval++)
	{
		status = this->GenerateTimeoutCode(password, pwdLen, otp, 50, interval);
		if (strcmp(otpCode, otp) == 0)
		{
			*valid = TRUE;
			delete otp;
			return status;
			
		}
	}
	
	delete otp;
	*valid = FALSE;
	return status;
}