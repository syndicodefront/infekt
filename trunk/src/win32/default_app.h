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

#ifndef _DEFAULT_APP_H
#define _DEFAULT_APP_H


// abstract base class

class CWinDefaultApp
{
public:
	/**
	 * @param a_appRegistryName e.g. "Company.App.MajorVer"
	 * @param a_fileExtension e.g. ".htm"
	 **/
	CWinDefaultApp(const std::wstring& a_appRegistryName, const std::wstring& a_fileExtension) :
		  m_appRegistryName(a_appRegistryName), m_extension(a_fileExtension)
	{ }
	virtual ~CWinDefaultApp()
	{ }

	virtual bool IsDefault() = 0;
	virtual bool MakeDefault() = 0;
protected:
	std::wstring m_appRegistryName;
	std::wstring m_extension;
private:
	CWinDefaultApp() {}
};


// class for Windows Vista and higher, using COM

class CWin6xDefaultApp : public CWinDefaultApp 
{
public:
	CWin6xDefaultApp(const std::wstring& a, const std::wstring& b);
	bool IsDefault();
	bool MakeDefault();
	bool GotNoSuchProgramName() const { return m_noSuchProgName; }
protected:
	bool m_noSuchProgName;
};


#if _WIN32_WINNT < 0x600

// class for Windows 2000 and XP, using direct registry writes

class CWin5xDefaultApp : public CWinDefaultApp 
{
public:
	CWin5xDefaultApp(const std::wstring& a, const std::wstring& b) : CWinDefaultApp(a, b)
	{ }

	bool IsDefault();
	bool MakeDefault();
protected:
	bool RegisterProgIdData();
};

#endif /* _WIN32_WINNT */

#endif /* !_DEFAULT_APP_H */
