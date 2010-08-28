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
#include "settings_backend.h"


/************************************************************************/
/* Generic Backend                                                      */
/************************************************************************/

CSettingsBackend::CSettingsBackend(const std::_tstring& a_appIdf)
{
	m_appIdf = a_appIdf;
}

CSettingsBackend::~CSettingsBackend()
{
}

CSettingsSection::CSettingsSection(const std::_tstring& a_name)
{
	m_name = a_name;
}

CSettingsSection::~CSettingsSection()
{
}

bool CSettingsSection::ReadBool(const TCHAR* a_szKey, bool a_default)
{
	return (ReadDword(a_szKey, (a_default ? 1 : 0)) != 0 ? 1 : 0);
}

bool CSettingsSection::WriteBool(const TCHAR* a_szKey, bool a_newValue)
{
	return WriteDword(a_szKey, (a_newValue ? 1 : 0));
}


/************************************************************************/
/* Registry-powered Backend                                             */
/************************************************************************/

CRegistrySettingsBackend::CRegistrySettingsBackend(const std::_tstring& a_appIdf) :
	CSettingsBackend(a_appIdf)
{
	m_hHive = HKEY_CURRENT_USER;
}

CRegistrySettingsBackend::~CRegistrySettingsBackend()
{
}

bool CRegistrySettingsBackend::OpenSectionForReading(const std::_tstring& a_name, PSettingsSection& ar_section)
{
	HKEY l_hKey;
	const std::_tstring l_path = m_appIdf + a_name;

	if(RegOpenKeyEx(GetHive(), l_path.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		//_ASSERT(false);
		return false;
	}

	ar_section = PSettingsSection(
		new CRegistrySettingsSection(this, a_name, l_hKey));

	return true;
}

bool CRegistrySettingsBackend::OpenSectionForWriting(const std::_tstring& a_name, PSettingsSection& ar_section)
{
	HKEY l_hKey;
	const std::_tstring l_path = m_appIdf + a_name;

	if(RegCreateKeyEx(GetHive(), l_path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_QUERY_VALUE, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		_ASSERT(false);
		return false;
	}

	ar_section = PSettingsSection(
		new CRegistrySettingsSection(this, a_name, l_hKey));

	return true;
}


CRegistrySettingsSection::CRegistrySettingsSection(CSettingsBackend* a_backend, const std::_tstring& a_path, HKEY a_hKey) :
	CSettingsSection(a_path)
{
	m_backend = dynamic_cast<CRegistrySettingsBackend*>(a_backend);
	m_hKey = a_hKey;
}

CRegistrySettingsSection::~CRegistrySettingsSection()
{
	RegCloseKey(m_hKey);
}

DWORD CRegistrySettingsSection::ReadDword(const TCHAR* a_szName, DWORD a_default)
{
	DWORD l_dwType;
	DWORD l_dwBuf = 0;
	DWORD l_dwBufSz = sizeof(DWORD);

	if(RegQueryValueEx(m_hKey, a_szName, NULL, &l_dwType,
		(LPBYTE)&l_dwBuf, &l_dwBufSz) == ERROR_SUCCESS && l_dwType == REG_DWORD)
	{
		return l_dwBuf;
	}

	return a_default;
}

bool CRegistrySettingsSection::WriteDword(const TCHAR* a_szName, DWORD a_newValue)
{
	bool l_success = (RegSetValueEx(m_hKey, a_szName, 0, REG_DWORD, (LPBYTE)&a_newValue, sizeof(DWORD)) == ERROR_SUCCESS);

	_ASSERT(l_success);

	return l_success;
}

std::_tstring CRegistrySettingsSection::ReadString(const TCHAR* a_szName, std::_tstring a_default)
{
	TCHAR l_buffer[1000] = {0};
	DWORD l_dwType, l_dwSize = 999 * sizeof(TCHAR);

	if(RegQueryValueEx(m_hKey, a_szName, 0, &l_dwType, (LPBYTE)l_buffer,
		&l_dwSize) == ERROR_SUCCESS && l_dwType == REG_SZ)
	{
		return l_buffer;
	}

	return a_default;
}

bool CRegistrySettingsSection::WriteString(const TCHAR* a_szName, std::_tstring a_newValue)
{
	bool l_success = (RegSetValueEx(m_hKey, a_szName, 0, REG_SZ, (LPBYTE)a_newValue.c_str(),
		(DWORD)(a_newValue.size() + 1) * sizeof(TCHAR)) == ERROR_SUCCESS);

	_ASSERT(l_success);

	return l_success;
}

bool CRegistrySettingsSection::DeletePair(const TCHAR* a_szKey)
{
	LSTATUS l_result = RegDeleteValue(m_hKey, a_szKey);

	return (l_result == ERROR_SUCCESS || l_result == ERROR_FILE_NOT_FOUND);
}
