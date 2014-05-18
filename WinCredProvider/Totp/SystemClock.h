#pragma once
#include "IClock.h"
class SystemClock :
	public IClock
{
private:
	int x = 30;
	int char2int(char input);
public:
	SystemClock();
	~SystemClock();
	void GetClock(char* clock, int maxLen);
	int GetTimeStep();
	void GetIntervalTime(char* time, ULONG step);
};

