/**
 * Copyright (C) 2010 cxxjoe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#include "stdafx.h"

// based on the Windows SDK samples "RecipePreviewHandler" and "RecipeThumbnailProvider"


#define DEFAULT_APP_PROG_ID L"iNFEKT.NFOFile.1"

/************************************************************************/
/* COM/SHELL signatures                                                 */
/************************************************************************/

// introducing: CNFOPreviewHandler shell/COM signature
extern HRESULT CNFOPreviewHandler_CreateInstance(REFIID riid, void **ppv);

#define SZ_CLSID_NFOPREVIEWHANDLER L"{B8D080EE-9541-460f-A1AE-7C43CDA96C0F}"
#define SZ_NFOPREVIEWHANDLER L"iNFekt NFO Shell Preview Handler"

static const CLSID CLSID_NFOPreviewHandler = { 0xb8d080ee, 0x9541, 0x460f, { 0xa1, 0xae, 0x7c, 0x43, 0xcd, 0xa9, 0x6c, 0xf } };

// introducing: CNFOThumbProvider shell/COM signature
extern HRESULT CNFOThumbProvider_CreateInstance(REFIID riid, void **ppv);

#define SZ_CLSID_NFOTHUMBHANDLER L"{B3F5EDE0-4267-49eb-A775-799895476453}"
#define SZ_NFOTHUMBHANDLER L"iNFekt NFO Shell Thumbnail Handler"

static const CLSID CLSID_NFOThumbHandler = { 0xb3f5ede0, 0x4267, 0x49eb, { 0xa7, 0x75, 0x79, 0x98, 0x95, 0x47, 0x64, 0x53 } };


/************************************************************************/
/* utility declarations                                                 */
/************************************************************************/

typedef HRESULT (*PFNCREATEINSTANCE)(REFIID riid, void **ppvObject);
struct CLASS_OBJECT_INIT
{
	const CLSID *pClsid;
	PFNCREATEINSTANCE pfnCreate;
};

// add classes supported by this module here
const CLASS_OBJECT_INIT c_rgClassObjectInit[] =
{
	{ &CLSID_NFOPreviewHandler, CNFOPreviewHandler_CreateInstance },
	{ &CLSID_NFOThumbHandler, CNFOThumbProvider_CreateInstance }
};


/************************************************************************/
/* static/global vars                                                   */
/************************************************************************/

long g_cRefModule = 0;
HINSTANCE g_hInst = NULL;


/************************************************************************/
/* Class Factory Class                                                  */
/************************************************************************/

static void DllAddRef()
{
	InterlockedIncrement(&g_cRefModule);
}

static void DllRelease()
{
	InterlockedDecrement(&g_cRefModule);
}


class CClassFactory : public IClassFactory
{
public:
	static HRESULT CreateInstance(REFCLSID clsid, const CLASS_OBJECT_INIT *pClassObjectInits, size_t cClassObjectInits, REFIID riid, void **ppv)
	{
		*ppv = NULL;
		HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
		for(size_t i = 0; i < cClassObjectInits; i++)
		{
			if(clsid == *pClassObjectInits[i].pClsid)
			{
				IClassFactory *pClassFactory = new (std::nothrow) CClassFactory(pClassObjectInits[i].pfnCreate);
				hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
				if(SUCCEEDED(hr))
				{
					hr = pClassFactory->QueryInterface(riid, ppv);
					pClassFactory->Release();
				}
				break; // match found
			}
		}
		return hr;
	}

	CClassFactory(PFNCREATEINSTANCE pfnCreate) : _cRef(1), _pfnCreate(pfnCreate)
	{
		DllAddRef();
	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(CClassFactory, IClassFactory),
			{ 0 }
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		long cRef = InterlockedDecrement(&_cRef);
		if(cRef == 0)
		{
			delete this;
		}
		return cRef;
	}

	// IClassFactory
	IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
	{
		return punkOuter ? CLASS_E_NOAGGREGATION : _pfnCreate(riid, ppv);
	}

	IFACEMETHODIMP LockServer(BOOL fLock)
	{
		if(fLock)
		{
			DllAddRef();
		}
		else
		{
			DllRelease();
		}
		return S_OK;
	}

private:
	~CClassFactory()
	{
		DllRelease();
	}

	long _cRef;
	PFNCREATEINSTANCE _pfnCreate;
};


/************************************************************************/
/* DLL COM interface routines                                           */
/************************************************************************/

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		g_hInst = hInstance;
		DisableThreadLibraryCalls(hInstance);
	}
	return TRUE;
}

STDAPI DllCanUnloadNow()
{
	// Only allow the DLL to be unloaded after all outstanding references have been released
	return (g_cRefModule == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
	return CClassFactory::CreateInstance(clsid, c_rgClassObjectInit, ARRAYSIZE(c_rgClassObjectInit), riid, ppv);
}


/************************************************************************/
/* Registry helper routines                                             */
/************************************************************************/

// A struct to hold the information required for a registry entry
struct REGISTRY_ENTRY
{
	HKEY   hkeyRoot;
	PCWSTR pszKeyName;
	PCWSTR pszValueName;
	PCWSTR pszData;
};

// Creates a registry key (if needed) and sets the default value of the key
HRESULT CreateRegKeyAndSetValue(const REGISTRY_ENTRY *pRegistryEntry)
{
	HKEY hKey;
	HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyExW(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL));

	if(SUCCEEDED(hr))
	{
		hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, pRegistryEntry->pszValueName, 0, REG_SZ,
			(LPBYTE)pRegistryEntry->pszData,
			((DWORD)wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR)));

		RegCloseKey(hKey);
	}

	return hr;
}


/************************************************************************/
/* DllRegisterServer/DllUnregisterServer                                */
/************************************************************************/

static HRESULT DeleteRegKeys()
{
	HRESULT hr = S_OK;

	const PCWSTR rgpszKeys[] =
	{
		L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOTHUMBHANDLER,
		L"Software\\Classes\\" DEFAULT_APP_PROG_ID L"\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}",
		L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOPREVIEWHANDLER,
		L"Software\\Classes\\" DEFAULT_APP_PROG_ID L"\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}"
	};

	for(int i = 0; i < ARRAYSIZE(rgpszKeys) && SUCCEEDED(hr); i++)
	{
		hr = HRESULT_FROM_WIN32(RegDeleteTreeW(HKEY_CURRENT_USER, rgpszKeys[i]));
		if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			// If the registry entry has already been deleted, say S_OK.
			hr = S_OK;
		}
	}

	HKEY hPrevHs;
	LSTATUS dwResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers", 0, KEY_ALL_ACCESS, &hPrevHs);
	if(dwResult == ERROR_SUCCESS)
	{
		// don't care about failure here:
		RegDeleteValueW(hPrevHs, SZ_CLSID_NFOPREVIEWHANDLER);
		RegCloseKey(hPrevHs);
	}
	else
		hr = HRESULT_FROM_WIN32(dwResult);

	return hr;
}


STDAPI DllRegisterServer()
{
	HRESULT hr;

	if(!CUtil::IsWin5x() && !CUtil::IsWin6x())
	{
		return S_FALSE;
	}

	WCHAR szModuleName[1000];

	if(!GetModuleFileNameW(g_hInst, szModuleName, ARRAYSIZE(szModuleName)))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		// List of registry entries we want to create
		const REGISTRY_ENTRY rgRegistryEntries[] =
		{
			// thumbnail handler:
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOTHUMBHANDLER,                             NULL,                       SZ_NFOTHUMBHANDLER},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOTHUMBHANDLER L"\\InProcServer32",         NULL,                       szModuleName},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOTHUMBHANDLER L"\\InProcServer32",         L"ThreadingModel",          L"Apartment"},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\" DEFAULT_APP_PROG_ID L"\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}", NULL,          SZ_CLSID_NFOTHUMBHANDLER},

			// preview handler:
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOPREVIEWHANDLER,                           NULL,                       SZ_NFOPREVIEWHANDLER},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOPREVIEWHANDLER L"\\InProcServer32",       NULL,                       szModuleName},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOPREVIEWHANDLER L"\\InProcServer32",       L"ThreadingModel",          L"Apartment"},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_NFOPREVIEWHANDLER,                           L"AppID",                   L"{6d2b5079-2f0b-48dd-ab7f-97cec514d30b}"},
			{HKEY_CURRENT_USER,   L"Software\\Classes\\" DEFAULT_APP_PROG_ID L"\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}", NULL,          SZ_CLSID_NFOPREVIEWHANDLER},
			{HKEY_CURRENT_USER,   L"Software\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers",                   SZ_CLSID_NFOPREVIEWHANDLER, SZ_NFOPREVIEWHANDLER},
		};

		hr = S_OK;
		for(int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
		{
			hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);
		}

		if(!SUCCEEDED(hr))
		{
			/* try to clean up nicely */
			DeleteRegKeys();
		}
	}

	if(SUCCEEDED(hr))
	{
		// This tells the shell to invalidate the thumbnail cache.  This is important because any associated files
		// viewed before registering this handler would otherwise show cached blank thumbnails.
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	return hr;
}


STDAPI DllUnregisterServer()
{
	return DeleteRegKeys();
}
