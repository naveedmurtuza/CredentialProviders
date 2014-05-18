#pragma once
#include "IClock.h"
#include "IExtractPin.h"
#include"IMac.h"
class TotpGenerator
{
public:
	TotpGenerator(IMac *mac,IClock *clock,IExtractPin *extractor);
	~TotpGenerator();
	void GenerateTimeoutCode(PBYTE password, size_t pwdLen, PCHAR otpCode, size_t otpCodeLen);
	void GenerateTimeoutCode(PBYTE password, size_t pwdLen, PCHAR otpCode, size_t otpCodeLen, int interval);
	BOOL VerifyTimeOutCode(PBYTE password, size_t pwdLen, PCHAR otpCode);
private:
	IMac		*_mac;
	IClock		*_clock;
	IExtractPin *_extractor;
};

