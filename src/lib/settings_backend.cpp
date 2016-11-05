/**
 * Copyright (C) 2010 syndicode
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
#include "settings_backend.h"
#include "util.h"


/************************************************************************/
/* Generic Backend                                                      */
/************************************************************************/

CSettingsBackend::CSettingsBackend(const std::wstring& a_appIdf)
{
	m_appIdf = a_appIdf;
}

CSettingsBackend::~CSettingsBackend()
{
}

CSettingsSection::CSettingsSection(const std::wstring& a_name)
{
	m_name = a_name;
}

CSettingsSection::~CSettingsSection()
{
}

bool CSettingsSection::ReadBool(const wchar_t* a_szKey, bool a_default)
{
	return (ReadDword(a_szKey, (a_default ? 1 : 0)) != 0 ? 1 : 0);
}

bool CSettingsSection::WriteBool(const wchar_t* a_szKey, bool a_newValue)
{
	return WriteDword(a_szKey, (a_newValue ? 1 : 0));
}


/************************************************************************/
/* Registry-powered Backend                                             */
/************************************************************************/

CRegistrySettingsBackend::CRegistrySettingsBackend(const std::wstring& a_appIdf) :
	CSettingsBackend(a_appIdf)
{
	m_hHive = HKEY_CURRENT_USER;
}

CRegistrySettingsBackend::~CRegistrySettingsBackend()
{
}

bool CRegistrySettingsBackend::OpenSectionForReading(const std::wstring& a_name, PSettingsSection& ar_section)
{
	HKEY l_hKey;
	const std::wstring l_path = m_appIdf + a_name;

	if (RegOpenKeyEx(GetHive(), l_path.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		//_ASSERT(false);
		return false;
	}

	ar_section = PSettingsSection(
		new CRegistrySettingsSection(a_name, l_hKey));

	return true;
}

bool CRegistrySettingsBackend::OpenSectionForWriting(const std::wstring& a_name, PSettingsSection& ar_section)
{
	HKEY l_hKey;
	const std::wstring l_path = m_appIdf + a_name;

	if (RegCreateKeyEx(GetHive(), l_path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_QUERY_VALUE, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		_ASSERT(false);
		return false;
	}

	ar_section = PSettingsSection(
		new CRegistrySettingsSection(a_name, l_hKey));

	return true;
}


CRegistrySettingsSection::CRegistrySettingsSection(const std::wstring& a_path, HKEY a_hKey) :
	CSettingsSection(a_path)
{
	m_hKey = a_hKey;
}

CRegistrySettingsSection::~CRegistrySettingsSection()
{
	RegCloseKey(m_hKey);
}

DWORD CRegistrySettingsSection::ReadDword(const wchar_t* a_szName, DWORD a_default)
{
	DWORD l_dwType;
	DWORD l_dwBuf = 0;
	DWORD l_dwBufSz = sizeof(DWORD);

	if (RegQueryValueEx(m_hKey, a_szName, NULL, &l_dwType,
		(LPBYTE)&l_dwBuf, &l_dwBufSz) == ERROR_SUCCESS && l_dwType == REG_DWORD)
	{
		return l_dwBuf;
	}

	return a_default;
}

bool CRegistrySettingsSection::WriteDword(const wchar_t* a_szName, DWORD a_newValue)
{
	bool l_success = (RegSetValueEx(m_hKey, a_szName, 0, REG_DWORD, (LPBYTE)&a_newValue, sizeof(DWORD)) == ERROR_SUCCESS);

	_ASSERT(l_success);

	return l_success;
}

std::wstring CRegistrySettingsSection::ReadString(const wchar_t* a_szName, std::wstring a_default)
{
	wchar_t l_buffer[1000] = { 0 };
	DWORD l_dwType, l_dwSize = 999 * sizeof(wchar_t);

	if (RegQueryValueEx(m_hKey, a_szName, 0, &l_dwType, (LPBYTE)l_buffer,
		&l_dwSize) == ERROR_SUCCESS && l_dwType == REG_SZ)
	{
		return l_buffer;
	}

	return a_default;
}

bool CRegistrySettingsSection::WriteString(const wchar_t* a_szName, std::wstring a_newValue)
{
	bool l_success = (RegSetValueEx(m_hKey, a_szName, 0, REG_SZ, (LPBYTE)a_newValue.c_str(),
		(DWORD)(a_newValue.size() + 1) * sizeof(wchar_t)) == ERROR_SUCCESS);

	_ASSERT(l_success);

	return l_success;
}

bool CRegistrySettingsSection::DeletePair(const wchar_t* a_szKey)
{
	LSTATUS l_result = RegDeleteValue(m_hKey, a_szKey);

	return (l_result == ERROR_SUCCESS || l_result == ERROR_FILE_NOT_FOUND);
}


/************************************************************************/
/* INI File-powered Backend                                             */
/************************************************************************/

CINISettingsBackend::CINISettingsBackend(const std::wstring& a_iniPath) :
	CSettingsBackend(a_iniPath)
{

}

CINISettingsBackend::~CINISettingsBackend()
{
}

bool CINISettingsBackend::OpenSectionForReading(const std::wstring& a_name, PSettingsSection& ar_section)
{
	if (_waccess_s(m_appIdf.c_str(), 4) != 0) // 4 = reading
	{
		return false;
	}

	ar_section = PSettingsSection(new CINISettingsSection(m_appIdf, a_name));

	return true;
}

bool CINISettingsBackend::OpenSectionForWriting(const std::wstring& a_name, PSettingsSection& ar_section)
{
	if (_waccess_s(m_appIdf.c_str(), 6) != 0) // 6 = reading and writing
	{
		return false;
	}

	ar_section = PSettingsSection(new CINISettingsSection(m_appIdf, a_name));

	return true;
}


CINISettingsSection::CINISettingsSection(const std::wstring& a_iniPath, const std::wstring& a_sectionName) :
	CSettingsSection(a_sectionName)
{
	m_iniPath = a_iniPath;
}

CINISettingsSection::~CINISettingsSection()
{
}

DWORD CINISettingsSection::ReadDword(const wchar_t* a_szName, DWORD a_default)
{
	return (DWORD)GetPrivateProfileInt(m_name.c_str(), a_szName, (INT)a_default, m_iniPath.c_str());
}

bool CINISettingsSection::WriteDword(const wchar_t* a_szName, DWORD a_newValue)
{
	std::wstring l_val = FORMAT(L"%d", a_newValue);

	return (WritePrivateProfileString(m_name.c_str(), a_szName, l_val.c_str(), m_iniPath.c_str()) != FALSE);
}

std::wstring CINISettingsSection::ReadString(const wchar_t* a_szName, std::wstring a_default)
{
	wchar_t l_buf[1000] = { 0 };

	if (GetPrivateProfileString(m_name.c_str(), a_szName, a_default.c_str(), l_buf, 999, m_iniPath.c_str()) < 1000)
	{
		return l_buf;
	}

	return a_default;
}

bool CINISettingsSection::WriteString(const wchar_t* a_szName, std::wstring a_newValue)
{
	return (WritePrivateProfileString(m_name.c_str(), a_szName, a_newValue.c_str(), m_iniPath.c_str()) != FALSE);
}

bool CINISettingsSection::DeletePair(const wchar_t* a_szName)
{
	return (WritePrivateProfileString(m_name.c_str(), a_szName, NULL, m_iniPath.c_str()) != FALSE);
}
