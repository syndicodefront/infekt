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
	CSettingsSection(const std::wstring& a_path);
	virtual ~CSettingsSection();

	virtual DWORD ReadDword(const wchar_t* a_szKey, DWORD a_default = 0) = 0;
	virtual bool WriteDword(const wchar_t* a_szKey, DWORD a_newValue) = 0;
	virtual std::wstring ReadString(const wchar_t* a_szKey, std::wstring a_default = L"") = 0;
	virtual bool WriteString(const wchar_t* a_szKey, std::wstring a_newValue) = 0;

	virtual bool DeletePair(const wchar_t* a_szKey) = 0;

	virtual bool ReadBool(const wchar_t* a_szKey, bool a_default);
	virtual bool WriteBool(const wchar_t* a_szKey, bool a_newValue);
protected:
	std::wstring m_name;
};

typedef std::shared_ptr<CSettingsSection> PSettingsSection;


class CSettingsBackend
{
public:
	CSettingsBackend(const std::wstring& a_appIdf);
	virtual ~CSettingsBackend();

	virtual bool OpenSectionForReading(const std::wstring& a_name, PSettingsSection& ar_section) = 0;
	virtual bool OpenSectionForWriting(const std::wstring& a_name, PSettingsSection& ar_section) = 0;

	virtual std::wstring GetName() const = 0;
protected:
	std::wstring m_appIdf;
};

typedef std::shared_ptr<CSettingsBackend> PSettingsBackend;


/************************************************************************/
/* Registry-powered Backend                                             */
/************************************************************************/

class CRegistrySettingsBackend : public CSettingsBackend
{
public:
	CRegistrySettingsBackend(const std::wstring& a_appIdf);
	virtual ~CRegistrySettingsBackend();

	bool OpenSectionForReading(const std::wstring& a_name, PSettingsSection& ar_section);
	bool OpenSectionForWriting(const std::wstring& a_name, PSettingsSection& ar_section);

	std::wstring GetName() const { return L"Registry"; }
	HKEY GetHive() const { return m_hHive; }
protected:
	HKEY m_hHive;
};


class CRegistrySettingsSection : public CSettingsSection
{
public:
	CRegistrySettingsSection(const std::wstring& a_path, HKEY a_hKey);
	virtual ~CRegistrySettingsSection();

	DWORD ReadDword(const wchar_t* a_szName, DWORD a_default = 0);
	bool WriteDword(const wchar_t* a_szName, DWORD a_newValue);
	std::wstring ReadString(const wchar_t* a_szName, std::wstring a_default = L"");
	bool WriteString(const wchar_t* a_szName, std::wstring a_newValue);

	bool DeletePair(const wchar_t* a_szKey);
protected:
	HKEY m_hKey;
};


/************************************************************************/
/* INI File-powered Backend                                             */
/************************************************************************/

class CINISettingsBackend : public CSettingsBackend
{
public:
	CINISettingsBackend(const std::wstring& a_iniPath);
	virtual ~CINISettingsBackend();

	bool OpenSectionForReading(const std::wstring& a_name, PSettingsSection& ar_section);
	bool OpenSectionForWriting(const std::wstring& a_name, PSettingsSection& ar_section);

	std::wstring GetName() const { return L"INI File"; }
};


class CINISettingsSection : public CSettingsSection
{
public:
	CINISettingsSection(const std::wstring& a_iniPath, const std::wstring& a_sectionName);
	virtual ~CINISettingsSection();

	DWORD ReadDword(const wchar_t* a_szName, DWORD a_default = 0);
	bool WriteDword(const wchar_t* a_szName, DWORD a_newValue);
	std::wstring ReadString(const wchar_t* a_szName, std::wstring a_default = L"");
	bool WriteString(const wchar_t* a_szName, std::wstring a_newValue);

	bool DeletePair(const wchar_t* a_szKey);
protected:
	std::wstring m_iniPath;
};


#endif
