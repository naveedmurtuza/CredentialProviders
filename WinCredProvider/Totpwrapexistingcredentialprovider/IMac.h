#pragma once
#include <windows.h>

class IMac
{
public:
	IMac(){}
	~IMac(){}
	virtual long Hash(PUCHAR message, ULONG size, PUCHAR hmacKey, ULONG hmacKeyLen, PUCHAR hash) = 0;
	virtual long DeriveKey(PUCHAR password, int passLen, PUCHAR key, ULONG keyLen) = 0;
};

