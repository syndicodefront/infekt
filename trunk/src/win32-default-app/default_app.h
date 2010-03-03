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


struct CWinDefaultAppInfo
{
	std::wstring sAppRegistryName;
	std::wstring sExtension;
	std::wstring sExePath;
};


// abstract base class

class CWinDefaultApp
{
public:
	/**
	 * @param a_appRegistryName e.g. "Company.App.MajorVer"
	 * @param a_fileExtension e.g. ".htm"
	 **/
	CWinDefaultApp(const CWinDefaultAppInfo& a_info) :
		m_info(a_info)
	{ }
	virtual ~CWinDefaultApp()
	{ }

	virtual bool IsDefault() const = 0;
#ifdef MAKE_DEFAULT_APP_CAPAB
	virtual bool MakeDefault() const = 0;
	bool CWinDefaultApp::RegisterProgIdData() const;
#endif

	static std::wstring GetExePath();
protected:
	CWinDefaultAppInfo m_info;

private:
	CWinDefaultApp() {}
};


// class for Windows Vista and higher, using COM

class CWin6xDefaultApp : public CWinDefaultApp 
{
public:
	CWin6xDefaultApp(const CWinDefaultAppInfo& a_info);
	virtual ~CWin6xDefaultApp();
	bool IsDefault() const;
#ifdef MAKE_DEFAULT_APP_CAPAB
	bool MakeDefault() const;
#endif
};


// class for Windows 2000 and XP, using direct registry writes

class CWin5xDefaultApp : public CWinDefaultApp 
{
public:
	CWin5xDefaultApp(const CWinDefaultAppInfo& a_info) : CWinDefaultApp(a_info)
	{ }

	bool IsDefault() const;
#ifdef MAKE_DEFAULT_APP_CAPAB
	bool MakeDefault() const;
#endif
};

#endif /* !_DEFAULT_APP_H */
