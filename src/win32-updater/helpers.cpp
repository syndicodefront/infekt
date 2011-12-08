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

#include "updater.h"
#include <Wincrypt.h>


/************************************************************************/
/* HELPER METHODS                                                       */
/************************************************************************/

std::wstring GetSysDirPath()
{
	wchar_t l_buf[1000] = {0};

	if(::GetSystemDirectory(l_buf, 999))
	{
		::PathRemoveBackslash(l_buf);

		return l_buf;
	}

	return L"";
}


std::wstring GetTempFilePath(const std::wstring& a_suffix)
{
	wchar_t l_buf[1000] = {0};

	if(::GetTempPath(999, l_buf))
	{
		::PathAddBackslash(l_buf);

		std::wstring l_tempDir = l_buf, l_path;

		do 
		{
			swprintf_s(l_buf, 999, L"%x", GetTickCount());

			l_path = l_tempDir + std::wstring(l_buf) + a_suffix;
		} while(::PathFileExists(l_path.c_str()));

		return l_path;
	}

	return L"";
}


std::wstring GetExePath()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);

	return l_buf2;
}


bool ShellExecuteAndWait(const std::wstring& a_path, const std::wstring& a_parameters, int nShowCmd, bool a_requireZeroExitCode)
{
	SHELLEXECUTEINFO l_sei = { sizeof(SHELLEXECUTEINFO), 0 };

	l_sei.lpVerb = L"open";
	l_sei.lpFile = a_path.c_str();
	l_sei.lpParameters = a_parameters.c_str();
	l_sei.nShow = nShowCmd;
	l_sei.fMask = SEE_MASK_NOCLOSEPROCESS;

	if(::ShellExecuteEx(&l_sei))
	{
		if(::WaitForSingleObject(l_sei.hProcess, 10000) == WAIT_OBJECT_0)
		{
			if(a_requireZeroExitCode)
			{
				DWORD dwExitCode = 0;
				::GetExitCodeProcess(l_sei.hProcess, &dwExitCode);

				::CloseHandle(l_sei.hProcess);

				// *pukes* quick and dirty code.
				return (dwExitCode == 0);
			}
			else
			{
				::CloseHandle(l_sei.hProcess);
			}
		}
		// else: accept possible resource leak, but keep program responding

		return true;
	}

	return false;
}


std::wstring SHA1_File(const std::wstring& a_filePath)
{
	HCRYPTPROV hProv;
	std::wstring l_result;

	if(::CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET))
	{
		HCRYPTHASH hHash;

		if(::CryptCreateHash(hProv, CALG_SHA1, NULL, 0, &hHash))
		{
			FILE *fInput = NULL;

			if(_wfopen_s(&fInput, a_filePath.c_str(), L"rb") == 0)
			{
				char pbBuf[4096] = {0};

				while(!feof(fInput))
				{
					DWORD dwRead = static_cast<DWORD>(
						fread_s(pbBuf, 4096, sizeof(char), 4096, fInput));

					::CryptHashData(hHash, (BYTE*)pbBuf, dwRead, 0);
				}

				fclose(fInput);

				DWORD dwHashLen = 0;
				DWORD dwHashLenSize = sizeof(DWORD);

				if(::CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&dwHashLen, &dwHashLenSize, 0))
				{
					BYTE *pbHashBuf = new BYTE[dwHashLen];
					memset(pbHashBuf, 0, dwHashLen);

					if(::CryptGetHashParam(hHash, HP_HASHVAL, pbHashBuf, &dwHashLen, 0))
					{
						wchar_t wbHex[3] = {0};

						for(DWORD i = 0; i < dwHashLen; i++)
						{
							swprintf_s(wbHex, 3, L"%02x", pbHashBuf[i]);
							l_result += wbHex;
						}
					}

					delete[] pbHashBuf;
				}
			}

			::CryptDestroyHash(hHash);
		}

		::CryptReleaseContext(hProv, 0);
	}

	return l_result;
}
