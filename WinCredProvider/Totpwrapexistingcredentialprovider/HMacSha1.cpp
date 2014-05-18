
#include "HMacSha1.h"
#include <string>
#include <bcrypt.h>

HMacSha1::HMacSha1()
{
}


HMacSha1::~HMacSha1()
{
}
void HMacSha1::Cleanup()
{
	NTSTATUS            Status;
	if (NULL != secretKeyHandle)
	{
		Status = BCryptDestroyKey(secretKeyHandle);
		if (!NT_SUCCESS(Status))
		{
			//
		}
		secretKeyHandle = NULL;
	}

	if (NULL != hAlg)
	{
		Status = BCryptCloseAlgorithmProvider(hAlg, 0);
		if (!NT_SUCCESS(Status))
		{
			//
		}
		hAlg = NULL;
	}
	if (pbHashObject)
	{
		HeapFree(GetProcessHeap(), 0, pbHashObject);
		pbHashObject = NULL;
	}
	if (hFile)
	{
		CloseHandle(hFile);
	}
}


NTSTATUS HMacSha1::DeriveKey(PUCHAR password, int passLen, PUCHAR key, ULONG keyLen)
{

	NTSTATUS            Status;
	UCHAR salt[8] =
	{
		0x78, 0x60, 0x22, 0x07, 0x19, 0x83, 0xde, 0xad,
	};

	// open an algorithm handle
	if (!NT_SUCCESS(Status = BCryptOpenAlgorithmProvider(
		&hAlg,
		BCRYPT_SHA1_ALGORITHM,
		NULL,
		BCRYPT_ALG_HANDLE_HMAC_FLAG)))
	{
		Cleanup();
		return Status;
	}


	if (!NT_SUCCESS(Status = BCryptDeriveKeyPBKDF2(
		hAlg, password, passLen, salt, 8, 12000, key, keyLen, 0)))

	{
		Cleanup();
		return Status;
	}

	return Status;

}

NTSTATUS HMacSha1::Hash(PUCHAR message, ULONG size, PUCHAR hmacKey, ULONG hmacKeyLen, PUCHAR hash)
{
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD                   cbData = 0, cbHashObject = 0;
	
	// open an algorithm handle
	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
		&hAlg,
		BCRYPT_SHA1_ALGORITHM,
		NULL,
		BCRYPT_ALG_HANDLE_HMAC_FLAG)))
	{
		
		Cleanup();
		return status;
	}

	//calculate the size of the buffer to hold the hash object
	if (!NT_SUCCESS(status = BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		(PBYTE)&cbHashObject,
		sizeof(DWORD),
		&cbData,
		0)))
	{
		Cleanup();
		return status;
	}

	//allocate the hash object on the heap
	pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
	if (NULL == pbHashObject)
	{
		Cleanup();
		return status;
	}
	//create a hash
	if (!NT_SUCCESS(status = BCryptCreateHash(
		hAlg,
		&hHash,
		pbHashObject,
		cbHashObject,
		hmacKey,
		hmacKeyLen,
		0)))
	{
		Cleanup();
		return status;
	}


	
	if (!NT_SUCCESS(status = BCryptHashData(
		hHash,
		message,
		size,
		0)))
	{
		Cleanup();
		return status;
	}

	//close the hash
	if (!NT_SUCCESS(status = BCryptFinishHash(
		hHash,
		hash,
		20,
		0)))
	{
		Cleanup();
		return status;
	}

	Cleanup();
	return status;
}