#pragma once
#include <windows.h>

class IMac
{
public:
	IMac(){}
	~IMac(){}
	virtual long Hash(PUCHAR message, size_t size, PUCHAR hmacKey, size_t hmacKeyLen, PUCHAR hash) = 0;
	virtual long DeriveKey(PUCHAR password, int passLen, PUCHAR key, ULONG keyLen) = 0;
};

