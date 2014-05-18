//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif
#include <unknwn.h>
#include "HMacSha1.h"
#include "SimplePinExtractor.h"
#include "SystemClock.h"
#include "CTotpCredential.h"
#include "CWrappedCredentialEvents.h"
#include "guid.h"

// CSampleCredential ////////////////////////////////////////////////////////
// NOTE: Please read the readme.txt file to understand when it's appropriate to
// wrap an another credential provider and when it's not.  If you have questions
// about whether your scenario is an appropriate use of wrapping another credprov,
// please contact credprov@microsoft.com
CTotpCredential::CTotpCredential() :
    _cRef(1)
{
    DllAddRef();

    ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
    ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
    ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));

    _pWrappedCredential = NULL;
    _pWrappedCredentialEvents = NULL;
    _pCredProvCredentialEvents = NULL;

    _dwWrappedDescriptorCount = 0;
}

CTotpCredential::~CTotpCredential()
{
    for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
    {
        CoTaskMemFree(_rgFieldStrings[i]);
        CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
    }

    _CleanupEvents();
    
    if (_pWrappedCredential)
    {
        _pWrappedCredential->Release();
    }

    DllRelease();
}

// Initializes one credential with the field information passed in. We also keep track
// of our wrapped credential and how many fields it has.
HRESULT CTotpCredential::Initialize(
    __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
    __in const FIELD_STATE_PAIR* rgfsp,
    __in ICredentialProviderCredential *pWrappedCredential,
    __in DWORD dwWrappedDescriptorCount
    )
{
    HRESULT hr = S_OK;
    // Grab the credential we're wrapping for future reference.
    if (_pWrappedCredential != NULL)
    {
        _pWrappedCredential->Release();
    }
    _pWrappedCredential = pWrappedCredential;
    _pWrappedCredential->AddRef();

    // We also need to remember how many fields the inner credential has.
    _dwWrappedDescriptorCount = dwWrappedDescriptorCount;

    // Copy the field descriptors for each field. This is useful if you want to vary the field
    // descriptors based on what Usage scenario the credential was created for.
    for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
    {
        _rgFieldStatePairs[i] = rgfsp[i];
        hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
    }

    
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"", &_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]);
    }

    return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of 
// anything. We'll also provide it to the wrapped credential.
HRESULT CTotpCredential::Advise(
    __in ICredentialProviderCredentialEvents* pcpce
    )
{
    HRESULT hr = S_OK;

    _CleanupEvents();

    // We keep a strong reference on the real ICredentialProviderCredentialEvents
    // to ensure that the weak reference held by the CWrappedCredentialEvents is valid.
    _pCredProvCredentialEvents = pcpce;
    _pCredProvCredentialEvents->AddRef();

    _pWrappedCredentialEvents = new CWrappedCredentialEvents();
    
    if (_pWrappedCredentialEvents != NULL)
    {
        _pWrappedCredentialEvents->Initialize(this, pcpce);
    
        if (_pWrappedCredential != NULL)
        {
            hr = _pWrappedCredential->Advise(_pWrappedCredentialEvents);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

// LogonUI calls this to tell us to release the callback. 
// We'll also provide it to the wrapped credential.
HRESULT CTotpCredential::UnAdvise()
{
    HRESULT hr = S_OK;
    
    if (_pWrappedCredential != NULL)
    {
        _pWrappedCredential->UnAdvise();
    }

    _CleanupEvents();

    return hr;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions. In fact, we're just going to hand it off to the
// wrapped credential in case it wants to do something.
HRESULT CTotpCredential::SetSelected(__out BOOL* pbAutoLogon)  
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedCredential != NULL)
    {
        hr = _pWrappedCredential->SetSelected(pbAutoLogon);
    }

    return hr;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. We'll let the wrapped credential do anything it needs.
HRESULT CTotpCredential::SetDeselected()
{

	HRESULT hr = S_OK;
	_ClearFields();
	if (_pWrappedCredential != NULL)
	{
		hr = _pWrappedCredential->SetDeselected();
	}

    return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile. We'll check to see if it's for us or the wrapped credential, and then
// handle or route it as appropriate.
HRESULT CTotpCredential::GetFieldState(
    __in DWORD dwFieldID,
    __out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
    __out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
    )
{
    HRESULT hr = E_UNEXPECTED;

    // Make sure we have a wrapped credential.
    if (_pWrappedCredential != NULL)
    {
        // Validate parameters.
        if ((pcpfs != NULL) && (pcpfis != NULL))
        {
            // If the field is in the wrapped credential, hand it off.
            if (_IsFieldInWrappedCredential(dwFieldID))
            {
                hr = _pWrappedCredential->GetFieldState(dwFieldID, pcpfs, pcpfis);
            }
            // Otherwise, we need to see if it's one of ours.
            else
            {
                FIELD_STATE_PAIR *pfsp = _LookupLocalFieldStatePair(dwFieldID);
                // If the field ID is valid, give it info it needs.
                if (pfsp != NULL)
                {
                    *pcpfs = pfsp->cpfs;
                    *pcpfis = pfsp->cpfis;

                    hr = S_OK;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID. We'll check to see if 
// it's for us or the wrapped credential, and then handle or route it as appropriate.
HRESULT CTotpCredential::GetStringValue(
    __in DWORD dwFieldID, 
    __deref_out PWSTR* ppwsz
    )
{
	HRESULT hr = E_UNEXPECTED;



    // Make sure we have a wrapped credential.
    if (_pWrappedCredential != NULL)
    {
        // If this field belongs to the wrapped credential, hand it off.
        if (_IsFieldInWrappedCredential(dwFieldID))
        {
            hr = _pWrappedCredential->GetStringValue(dwFieldID, ppwsz);
        }
        // Otherwise determine if we need to handle it.
        else
        {
			FIELD_STATE_PAIR *pfsp = _LookupLocalFieldStatePair(dwFieldID);
            if (pfsp != NULL)
            {
				hr = SHStrDupW(_rgFieldStrings[SFI_OTP_CODE_EDITTEXT], ppwsz);
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
    }
    return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the 
// currently selected item (pdwSelectedItem). We'll check to see if it's for us or the 
// wrapped credential, and then handle or route it as appropriate.
HRESULT CTotpCredential::GetComboBoxValueCount(
    __in DWORD dwFieldID, 
    __out DWORD* pcItems, 
    __out_range(<,*pcItems) DWORD* pdwSelectedItem
    )
{
    HRESULT hr = E_UNEXPECTED;

    // Make sure we have a wrapped credential.
    if (_pWrappedCredential != NULL)
    {
        // If this field belongs to the wrapped credential, hand it off.
        if (_IsFieldInWrappedCredential(dwFieldID))
        {
            hr = _pWrappedCredential->GetComboBoxValueCount(dwFieldID, pcItems, pdwSelectedItem);
        }
        // Otherwise determine if we need to handle it.
        else
        {
            /*FIELD_STATE_PAIR *pfsp = _LookupLocalFieldStatePair(dwFieldID);
            if (pfsp != NULL)
            {
                *pcItems = ARRAYSIZE(s_rgDatabases);
                *pdwSelectedItem = _dwDatabaseIndex;
                hr = S_OK;
            }
            else
            {
                hr = E_INVALIDARG;
            }*/
        }
    }

    return hr;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
// We'll check to see if it's for us or the wrapped credential, and then handle or route 
// it as appropriate.
HRESULT CTotpCredential::GetComboBoxValueAt(
    __in DWORD dwFieldID, 
    __in DWORD dwItem,
    __deref_out PWSTR* ppwszItem
    )
{
    HRESULT hr = E_UNEXPECTED;

    // Make sure we have a wrapped credential.
    if (_pWrappedCredential != NULL)
    {
        // If this field belongs to the wrapped credential, hand it off.
        if (_IsFieldInWrappedCredential(dwFieldID))
        {
            hr = _pWrappedCredential->GetComboBoxValueAt(dwFieldID, dwItem, ppwszItem);
        }
        // Otherwise determine if we need to handle it.
        else
        {
            /*FIELD_STATE_PAIR *pfsp = _LookupLocalFieldStatePair(dwFieldID);
            if ((pfsp != NULL) && (dwItem < ARRAYSIZE(s_rgDatabases)))
            {
                hr = SHStrDupW(s_rgDatabases[dwItem], ppwszItem);
            }
            else
            {
                hr = E_INVALIDARG;
            }*/

        }
    }

    return hr;
}

// Called when the user changes the selected item in the combobox. We'll check to see if 
// it's for us or the wrapped credential, and then handle or route it as appropriate.
HRESULT CTotpCredential::SetComboBoxSelectedValue(
    __in DWORD dwFieldID,
    __in DWORD dwSelectedItem
    )
{
    HRESULT hr = E_UNEXPECTED;

    // Make sure we have a wrapped credential.
    if (_pWrappedCredential != NULL)
    {
        // If this field belongs to the wrapped credential, hand it off.
        if (_IsFieldInWrappedCredential(dwFieldID))
        {
            hr = _pWrappedCredential->SetComboBoxSelectedValue(dwFieldID, dwSelectedItem);
        }
        // Otherwise determine if we need to handle it.
        else
        {
            /*FIELD_STATE_PAIR *pfsp = _LookupLocalFieldStatePair(dwFieldID);
            if ((pfsp != NULL) && (dwSelectedItem < ARRAYSIZE(s_rgDatabases)))
            {
                _dwDatabaseIndex = dwSelectedItem;
                hr = S_OK;
            }
            else
            {
                hr = E_INVALIDARG;
            }*/
        }
    }

    return hr;
}

//------------- 
// The following methods are for logonUI to get the values of various UI elements and 
// then communicate to the credential about what the user did in that field. Even though
// we don't offer these field types ourselves, we need to pass along the request to the
// wrapped credential.

HRESULT CTotpCredential::GetBitmapValue(
    __in DWORD dwFieldID, 
    __out HBITMAP* phbmp
    )
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedCredential != NULL)
    {
        hr = _pWrappedCredential->GetBitmapValue(dwFieldID, phbmp);
    }

    return hr;
}

HRESULT CTotpCredential::GetSubmitButtonValue(
    __in DWORD dwFieldID,
    __out DWORD* pdwAdjacentTo
    )
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedCredential != NULL)
    {
        hr = _pWrappedCredential->GetSubmitButtonValue(dwFieldID, pdwAdjacentTo);
    }
	*pdwAdjacentTo = _dwWrappedDescriptorCount + SFI_OTP_CODE_EDITTEXT;
    return hr;
}

HRESULT CTotpCredential::SetStringValue(
	__in DWORD dwFieldID,
	__in PCWSTR pwz
	)
{
	HRESULT hr = E_UNEXPECTED;
	// Make sure we have a wrapped credential.
	if (_pWrappedCredential != NULL)
	{
		// If this field belongs to the wrapped credential, hand it off.
		if (_IsFieldInWrappedCredential(dwFieldID))
		{
			hr = _pWrappedCredential->SetStringValue(dwFieldID, pwz);
		}
		// Otherwise determine if we need to handle it.
		else
		{
			DWORD dwOffsetId = _dwWrappedDescriptorCount - dwFieldID;
			if (dwOffsetId < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
				(CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwOffsetId].cpft ||
				CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwOffsetId].cpft))
			{
				PWSTR* ppwszStored = &_rgFieldStrings[dwOffsetId];
				CoTaskMemFree(*ppwszStored);
				hr = SHStrDupW(pwz, ppwszStored);	
			}
			else
			{
				hr = E_INVALIDARG;
			}
		}		
	}
	return hr;
}

HRESULT CTotpCredential::GetCheckboxValue(
    __in DWORD dwFieldID, 
    __out BOOL* pbChecked,
    __deref_out PWSTR* ppwszLabel
    )
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedCredential != NULL)
    {
        if (_IsFieldInWrappedCredential(dwFieldID))
        {
            hr = _pWrappedCredential->GetCheckboxValue(dwFieldID, pbChecked, ppwszLabel);
        }
    }

    return hr;
}

HRESULT CTotpCredential::SetCheckboxValue(
    __in DWORD dwFieldID, 
    __in BOOL bChecked
    )
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedCredential != NULL)
    {
        hr = _pWrappedCredential->SetCheckboxValue(dwFieldID, bChecked);
    }

    return hr;
}

HRESULT CTotpCredential::CommandLinkClicked(__in DWORD dwFieldID)
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedCredential != NULL)
    {
        hr = _pWrappedCredential->CommandLinkClicked(dwFieldID);
    }

    return hr;
}
//------ end of methods for controls we don't have ourselves ----//


//
// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
//
HRESULT CTotpCredential::GetSerialization(
    __out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    __out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
    __deref_out_opt PWSTR* ppwszOptionalStatusText, 
    __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
    )
{
    HRESULT hr = E_UNEXPECTED;
	
    if (_pWrappedCredential != NULL)
    {
		if (_rgFieldStrings[SFI_OTP_CODE_EDITTEXT] && wcslen(_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]) > 0)
		{
			//::MessageBox(NULL, _rgFieldStrings[SFI_OTP_CODE_EDITTEXT], L"CAPTION", MB_OK);
			PWSTR			username,
				password;
			char			pwd[100];
			char			otp[100];
			IMac *mac = new HMacSha1();
			IClock *clock = new SystemClock();
			IExtractPin *extractor = new SimplePinExtractor();
			TotpGenerator *otpGen = new TotpGenerator(mac, clock, extractor);
			size_t			numOfCharConverted;
			_pWrappedCredential->GetStringValue(0, &username);
			_pWrappedCredential->GetStringValue(3, &password);
			size_t			pwdSize = 0;
			//::MessageBox(NULL, _otpValue, password, MB_OK);
			wcstombs_s(&pwdSize, pwd, (size_t)100, password, wcslen(password));
			wcstombs_s(&numOfCharConverted, otp, (size_t)100, _rgFieldStrings[SFI_OTP_CODE_EDITTEXT], wcslen(_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]));
			char tmp[100];
			sprintf_s(tmp, 100, "%d", numOfCharConverted);
			BOOL otpValid = FALSE;
			NTSTATUS status = otpGen->VerifyTimeOutCode((PBYTE)pwd, (int)pwdSize, otp,&otpValid);
			if (NT_SUCCESS(status))
			{
				if (otpValid == TRUE)
				{
					hr = _pWrappedCredential->GetSerialization(pcpgsr, pcpcs, ppwszOptionalStatusText, pcpsiOptionalStatusIcon);
				}
				else
				{
					SHStrDupW(L"Totp entered is invalid", ppwszOptionalStatusText);
					*pcpsiOptionalStatusIcon = CPSI_ERROR;
					hr = S_FALSE;
				}
			}
			else
			{
				SHStrDupW(L"Internal Error", ppwszOptionalStatusText);
				*pcpsiOptionalStatusIcon = CPSI_ERROR;
				hr = S_FALSE;
			}
			
			SecureZeroMemory(pwd, 100);
			SecureZeroMemory(otp, 100);
			SecureZeroMemory(username, wcslen(username));
			SecureZeroMemory(password, pwdSize);
			//clean up the mess we made
			delete mac;
			delete clock;
			delete extractor;
			delete otpGen;
		}
		else
		{
			SHStrDupW(L"enter Totp value", ppwszOptionalStatusText);
			SHStrDupW(L"", &_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]);
			hr = S_FALSE;
		}
    }
	_ClearFields();
    return hr;
}

// ReportResult is completely optional. However, we will hand it off to the wrapped
// credential in case they want to handle it.
HRESULT CTotpCredential::ReportResult(
    __in NTSTATUS ntsStatus, 
    __in NTSTATUS ntsSubstatus,
    __deref_out_opt PWSTR* ppwszOptionalStatusText, 
    __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
    )
{
    HRESULT hr = E_UNEXPECTED;
    if (_pWrappedCredential != NULL)
    {
        hr = _pWrappedCredential->ReportResult(ntsStatus, ntsSubstatus, ppwszOptionalStatusText, pcpsiOptionalStatusIcon);
    }

    return hr;
}

void CTotpCredential::_ClearFields()
{
	HRESULT hr = E_NOTIMPL;
	if (_rgFieldStrings[SFI_OTP_CODE_EDITTEXT])
	{

		size_t lenPassword = lstrlen(_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]);
		SecureZeroMemory(_rgFieldStrings[SFI_OTP_CODE_EDITTEXT], lenPassword * sizeof(*_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]));

		CoTaskMemFree(_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]);
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_OTP_CODE_EDITTEXT]);
		if (SUCCEEDED(hr) && _pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, _dwWrappedDescriptorCount + SFI_OTP_CODE_EDITTEXT, _rgFieldStrings[SFI_OTP_CODE_EDITTEXT]);
			_pCredProvCredentialEvents->SetFieldString(this, 3, L"");
		}
	}
}

BOOL CTotpCredential::_IsFieldInWrappedCredential(
    __in DWORD dwFieldID
    )
{
    return (dwFieldID < _dwWrappedDescriptorCount);
}

FIELD_STATE_PAIR *CTotpCredential::_LookupLocalFieldStatePair(
    __in DWORD dwFieldID
    )
{
    // Offset into the ID to account for the wrapped fields.
    dwFieldID -= _dwWrappedDescriptorCount;

    // If the index if valid, give it the info it wants.
    if (dwFieldID < SFI_NUM_FIELDS)
    {
        return &(_rgFieldStatePairs[dwFieldID]);
    }
    
    return NULL;
}

void CTotpCredential::_CleanupEvents()
{
    // Call Uninitialize before releasing our reference on the real 
    // ICredentialProviderCredentialEvents to avoid having an
    // invalid reference.
    if (_pWrappedCredentialEvents != NULL)
    {
        _pWrappedCredentialEvents->Uninitialize();
        _pWrappedCredentialEvents->Release();
        _pWrappedCredentialEvents = NULL;
    }

    if (_pCredProvCredentialEvents != NULL)
    {
        _pCredProvCredentialEvents->Release();
        _pCredProvCredentialEvents = NULL;
    }
}
