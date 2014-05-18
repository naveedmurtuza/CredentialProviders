
#include "stdafx.h"
#include "HMacSha1.h"
#include "SimplePinExtractor.h"
#include "SystemClock.h"
#include "TotpGenerator.h"
#include <windows.h>
#include <WinCred.h>
#include <bcrypt.h>
int _tmain(int argc, _TCHAR* argv[])
{
	PWSTR pwzProtectedPassword = L"1";
	int size = wcslen(pwzProtectedPassword);
	IMac *mac = new HMacSha1();
	IClock *clock = new SystemClock();
	IExtractPin *extractor = new SimplePinExtractor();
	TotpGenerator *otpGen = new TotpGenerator(mac, clock, extractor);
	size_t numOfCharConverted;
	char dest[100];
	char otp[100];
	errno_t err = wcstombs_s(&numOfCharConverted, dest, (size_t)100, pwzProtectedPassword, (size_t)size);
	otpGen->GenerateTimeoutCode((PBYTE)dest, numOfCharConverted, otp, 100);
	
	//otpGen->VerifyTimeOutCode((PBYTE)dest, numOfCharConverted, dest);
	for (;;)
	{
		otpGen->GenerateTimeoutCode((PBYTE)dest, numOfCharConverted, otp, 100);
		printf("%s \n", otp);
	}
	return 0;

}

