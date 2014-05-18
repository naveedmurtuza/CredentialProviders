#include "stdafx.h"
#include "SystemClock.h"
#include <time.h>
#include <string>

SystemClock::SystemClock()
{
}


SystemClock::~SystemClock()
{
}

int SystemClock::GetTimeStep()
{
	return x;
}

void SystemClock::GetClock(char* clock, int maxLen)
{
	time_t t = time(NULL);
	_i64toa_s(t, clock, maxLen, 16);
}

void SystemClock::GetIntervalTime(char* clock, ULONG step)
{
	time_t tttt = time(NULL);
	ULONG t = (time(NULL) - step) / GetTimeStep();
	char steps[40]; 
	char target[8];
	_i64toa_s(t, steps, 17, 16);
	int len = strlen(steps);
	if (len < 16)
	{
		memset(steps, '0', 17);
		_i64toa_s(t, &steps[16 - len], 16, 16);
	}
	//steps[0] = '1';
	char b[8];
	for (size_t i = 0; i < 16; i += 2)
	{
		b[i / 2] = (char2int(steps[i]) << 4) + (char2int(steps[i + 1]));
	}
	memcpy_s(clock, 8, b, 8);
}
int  SystemClock::char2int(char input)
{
	if (input >= '0' && input <= '9')
		return input - '0';
	if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	return -1;
}