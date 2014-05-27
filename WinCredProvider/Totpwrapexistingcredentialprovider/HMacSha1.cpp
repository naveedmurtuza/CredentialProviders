
#include "HMacSha1.h"
#include <string>
#include <bcrypt.h>
#include "log.h"

using namespace Logging;

extern Logger *l;
HMacSha1::HMacSha1()
{
}


HMacSha1::~HMacSha1()
{
}
void HMacSha1::Cleanup()
{
	NTSTATUS            Status;
	l->LogS(INFO, "BCryptCloseAlgorithmProvider \n");
	if (NULL != hAlg)
	{
		Status = BCryptCloseAlgorithmProvider(hAlg, 0);
		if (!NT_SUCCESS(Status))
		{
			//
		}
		hAlg = NULL;
	}
	l->LogS(INFO, "BCryptCloseAlgorithmProvider Done \n");
	l->LogS(INFO, "pbHashObject \n");
	//todo: According to the sample at http://msdn.microsoft.com/en-us/library/windows/desktop/aa376217%28v=vs.85%29.aspx
	//we shud call BCryptDestroyHash on 'hHash' 
	//but in this case, it crashes the cred provider ??
	//need to look into
	if (hHash)
	{
		BCryptDestroyHash(hHash);
		hHash = NULL;
	}
	if (pbHashObject)
	{
		HeapFree(GetProcessHeap(), 0, pbHashObject);
		pbHashObject = NULL;
	}
	
	l->LogS(INFO, "pbHashObject Done \n");
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
		l->LogS(INFO, "DeriveKey");
		Cleanup();
		return Status;
	}


	if (!NT_SUCCESS(Status = BCryptDeriveKeyPBKDF2(
		hAlg, password, passLen, salt, 8, 12000, key, keyLen, 0)))

	{
		Cleanup();
		return Status;
	}
	//if we are here then the key derivation was successfull
	//say so in logs
	l->LogS(INFO, "Key Derivation successful \n");
	Cleanup();
	l->LogS(INFO, "Clean up \n");
	return Status;

}

NTSTATUS HMacSha1::Hash(PUCHAR message, ULONG size, PUCHAR hmacKey, ULONG hmacKeyLen, PUCHAR hash)
{
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD                   cbData = 0, cbHashObject = 0;
	l->LogS(INFO, "Opening ALgo handle \n");
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
	l->LogS(INFO, "Opening ALgo handle ... DOne \n");
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
	l->LogS(INFO, "HeapAlloc \n");
	//allocate the hash object on the heap
	pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
	if (NULL == pbHashObject)
	{
		Cleanup();
		return status;
	}
	l->LogS(INFO, "HeapAlloc .. DOne \n");
	l->LogS(INFO, "create a hash \n");
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
	l->LogS(INFO, "create a hash ... Done \n");


	l->LogS(INFO, "BCryptHashData \n");
	if (!NT_SUCCESS(status = BCryptHashData(
		hHash,
		message,
		size,
		0)))
	{
		Cleanup();
		return status;
	}
	l->LogS(INFO, "BCryptHashData ... Done \n");
	l->LogS(INFO, "BCryptFinishHash \n");
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
	l->LogS(INFO, "BCryptFinishHash .. Done\n");
	//if we are here then the hash was successfull
	//say so in logs
	
	l->LogS(INFO, "Hashing successful \n");
	Cleanup();
	return status;
}