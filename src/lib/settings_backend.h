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

#ifndef _SETTINGS_BACKEND_H
#define _SETTINGS_BACKEND_H

/************************************************************************/
/* Generic Backend Interface Classes To Derive From                     */
/************************************************************************/

class CSettingsSection
{
public:
	CSettingsSection(const std::_tstring& a_path);
	virtual ~CSettingsSection();

	virtual DWORD ReadDword(const TCHAR* a_szKey, DWORD a_default = 0) = 0;
	virtual bool WriteDword(const TCHAR* a_szKey, DWORD a_newValue) = 0;
	virtual std::_tstring ReadString(const TCHAR* a_szKey, std::_tstring a_default = _T("")) = 0;
	virtual bool WriteString(const TCHAR* a_szKey, std::_tstring a_newValue) = 0;

	virtual bool DeletePair(const TCHAR* a_szKey) = 0;

	virtual bool ReadBool(const TCHAR* a_szKey, bool a_default);
	virtual bool WriteBool(const TCHAR* a_szKey, bool a_newValue);
protected:
	std::_tstring m_name;
};

typedef boost::shared_ptr<CSettingsSection> PSettingsSection;


class CSettingsBackend
{
public:
	CSettingsBackend(const std::_tstring& a_appIdf);
	virtual ~CSettingsBackend();

	virtual bool OpenSectionForReading(const std::_tstring& a_name, PSettingsSection& ar_section) = 0;
	virtual bool OpenSectionForWriting(const std::_tstring& a_name, PSettingsSection& ar_section) = 0;
protected:
	std::_tstring m_appIdf;
};

typedef boost::shared_ptr<CSettingsBackend> PSettingsBackend;


/************************************************************************/
/* Registry-powered Backend                                             */
/************************************************************************/

class CRegistrySettingsBackend : public CSettingsBackend
{
public:
	CRegistrySettingsBackend(const std::_tstring& a_appIdf);
	virtual ~CRegistrySettingsBackend();

	bool OpenSectionForReading(const std::_tstring& a_name, PSettingsSection& ar_section);
	bool OpenSectionForWriting(const std::_tstring& a_name, PSettingsSection& ar_section);

	HKEY GetHive() const { return m_hHive; }
protected:
	HKEY m_hHive;
};


class CRegistrySettingsSection : public CSettingsSection
{
public:
	CRegistrySettingsSection(CSettingsBackend* a_backend, const std::_tstring& a_path, HKEY a_hKey);
	virtual ~CRegistrySettingsSection();

	DWORD ReadDword(const TCHAR* a_szName, DWORD a_default = 0);
	bool WriteDword(const TCHAR* a_szName, DWORD a_newValue);
	std::_tstring ReadString(const TCHAR* a_szName, std::_tstring a_default = _T(""));
	bool WriteString(const TCHAR* a_szName, std::_tstring a_newValue);

	bool DeletePair(const TCHAR* a_szKey);
protected:
	CRegistrySettingsBackend* m_backend;
	HKEY m_hKey;
};


/************************************************************************/
/* INI File-powered Backend                                             */
/************************************************************************/


#endif
