#pragma once
#include <Windows.h>
#include <bcrypt.h>
#include "IMac.h"

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
class HMacSha1 :
	public IMac
{
private:
	const int				HASH_LEN = 32;
	const int				DERIVED_KEY_LEN = 60;
	const int				ITERATION_COUNT = 12000;

	BCRYPT_ALG_HANDLE		hAlg;
	BCRYPT_KEY_HANDLE		secretKeyHandle;
	BCRYPT_HASH_HANDLE		hHash;
	PBYTE                   pbHashObject;
	void Cleanup();
public:
	HMacSha1();
	~HMacSha1();

	NTSTATUS Hash(PUCHAR message, size_t size, PUCHAR hmacKey, size_t hmacKeyLen, PUCHAR hash);
	NTSTATUS DeriveKey(PUCHAR password, int passLen, PUCHAR key, ULONG keyLen);
};

