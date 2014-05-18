#pragma once
#include <windows.h>

class IClock
{
public:
	IClock(){}
	virtual ~IClock(){}
	virtual int GetTimeStep() = 0;
	virtual void GetIntervalTime(char* time, ULONG step) = 0;
};

