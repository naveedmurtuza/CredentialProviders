#pragma once
#include <Windows.h>
#include "common_defs.h"

class TotpGenerator
{
public:
	TotpGenerator(IMac *mac,IClock *clock,IExtractPin *extractor);
	~TotpGenerator();
	NTSTATUS GenerateTimeoutCode(PBYTE password, int pwdLen, PCHAR otpCode, int otpCodeLen);
	NTSTATUS GenerateTimeoutCode(PBYTE password, int pwdLen, PCHAR otpCode, int otpCodeLen, int interval);
	NTSTATUS VerifyTimeOutCode(PBYTE password, int pwdLen, PCHAR otpCode,BOOL *valid);
private:
	IMac		*_mac;
	IClock		*_clock;
	IExtractPin *_extractor;
};

