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
#include "app.h"
#include "main_frame.h"
#include "settings_dlg.h"
#include "about_dlg.h"
#include "plugin_manager.h"
#include "nfo_renderer_export.h"
#include "resource.h"

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x31A
#endif


using namespace std;


enum _toolbar_button_ids {
	TBBID_OPEN = 9001, // over 9000!
	TBBID_SETTINGS,
	TBBID_VIEW_RENDERED,
	TBBID_VIEW_CLASSIC,
	TBBID_VIEW_TEXTONLY,
	TBBID_ABOUT,
	TBBID_CLEARMRU,
	TBBID_SHOWMENU,
	TBBID_OPENMRUSTART // must be the last item in this list.
};

#define VIEW_MENU_POS 1


CMainFrame::CMainFrame() : CFrame(),
	m_showingAbout(false), m_dropHelper(NULL)
{
	SetView(m_view);
}


void CMainFrame::PreCreate(CREATESTRUCT& cs)
{
	// allow some class flags to be set...
	CFrame::PreCreate(cs);

	// read saved positions and dimensions:
	PSettingsSection l_sect;

	if(dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(L"Frame Settings", l_sect))
	{
#if 0
		if(!::GetAsyncKeyState(VK_SHIFT))
		{
			// holding down SHIFT during startup causes the
			// saved position to be discarded
#else
		if(true)
		{
			// GetAsyncKeyState and GetKeyState randomly return true... wtf.
#endif

			cs.x = l_sect->ReadDword(L"Left");
			cs.y = l_sect->ReadDword(L"Top");
			cs.cx = l_sect->ReadDword(L"Width");
			cs.cy = l_sect->ReadDword(L"Height");

			m_bShowStatusbar = l_sect->ReadBool(L"Statusbar", true);
			m_bShowToolbar = l_sect->ReadBool(L"Toolbar", true);
		}
	}

	if(cs.cx == 0 && cs.cy == 0)
	{
		// default dimensions:
		cs.cx = 630;
		cs.cy = 680;
	}

	// get work area of the target (saved) monitor
	// and perform checks based on that:
	POINT l_pt = { cs.x, cs.y };
	MONITORINFO l_moInfo = { sizeof(MONITORINFO), 0 };
	HMONITOR l_hMonitor = ::MonitorFromPoint(l_pt, MONITOR_DEFAULTTOPRIMARY);

	if(::GetMonitorInfo(l_hMonitor, &l_moInfo))
	{
		RECT l_wa = l_moInfo.rcWork;

		// don't let the window grow larger than the size of the work area
		if(cs.cx > l_wa.right - l_wa.left)
			cs.cx = l_wa.right - l_wa.left;
		if(cs.cy > l_wa.bottom - l_wa.top)
			cs.cy = l_wa.bottom - l_wa.top;

		// try to always keep the window visible
		if(cs.x >= l_wa.right - 20)
			cs.x = l_wa.right - cs.cx;
		if(cs.y >= l_wa.bottom - 100)
			cs.y = l_wa.bottom - cs.cy;

		// this check is necessary in case the monitor setup
		// has changed since the last time iNFEKT ran
		if(cs.x < l_wa.left)
			cs.x = l_wa.left;
		if(cs.y < l_wa.top)
			cs.y = l_wa.top;
	}

	// enforce minimum window size:
	if(cs.cx < ms_minWidth)
		cs.cx = ms_minWidth;
	if(cs.cy < ms_minHeight)
		cs.cy = ms_minHeight;

	// hide the window to avoid flicker:
	cs.style &= ~WS_VISIBLE;

	// we need a unique class name for the "single window" functionality:
	cs.lpszClass = INFEKT_MAIN_WINDOW_CLASS_NAME;
}


void CMainFrame::OnCreate()
{
	// load settings:
	LoadOpenMruList();

	m_settings = PMainSettings(new CMainSettings(true));

	// tame Win32++:
	m_bUseThemes = FALSE;
	m_bShowIndicatorStatus = FALSE;
	m_bShowMenuStatus = FALSE;

	CFrame::OnCreate();

	SetIconLarge(IDI_APPICON);
	SetIconSmall(IDI_APPICON);
}


void CMainFrame::OnInitialUpdate()
{
	if(m_settings->bCenterWindow)
	{
		CenterWindow();
	}

	UpdateCaption();

	LoadRenderSettingsFromRegistry(_T("RenderedView"), m_view.GetRenderCtrl().get());
	LoadRenderSettingsFromRegistry(_T("ClassicView"), m_view.GetClassicCtrl().get());
	LoadRenderSettingsFromRegistry(_T("TextOnlyView"), m_view.GetTextOnlyCtrl().get());

	UpdateStatusbar();

	if(GetSettings()->iDefaultView == -1)
	{
		SwitchView((EMainView)GetSettings()->iLastView);
	}
	else
	{
		SwitchView((EMainView)GetSettings()->iDefaultView);
	}

	m_dropHelper = new (std::nothrow) CMainDropTargetHelper(m_hWnd);
	if(m_dropHelper)
	{
		::CoLockObjectExternal(m_dropHelper, TRUE, FALSE);
		::RegisterDragDrop(m_hWnd, m_dropHelper);
	}

	PSettingsSection l_sect;
	bool l_maximize = false;

	if(dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(L"Frame Settings", l_sect))
	{
		l_maximize = l_sect->ReadBool(L"Maximized", false);
		l_sect.reset();
	}

	m_view.SetCopyOnSelect(m_settings->bCopyOnSelect);
	m_view.SetCenterNfo(m_settings->bCenterNFO);

	ShowWindow(l_maximize ? SW_MAXIMIZE : SW_SHOWNORMAL);

	if(m_settings->bCheckDefaultOnStartup)
	{
		dynamic_cast<CNFOApp*>(GetApp())->CheckDefaultNfoViewer(m_hWnd, false);
	}

	std::_tstring l_path = dynamic_cast<CNFOApp*>(GetApp())->GetStartupFilePath();
	if(!l_path.empty())
	{
		::SetCursor(::LoadCursor(NULL, IDC_WAIT));

		OpenFile(l_path);
	}

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}


void CMainFrame::SetupToolbar()
{
	// turn off Win32++ "theme":
	CRebar& RB = GetRebar();
	ThemeRebar RBTheme = RB.GetRebarTheme();
	RBTheme.UseThemes = FALSE;
	RB.SetRebarTheme(RBTheme);

	// add main window menu into a rebar:
	GetMenubar().SetMenu(::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAIN_MENU)));

	// always show grippers in classic themes:
	RB.ShowGripper(RB.IDToIndex(IDW_TOOLBAR), !CThemeAPI::GetInstance()->IsThemeActive());
	RB.ShowGripper(RB.IDToIndex(IDW_MENUBAR), FALSE);

	AddToolbarButtons();

	ShowMenuBar(m_settings->bAlwaysShowMenubar);
}


void CMainFrame::AddToolbarButtons()
{
	const int IML = 0;

	// add icons:
	enum _icon_type {
		ICO_FILEOPEN = 0,
		ICO_INFO,
		ICO_SETTINGS,
		ICO_VIEW_RENDERED,
		ICO_VIEW_CLASSIC,
		ICO_VIEW_TEXTONLY,
		ICO_SHOWMENU,
		_ICOMAX
	};

	HIMAGELIST l_imgLst = ImageList_Create(22, 22, ILC_COLOR32, _ICOMAX, 0);
	GetToolbar().SendMessage(TB_SETIMAGELIST, IML, (LPARAM)l_imgLst);
	// CToolbar::OnDestroy destroys the image list...

	int l_icoId[_ICOMAX] = {0};
	l_icoId[ICO_FILEOPEN] =	CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_FILEOPEN,	22, 22);
	l_icoId[ICO_INFO] =		CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_INFO,		22, 22);
	l_icoId[ICO_SETTINGS] =	CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_SETTINGS,	22, 22);
	l_icoId[ICO_VIEW_RENDERED]	= CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_VIEW_RENDERED,	22, 22);
	l_icoId[ICO_VIEW_CLASSIC]	= CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_VIEW_CLASSIC,		22, 22);
	l_icoId[ICO_VIEW_TEXTONLY]	= CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_VIEW_TEXTONLY,	22, 22);
	l_icoId[ICO_SHOWMENU]		= CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_SHOWMENU,			22,	22);

	const BYTE defState = TBSTATE_ENABLED;
	const BYTE defStyle = BTNS_AUTOSIZE;

#define _TBBTN(A_ICO, A_IDF, A_STATE, A_STYLE, A_TEXT) \
		{ MAKELONG(l_icoId[A_ICO], IML), A_IDF, A_STATE, A_STYLE, {0}, 0, (INT_PTR)_T(A_TEXT) }
#define _TBSEP { 0, 0, 0, BTNS_SEP, {0}, 0, 0 }

	// define buttons:
	TBBUTTON l_btns[] = {
		_TBBTN(ICO_FILEOPEN, TBBID_OPEN, defState, defStyle | BTNS_DROPDOWN, "Open (Ctrl+O)"),
		_TBSEP,
		_TBBTN(ICO_SETTINGS, TBBID_SETTINGS, defState, defStyle, "Settings (F6)"),
		_TBBTN(ICO_SHOWMENU, TBBID_SHOWMENU, defState, defStyle | BTNS_CHECK, "Show Menu (Alt)"),
		_TBSEP,
		_TBBTN(ICO_VIEW_RENDERED, TBBID_VIEW_RENDERED, defState | TBSTATE_CHECKED, defStyle | BTNS_CHECKGROUP, "Rendered (F2)"),
		_TBBTN(ICO_VIEW_CLASSIC, TBBID_VIEW_CLASSIC, defState, defStyle | BTNS_CHECKGROUP, "Classic (F3)"),
		_TBBTN(ICO_VIEW_TEXTONLY, TBBID_VIEW_TEXTONLY, defState, defStyle | BTNS_CHECKGROUP, "Text Only (F4)"),
		_TBSEP,
		_TBBTN(ICO_INFO, TBBID_ABOUT, defState, defStyle, "About (F1)"),
	};

#undef _TBSEP
#undef _TBBTN

	GetToolbar().SendMessage(TB_ADDBUTTONS, sizeof(l_btns) / sizeof(l_btns[0]), (LPARAM)&l_btns);

	// adjust size of the toolbar and the parent rebar:
	GetToolbar().SendMessage(TB_AUTOSIZE);
	SIZE l_updatedSize = GetToolbar().GetMaxSize();
	::SendMessage(GetToolbar().GetParent(), UWM_TOOLBAR_RESIZE,
		(WPARAM)GetToolbar().GetHwnd(), (LPARAM)&l_updatedSize);
}


void CMainFrame::ShowMenuBar(bool a_show)
{
	BOOL b = (a_show ? TRUE : FALSE);

	GetRebar().ShowBand(GetRebar().IDToIndex(IDW_MENUBAR), b);
	GetToolbar().SendMessage(TB_CHECKBUTTON, TBBID_SHOWMENU, b);

	m_menuBarVisible = a_show;
}


BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	DWORD l_item = LOWORD(wParam);

	if(l_item >= TBBID_OPENMRUSTART && l_item < TBBID_OPENMRUSTART + ms_mruLength)
	{
		l_item = l_item - TBBID_OPENMRUSTART;

		if(l_item < m_mruPaths.size())
		{
			OpenFile(m_mruPaths[l_item]);
		}

		return TRUE;
	}

	switch(l_item)
	{
	case IDM_EXIT:
		SendMessage(WM_CLOSE);
		return TRUE;

	case TBBID_OPEN:
	case IDM_FILE_OPEN:
		OpenChooseFileName();
		return TRUE;

	case TBBID_SETTINGS:
	case IDM_SETTINGS: {
		CSettingsWindowDialog l_dlg(m_hWnd);
		l_dlg.SetMainWin(this);
		l_dlg.DoModal();
		return TRUE; }

	case TBBID_SHOWMENU:
		ShowMenuBar(!m_menuBarVisible);
		m_settings->bAlwaysShowMenubar = m_menuBarVisible;
		m_settings->SaveToRegistry();
		return TRUE;

	case TBBID_ABOUT:
	case IDM_ABOUT:
		OnHelp();
		return TRUE;

	case IDMC_COPY:
		m_view.CopySelectedTextToClipboard();
		return TRUE;

	case IDMC_SELECTALL:
		m_view.SelectAll();
		return TRUE;

	case IDM_CHECKFORUPDATES:
		this->CheckForUpdates();
		return TRUE;

	case IDM_EXPORT_PNG:
	case IDM_EXPORT_PNG_TRANSP:
	case IDM_EXPORT_UTF8:
	case IDM_EXPORT_UTF16:
	case IDM_EXPORT_XHTML:
	case IDM_EXPORT_PDF:
	case IDM_EXPORT_PDF_DIN:
		DoNfoExport(LOWORD(wParam));
		return TRUE;

	case TBBID_VIEW_RENDERED:
	case IDM_VIEW_RENDERED:
		SwitchView(MAIN_VIEW_RENDERED);
		return TRUE;

	case TBBID_VIEW_CLASSIC:
	case IDM_VIEW_CLASSIC:
		SwitchView(MAIN_VIEW_CLASSIC);
		return TRUE;

	case TBBID_VIEW_TEXTONLY:
	case IDM_VIEW_TEXTONLY:
		SwitchView(MAIN_VIEW_TEXTONLY);
		return TRUE;

	case IDM_ALWAYSONTOP:
	case IDMC_ALWAYSONTOP:
		GetSettings()->bAlwaysOnTop = !GetSettings()->bAlwaysOnTop;
		UpdateAlwaysOnTop();
		return TRUE;

	case IDM_VIEW_RELOAD:
		m_view.ReloadFile();
		return TRUE;

	case TBBID_CLEARMRU:
		if(!m_mruPaths.empty())
		{
			// "empty list" menu item
			m_mruPaths.clear();
			SaveOpenMruList();
		}
		else
		{
			// "browse..." menu item
			OpenChooseFileName();
		}
		return TRUE;

	case IDM_ZOOM_IN:
		if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
			m_view.GetActiveCtrl()->ZoomIn();
		break;

	case IDM_ZOOM_OUT:
		if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
			m_view.GetActiveCtrl()->ZoomOut();
		break;

	case IDM_ZOOM_RESET:
		if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
			m_view.GetActiveCtrl()->ZoomReset();
		break;
	}

	return FALSE;
}


LRESULT CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR l_lpnm = (LPNMHDR)lParam;

	switch(l_lpnm->code)
	{
	case TBN_DROPDOWN:
		if(DoOpenMruMenu((LPNMTOOLBAR)lParam))
			return FALSE;
		break;
	}

	return CFrame::OnNotify(wParam, lParam);
}


bool CMainFrame::DoOpenMruMenu(const LPNMTOOLBAR a_lpnm)
{
	if(a_lpnm->iItem == TBBID_OPEN)
	{
		// Get the coordinates of the button:
		RECT rc;
		::SendMessage(a_lpnm->hdr.hwndFrom, TB_GETRECT, (WPARAM)a_lpnm->iItem, (LPARAM)&rc);        
		::MapWindowPoints(a_lpnm->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);                         

		// Create a temp menu:
		HMENU hPopupMenu = ::CreatePopupMenu();

		size_t l_idx = 0;
		for(vector<_tstring>::const_iterator it = m_mruPaths.begin(); it != m_mruPaths.end(); it++, l_idx++)
		{
			const wchar_t* l_entry = it->c_str();
			l_entry = ::PathFindFileName(l_entry);
			::AppendMenu(hPopupMenu, MF_STRING, TBBID_OPENMRUSTART + l_idx, l_entry);
		}

		if(m_mruPaths.size() == 0)
		{
			::AppendMenu(hPopupMenu, MF_STRING, TBBID_CLEARMRU, _T("Browse..."));
		}
		else
		{
			::AppendMenu(hPopupMenu, MF_SEPARATOR, 0, NULL);
			::AppendMenu(hPopupMenu, MF_STRING, TBBID_CLEARMRU, _T("Empty Recently Viewed List"));
		}

		// Set up the popup menu.
		// Set rcExclude equal to the button rectangle so that if the toolbar
		// is too close to the bottom of the screen, the menu will appear above
		// the button rather than below it.
		TPMPARAMS tpm;
		tpm.cbSize = sizeof(TPMPARAMS);
		tpm.rcExclude = rc;

		// Show the menu and wait for input.
		// If the user selects an item, its WM_COMMAND is sent.
		::TrackPopupMenuEx(hPopupMenu,
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,               
			rc.left, rc.bottom, m_hWnd, &tpm); 

		::DestroyMenu(hPopupMenu);

		return true;
	}

	return false;
}


void CMainFrame::OnHelp()
{
	if(!m_showingAbout)
	{
		CAboutDialog l_pAboutDialog(m_hWnd);
		l_pAboutDialog.SetMainWin(this);

		m_showingAbout = true;
		l_pAboutDialog.DoModal();
		m_showingAbout = false;
	}
}


void CMainFrame::SwitchView(EMainView a_view)
{
	if(CPluginManager::GetInstance()->TriggerViewChanging(a_view))
	{
		// WARNING: We use menu positions here exclusively. Using
		// the COMMAND identifiers just didn't work for no apparent reason :(

		HMENU l_hPopup = ::GetSubMenu(GetMenubar().GetMenu(), VIEW_MENU_POS);

		if(l_hPopup)
		{
			::CheckMenuRadioItem(l_hPopup, 0, 2,
				(a_view == MAIN_VIEW_RENDERED ? 0 : (a_view == MAIN_VIEW_CLASSIC ? 1 : 2)),
				MF_BYPOSITION);
		}

		GetToolbar().SendMessage(TB_CHECKBUTTON, TBBID_VIEW_RENDERED, (a_view == MAIN_VIEW_RENDERED ? TRUE : FALSE));
		GetToolbar().SendMessage(TB_CHECKBUTTON, TBBID_VIEW_CLASSIC, (a_view == MAIN_VIEW_CLASSIC ? TRUE : FALSE));
		GetToolbar().SendMessage(TB_CHECKBUTTON, TBBID_VIEW_TEXTONLY, (a_view == MAIN_VIEW_TEXTONLY ? TRUE : FALSE));

		m_settings->iLastView = a_view;
		if(m_settings->iDefaultView == -1)
		{
			m_settings->SaveToRegistry();
		}

		m_view.SwitchView(a_view);

		CPluginManager::GetInstance()->TriggerViewChanged();
	}
}


bool CMainFrame::OpenFile(const std::_tstring a_filePath)
// do not use a reference since it might be a string from m_mruPaths and that
// would turn out badly.
{
	if(m_view.OpenFile(a_filePath))
	{
		UpdateCaption();

		UpdateStatusbar();

		// update MRU list:
		for(vector<_tstring>::iterator it = m_mruPaths.begin(); it != m_mruPaths.end(); it++)
		{
			if(_tcsicmp(it->c_str(), a_filePath.c_str()) == 0)
			{
				m_mruPaths.erase(it);
				break;
			}
		}

		m_mruPaths.insert(m_mruPaths.begin(), a_filePath);

		if(m_mruPaths.size() > ms_mruLength)
		{
			m_mruPaths.erase(m_mruPaths.begin() + ms_mruLength, m_mruPaths.end());
		}

		if(m_settings->bKeepOpenMRU)
		{
			SaveOpenMruList();
		}

		if(m_settings->bAutoWidth)
		{
			WINDOWPLACEMENT l_wpl = { sizeof(WINDOWPLACEMENT), 0 };

			if(!GetWindowPlacement(l_wpl) || l_wpl.showCmd != SW_MAXIMIZE)
			{
				AdjustWindowToNFOWidth();
			}
		}

		// yay.
		return true;
	}

	return false;
}


void CMainFrame::UpdateCaption()
{
	wstring l_caption;

	if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
	{
		l_caption = m_view.GetNfoData()->GetFileName();
	}

	if(!l_caption.empty()) l_caption += _T(" - ");
	l_caption += _T("iNFekt v") + InfektVersionAsString();

	SetWindowText(l_caption.c_str());
}


void CMainFrame::UpdateStatusbar()
{
	if(!m_bShowStatusbar)
		return;

	CStatusbar& l_sb = GetStatusbar();

	l_sb.SendMessage(WM_SIZE, 0, 0);

	if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
	{
		RECT l_rc = l_sb.GetWindowRect();
		LONG l_width = l_rc.right - l_rc.left;

		const _tstring l_fileName = m_view.GetNfoData()->GetFileName();
		const _tstring l_charset = CNFOData::GetCharsetName(m_view.GetNfoData()->GetCharset());
		//const _tstring l_zoomPerc = FORMAT(L"%d%%", m_view.GetActiveCtrl()->GetZoom());

		_tstring l_timeInfo, l_sizeInfo;
		if(!m_view.GetNfoData()->GetFilePath().empty())
		{
			WIN32_FIND_DATA l_ff = {0};
			if(HANDLE l_hFile = ::FindFirstFile(m_view.GetNfoData()->GetFilePath().c_str(), &l_ff))
			{
				SYSTEMTIME l_sysTimeUTC = {0}, l_sysTime = {0};
				if(::FileTimeToSystemTime(&l_ff.ftLastWriteTime, &l_sysTimeUTC) &&
					::SystemTimeToTzSpecificLocalTime(NULL, &l_sysTimeUTC, &l_sysTime))
				{
					TCHAR l_date[100] = {0};

					if(::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &l_sysTime,
						NULL, l_date, 99) != 0)
					{
						l_timeInfo = l_date;
					}

					memset(l_date, 0, 100);

					if(::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &l_sysTime, NULL,
						l_date, 99) != 0)
					{
						l_timeInfo += _T(" ");
						l_timeInfo += l_date;
					}
				}

				ULARGE_INTEGER l_tmpFileSize;
				l_tmpFileSize.HighPart = l_ff.nFileSizeHigh;
				l_tmpFileSize.LowPart = l_ff.nFileSizeLow;

				TCHAR l_sizeBuf[100] = {0};

				if(::StrFormatByteSizeW(l_tmpFileSize.QuadPart, l_sizeBuf, 99))
				{
					l_sizeInfo = l_sizeBuf;
				}

				::FindClose(l_hFile);
			}
		}

		int l_sbWidths[5] = {0};
		l_sbWidths[1] = CUtil::StatusCalcPaneWidth(l_sb.GetHwnd(), l_timeInfo.c_str());
		l_sbWidths[2] = CUtil::StatusCalcPaneWidth(l_sb.GetHwnd(), l_sizeInfo.c_str());
		l_sbWidths[3] = CUtil::StatusCalcPaneWidth(l_sb.GetHwnd(), l_charset.c_str());
		//l_sbWidths[4] = CUtil::StatusCalcPaneWidth(l_sb.GetHwnd(), l_zoomPerc.c_str());
		l_sbWidths[4] = 25;

		int l_sbParts[5] = { l_width, 0 };
		for(int i = 1; i < 5; i++) l_sbParts[0] -= l_sbWidths[i];
		for(int i = 1; i < 5; i++) l_sbParts[i] = l_sbParts[i - 1] + l_sbWidths[i];

		l_sb.CreateParts(5, l_sbParts);
		l_sb.SetPartText(0, l_fileName.c_str());
		l_sb.SetPartText(1, l_timeInfo.c_str());
		l_sb.SetPartText(2, l_sizeInfo.c_str());
		l_sb.SetPartText(3, l_charset.c_str());
		//l_sb.SetPartText(4, l_zoomPerc.c_str());

		l_sb.SetSimple(FALSE);
	}
	else
	{
		l_sb.SendMessage(SB_SETTEXT, SB_SIMPLEID, (LPARAM)_T("Hit the Alt key to toggle the menu bar."));
		l_sb.SetSimple(TRUE);
	}
}


void CMainFrame::AdjustWindowToNFOWidth()
{
	int l_desiredWidth = static_cast<int>(m_view.GetActiveCtrl()->GetWidth());

	l_desiredWidth += ::GetSystemMetrics(SM_CXSIZEFRAME) * 2;
	l_desiredWidth += ::GetSystemMetrics(SM_CYVSCROLL);
	l_desiredWidth += 20; // some extra padding

	if(l_desiredWidth < ms_minWidth)
		l_desiredWidth = ms_minWidth;

	RECT l_rc;
	if(::GetWindowRect(GetHwnd(), &l_rc))
	{
		int l_mid = l_rc.left + (l_rc.right - l_rc.left) / 2;
		int l_oldLeft = l_rc.left;

		// center around previous location:
		l_rc.left = l_mid - l_desiredWidth / 2;

		// make sure windows don't go off-screen to the left:
		if(l_rc.left < 0 && l_oldLeft >= 0)
			l_rc.left = 0;

		// set width:
		l_rc.right = l_rc.left + l_desiredWidth;

		// perform extra checks on the new window size:
		POINT l_pt = { l_rc.left, l_rc.top };
		MONITORINFO l_moInfo = { sizeof(MONITORINFO), 0 };
		HMONITOR l_hMonitor = ::MonitorFromPoint(l_pt, MONITOR_DEFAULTTOPRIMARY);
		// use MonitorFromPoint instead of MonitorFromWindow to make the left-top corner count.

		if(::GetMonitorInfo(l_hMonitor, &l_moInfo))
		{
			RECT l_wa = l_moInfo.rcWork;

			// don't let the window grow larger than the size of the work area
			if(l_rc.right - l_rc.left > l_wa.right - l_wa.left)
			{
				l_rc.left = 0;
				l_rc.right = l_wa.right - l_wa.left;
			}
			// and don't allow parts of the window to go off-screen to the right:
			else if(l_rc.right > l_wa.right)
			{
				int l_excess = (l_rc.right - l_wa.right); 
				l_rc.left -= l_excess;
				l_rc.right -= l_excess;
			}
		}

		MoveWindow(l_rc);
	}
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	/* return TRUE if the message has been translated */

	switch(pMsg->message)
	{
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
	case WM_KEYDOWN:
		if(!m_view.ForwardFocusTypeMouseKeyboardEvent(pMsg))
		{
			return TRUE;
		}
		break;
	case WM_SYSKEYUP:
		if(pMsg->wParam == VK_MENU || pMsg->wParam == VK_F10)
		{
			ShowMenuBar(!m_menuBarVisible);

			/*if(m_menuBarVisible)
			{
				::SetFocus(GetMenubar().GetHwnd());
			} we can't let this happen or the menu bar will grab all keyboard events...
			... gonna have to figure this out some other day :TODO: */
		}
		return TRUE;
	}

	if(CWnd::PreTranslateMessage(pMsg))
	{
		return TRUE;
	}

	if(pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		static HACCEL hAccelTable = NULL;
		if(!hAccelTable) hAccelTable = ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_MAIN_KEYBOARD_SHORTCUTS));

		return ::TranslateAccelerator(m_hWnd, hAccelTable, pMsg);
	}

	return FALSE;
}


LRESULT CMainFrame::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_GETMINMAXINFO: {
		PMINMAXINFO l_info = (PMINMAXINFO)lParam;
		l_info->ptMinTrackSize.x = ms_minWidth;
		l_info->ptMinTrackSize.y = ms_minHeight;
		return 0; }
	case WM_LOAD_NFO: {
		const wstring l_path = (wchar_t*)wParam;
		if(::PathFileExists(l_path.c_str()))
		{
			OpenFile(l_path);
		}
		return 1; }
	case WM_SYNC_PLUGIN_TO_CORE:
		return CPluginManager::GetInstance()->SynchedPluginToCore((void*)lParam);
	case WM_COPYDATA: {
		const COPYDATASTRUCT* l_cpds = (PCOPYDATASTRUCT)lParam;
		if(l_cpds->dwData == WM_LOAD_NFO)
		{
			if(l_cpds->cbData < 1000 && l_cpds->lpData &&
				::IsTextUnicode(l_cpds->lpData, l_cpds->cbData, NULL))
			{
				const wstring l_path = (wchar_t*)l_cpds->lpData;
				if(::PathFileExists(l_path.c_str()))
				{
					OpenFile(l_path);

					WINDOWPLACEMENT wplm;
					if(GetWindowPlacement(wplm))
					{
						wplm.showCmd = SW_RESTORE;
						SetWindowPlacement(wplm);
					}

					ShowWindow(SW_SHOW);
					BringWindowToTop();
					SetForegroundWindow();
					SetActiveWindow();

					return TRUE;
				}
			}
		}
		break; }
	case WM_SIZE:
		UpdateStatusbar();
		break; // also invoke default
	case WM_THEMECHANGED: {
		CRebar& RB = GetRebar();
		RB.ShowGripper(RB.IDToIndex(IDW_TOOLBAR), !CThemeAPI::GetInstance()->IsThemeActive());
		break; }
	case WM_DESTROY:
		::RevokeDragDrop(m_hWnd);
		if(m_dropHelper)
		{
			::CoLockObjectExternal(m_dropHelper, FALSE, TRUE);
			m_dropHelper->Release();
		}
		break; // also invoke default
	}

	return WndProcDefault(uMsg, wParam, lParam);
}


void CMainFrame::UpdateAlwaysOnTop()
{
	if(GetSettings()->bAlwaysOnTop)
	{
		this->SetForegroundWindow();
		this->SetFocus();
		this->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		::CheckMenuItem(::GetSubMenu(GetMenubar().GetMenu(), VIEW_MENU_POS), IDM_ALWAYSONTOP, MF_CHECKED | MF_BYCOMMAND);
	}
	else
	{
		this->SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		::CheckMenuItem(::GetSubMenu(GetMenubar().GetMenu(), VIEW_MENU_POS), IDM_ALWAYSONTOP, MF_UNCHECKED | MF_BYCOMMAND);
	}

	GetSettings()->SaveToRegistry();
}


void CMainFrame::OpenChooseFileName()
{
	COMDLG_FILTERSPEC l_filter[] = { { L"NFO Files", L"*.nfo;*.diz;*.asc" },
		{ L"Text Files", L"*.txt;*.nfo;*.diz;*.sfv" }, { L"All Files", L"*" } };

	_tstring l_filePath = CUtil::OpenFileDialog(g_hInstance, GetHwnd(),
		_T("NFO Files\0*.nfo;*.diz;*.asc\0Text Files\0*.txt;*.nfo;*.diz;*.sfv\0All Files\0*\0\0"),
		l_filter, 3);

	if(!l_filePath.empty())
	{
		OpenFile(l_filePath);
	}
}


void CMainFrame::DoNfoExport(UINT a_id)
{
	if(!m_view.GetNfoData() || !m_view.GetNfoData()->HasData())
	{
		this->MessageBox(_T("No file has been loaded!"), _T("Error"), MB_ICONEXCLAMATION);
		return;
	}

	_tstring l_baseFileName = m_view.GetNfoData()->GetFileName();
	TCHAR* l_buf = new TCHAR[l_baseFileName.size() + 1];
	_tcscpy_s(l_buf, l_baseFileName.size() + 1, l_baseFileName.c_str());
	PathRemoveExtension(l_buf);
	l_baseFileName = l_buf;
	delete[] l_buf;

	_tstring l_defaultPath;
	if(m_settings->bDefaultExportToNFODir)
	{
		l_defaultPath = CUtil::PathRemoveFileSpec(m_view.GetNfoData()->GetFilePath());
	}

	if(a_id == IDM_EXPORT_PNG || a_id == IDM_EXPORT_PNG_TRANSP)
	{
		COMDLG_FILTERSPEC l_filter[] = { { L"PNG File", L"*.png" } };

		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("PNG File\0*.png\0\0"), l_filter, 1, _T("png"), l_baseFileName + _T(".png"), l_defaultPath);

		if(!l_filePath.empty())
		{
			CNFORenderer l_renderer(m_view.GetViewType() != MAIN_VIEW_RENDERED);
			CNFORenderSettings l_settings = m_view.GetActiveCtrl()->GetSettings();

			if(a_id == IDM_EXPORT_PNG_TRANSP)
			{
				l_settings.cBackColor.A = 0;
			}

			bool l_internalError = true;

			::SetCursor(::LoadCursor(NULL, IDC_WAIT));

			l_renderer.InjectSettings(l_settings);

			if(l_renderer.AssignNFO(m_view.GetActiveCtrl()->GetNfoData()))
			{
				size_t l_imgWidth = l_renderer.GetWidth(), l_imgHeight = l_renderer.GetHeight();

				_ASSERT(l_imgWidth < (unsigned int)std::numeric_limits<int>::max() && l_imgHeight < (unsigned int)std::numeric_limits<int>::max());

				if(cairo_surface_t *l_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)l_imgWidth, (int)l_imgHeight))
				{
					if(l_renderer.DrawToSurface(l_surface, 0, 0, 0, 0, (int)l_imgWidth, (int)l_imgHeight))
					{
						const std::string l_utfFilePath = CUtil::FromWideStr(l_filePath, CP_UTF8);
						if(cairo_surface_write_to_png(l_surface, l_utfFilePath.c_str()) != CAIRO_STATUS_SUCCESS)
						{
							this->MessageBox(_T("Unable to open file for writing!"), _T("Fail"), MB_ICONEXCLAMATION);
						}
						else
						{
							this->MessageBox(_T("File saved!"), _T("Success"), MB_ICONINFORMATION);
						}

						l_internalError = false;
					}

					cairo_surface_destroy(l_surface);
				}
			}

			::SetCursor(::LoadCursor(NULL, IDC_ARROW));

			if(l_internalError)
			{
				this->MessageBox(_T("An internal error occured!"), _T("Fail"), MB_ICONEXCLAMATION);
			}
		}
	}
	else if(a_id == IDM_EXPORT_UTF8 || a_id == IDM_EXPORT_UTF16)
	{
		bool l_utf8 = (a_id == IDM_EXPORT_UTF8);

		COMDLG_FILTERSPEC l_filter[] = { { L"NFO File", L"*.nfo" }, { L"Text File", L"*.txt" } };

		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("NFO File\0*.nfo;\0Text File\0*.txt\0\0"), l_filter, 2, _T("nfo"),
			l_baseFileName + (l_utf8 ? _T("-utf8.nfo") : _T("-utf16.nfo")), l_defaultPath);

		if(!l_filePath.empty())
		{
			if(m_view.GetActiveCtrl()->GetNfoData()->SaveToFile(l_filePath, l_utf8))
			{
				this->MessageBox(_T("File saved!"), _T("Success"), MB_ICONINFORMATION);
			}
			else
			{
				std::wstring l_msg = m_view.GetActiveCtrl()->GetNfoData()->GetLastErrorDescription();

				if(l_msg.empty())
				{
					l_msg = L"Writing to the file failed. Please select a different file or folder.";
				}

				this->MessageBox(l_msg.c_str(), _T("Fail"), MB_ICONEXCLAMATION);
			}
		}
	}
	else if(a_id == IDM_EXPORT_XHTML)
	{
		COMDLG_FILTERSPEC l_filter[] = { { L"(X)HTML File", L"*.html" } };

		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("(X)HTML File\0*.html\0\0"), l_filter, 1, _T("html"),
			l_baseFileName + _T(".html"), l_defaultPath);

		if(!l_filePath.empty())
		{
			CNFOToHTML l_exporter(m_view.GetActiveCtrl()->GetNfoData());
			l_exporter.SetSettings(m_view.GetActiveCtrl()->GetSettings());
			l_exporter.SetTitle(l_baseFileName + _T(".nfo"));

			wstring l_html = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
			l_html += l_exporter.GetHTML();

			string l_utf8 = CUtil::FromWideStr(l_html, CP_UTF8);

			FILE* l_file;
			if(_tfopen_s(&l_file, l_filePath.c_str(), _T("wb")) == 0 && l_file)
			{
				fwrite(l_utf8.c_str(), l_utf8.size(), 1, l_file);
				fclose(l_file);

				this->MessageBox(_T("File saved!"), _T("Success"), MB_ICONINFORMATION);
			}
			else
			{
				this->MessageBox(_T("Unable to open file for writing!"), _T("Fail"), MB_ICONEXCLAMATION);
			}
		}
	}
	else if(a_id == IDM_EXPORT_PDF || a_id == IDM_EXPORT_PDF_DIN)
	{
		COMDLG_FILTERSPEC l_filter[] = { { L"PDF File", L"*.pdf" } };

		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("PDF File\0*.pdf\0\0"), l_filter, 1, _T("pdf"),
			l_baseFileName + _T(".pdf"), l_defaultPath);

		if(!l_filePath.empty())
		{
			CNFOToPDF l_exporter(m_view.GetActiveCtrl() != m_view.GetRenderCtrl());
			l_exporter.SetUseDINSizes(a_id == IDM_EXPORT_PDF_DIN);
			l_exporter.AssignNFO(m_view.GetNfoData());
			l_exporter.InjectSettings(m_view.GetActiveCtrl()->GetSettings());

			if(l_exporter.SavePDF(l_filePath))
			{
				this->MessageBox(_T("File saved!"), _T("Success"), MB_ICONINFORMATION);
			}
			else
			{
				this->MessageBox(_T("An error occured while trying to save this NFO as PDF!"), _T("Fail"), MB_ICONEXCLAMATION);
			}
		}
	}
}


bool CMainFrame::SaveRenderSettingsToRegistry(const std::_tstring& a_key,
	const CNFORenderSettings& a_settings, bool a_classic)
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForWriting(a_key, l_sect))
	{
		return false;
	}

	l_sect->WriteDword(L"ClrText", a_settings.cTextColor.AsWord());
	l_sect->WriteDword(L"ClrBack", a_settings.cBackColor.AsWord());
	l_sect->WriteDword(L"ClrArt", a_settings.cArtColor.AsWord());
	l_sect->WriteDword(L"ClrLink", a_settings.cHyperlinkColor.AsWord());

	l_sect->WriteBool(L"HilightHyperlinks", a_settings.bHilightHyperlinks);
	l_sect->WriteBool(L"UnderlineHyperlinks", a_settings.bUnderlineHyperlinks);
	l_sect->WriteBool(L"FontAntiAlias", a_settings.bFontAntiAlias);

	if(!a_classic)
	{
		l_sect->WriteDword(L"BlockHeight", static_cast<DWORD>(a_settings.uBlockHeight));
		l_sect->WriteDword(L"BlockWidth", static_cast<DWORD>(a_settings.uBlockWidth));

		l_sect->WriteBool(L"GaussShadow", a_settings.bGaussShadow);
		l_sect->WriteDword(L"GaussBlurRadius", a_settings.uGaussBlurRadius);

		l_sect->WriteDword(L"ClrGauss", a_settings.cGaussColor.AsWord());
	}
	else
	{
		l_sect->WriteDword(L"FontSize", static_cast<DWORD>(a_settings.uFontSize));
		l_sect->WriteBool(L"WrapLines", a_settings.bWrapLines);
	}

	return l_sect->WriteString(L"FontName", a_settings.sFontFace); /* somewhat representative success check */
}


bool CMainFrame::LoadRenderSettingsFromRegistry(const std::_tstring& a_key, CNFORenderer* a_target)
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(a_key, l_sect))
	{
		return false;
	}

	CNFORenderer l_dummy;
	CNFORenderSettings l_newSets, l_defaults = l_dummy.GetSettings();

	l_newSets.cTextColor = _s_color_t(l_sect->ReadDword(L"ClrText", l_defaults.cTextColor.AsWord()));
	l_newSets.cBackColor = _s_color_t(l_sect->ReadDword(L"ClrBack", l_defaults.cBackColor.AsWord()));
	l_newSets.cArtColor = _s_color_t(l_sect->ReadDword(L"ClrArt", l_defaults.cArtColor.AsWord()));
	l_newSets.cHyperlinkColor = _s_color_t(l_sect->ReadDword(L"ClrLink", l_defaults.cHyperlinkColor.AsWord()));

	l_newSets.bHilightHyperlinks = l_sect->ReadBool(L"HilightHyperlinks", l_defaults.bHilightHyperlinks);
	l_newSets.bUnderlineHyperlinks = l_sect->ReadBool(L"UnderlineHyperlinks", l_defaults.bUnderlineHyperlinks);
	l_newSets.bFontAntiAlias = l_sect->ReadBool(L"FontAntiAlias", l_defaults.bFontAntiAlias);
	l_newSets.bWrapLines = l_sect->ReadBool(L"WrapLines", l_defaults.bWrapLines);

	if(!a_target->IsClassicMode())
	{
		l_newSets.cGaussColor = _s_color_t(l_sect->ReadDword(L"ClrGauss", l_defaults.cGaussColor.AsWord()));
		l_newSets.bGaussShadow = l_sect->ReadBool(L"GaussShadow", l_defaults.bGaussShadow);
		l_newSets.uBlockHeight = l_sect->ReadDword(L"BlockHeight", (DWORD)l_defaults.uBlockHeight);
		l_newSets.uBlockWidth = l_sect->ReadDword(L"BlockWidth", (DWORD)l_defaults.uBlockWidth);
		l_newSets.uGaussBlurRadius = l_sect->ReadDword(L"GaussBlurRadius", (DWORD)l_defaults.uGaussBlurRadius);
	}
	else
	{
		l_newSets.uFontSize = l_sect->ReadDword(L"FontSize", (DWORD)l_defaults.uFontSize);
	}

	std::wstring l_fontFace = l_sect->ReadString(L"FontName");
	if(l_fontFace.size() > 0 && l_fontFace.size() < LF_FACESIZE)
	{
		_tcsncpy_s(l_newSets.sFontFace, LF_FACESIZE + 1, l_fontFace.c_str(), l_fontFace.size());
	}

	a_target->InjectSettings(l_newSets);

	return true;
}


const std::_tstring CMainFrame::InfektVersionAsString()
{
	return FORMAT(_T("%d.%d.%d"), INFEKT_VERSION_MAJOR % INFEKT_VERSION_MINOR % INFEKT_VERSION_REVISION);
}


void CMainFrame::CheckForUpdates()
{
	const _tstring l_url(_T("https://infekt.googlecode.com/svn/wiki/CurrentVersion.wiki"));
	const _tstring l_projectUrl(_T("http://infekt.googlecode.com/"));

	::SetCursor(::LoadCursor(NULL, IDC_WAIT));

	_tstring l_contents = CUtil::DownloadHttpTextFile(l_url);
	wstring l_serverVersion, l_newDownloadUrl;

	_tstring::size_type l_pos = l_contents.find(_T("{{{")), l_endPos, l_prevPos;

	if(l_pos != _tstring::npos)
	{
		l_pos += 3;
		l_endPos = l_contents.find(_T("}}}"), l_pos);

		if(l_endPos != _tstring::npos)
		{
			map<const _tstring, _tstring> l_pairs;
			l_contents = l_contents.substr(l_pos, l_endPos - l_pos);

			l_prevPos = 0;
			l_pos = l_contents.find(_T('\n'));

			while(l_pos != _tstring::npos)
			{
				_tstring l_line = l_contents.substr(l_prevPos, l_pos - l_prevPos);
				_tstring::size_type l_equalPos = l_line.find(_T('='));

				if(l_equalPos != _tstring::npos)
				{
					_tstring l_left = l_line.substr(0, l_equalPos);
					_tstring l_right = l_line.substr(l_equalPos + 1);
					CUtil::StrTrim(l_left);
					CUtil::StrTrim(l_right);

					l_pairs[l_left] = l_right;
				}

				l_prevPos = l_pos + 1;
				l_pos = l_contents.find(_T('\n'), l_prevPos);
			}

			l_serverVersion = l_pairs[_T("latest[stable].1")];
			l_newDownloadUrl = l_pairs[_T("download_latest[stable].1")];
		}
	}

	l_contents.clear();

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	if(l_serverVersion.empty())
	{
		const _tstring l_msg = _T("We failed to contact infekt.googlecode.com to get the latest version's info. ")
			_T("Please make sure you are connected to the internet and try again later.\n\nDo you want to visit ") +
			l_projectUrl + _T(" now instead?");

		if(this->MessageBox(l_msg.c_str(), _T("Connection Problem"), MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
		{
			::ShellExecute(0, _T("open"), l_projectUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}

		return;
	}

	int l_result = CUtil::VersionCompare(InfektVersionAsString(), l_serverVersion);

	if(l_result == 0)
	{
		const _tstring l_msg = _T("You are using the latest stable version (") + InfektVersionAsString() + _T(")!");
		this->MessageBox(l_msg.c_str(), _T("Nice."), MB_ICONINFORMATION);
	}
	else if(l_result < 0)
	{
		if(l_newDownloadUrl.empty()) l_newDownloadUrl = l_projectUrl;

		const _tstring l_msg = _T("Attention! There is a new version (iNFekt v") + l_serverVersion + _T(") available!") +
			_T("\n\nDo you want to go to the download page now?");

		if(this->MessageBox(l_msg.c_str(), _T("New Version Found"), MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
		{
			if(::PathIsURL(l_newDownloadUrl.c_str()) &&
				(l_newDownloadUrl.find(_T("http://")) == 0 || l_newDownloadUrl.find(_T("https://")) == 0))
			{
				::ShellExecute(0, _T("open"), l_newDownloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
	else if(l_result > 0)
	{
		this->MessageBox(_T("Looks like you compiled from source. Your version is newer than the latest stable one!"),
			_T("Nice."), MB_ICONINFORMATION);
	}
}


void CMainFrame::LoadOpenMruList()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(L"OpenMRU", l_sect))
	{
		return;
	}

	m_mruPaths.clear();

	for(size_t i = 0; i < ms_mruLength; i++)
	{
		const wstring l_valName = FORMAT(L"%d", i);
		const std::wstring l_path = l_sect->ReadString(l_valName.c_str());

		if(!l_path.empty() && ::PathFileExists(l_path.c_str()))
		{
			m_mruPaths.push_back(l_path);
		}
	}
}


void CMainFrame::SaveOpenMruList()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForWriting(L"OpenMRU", l_sect))
	{
		return;
	}

	vector<_tstring>::const_iterator l_it = m_mruPaths.begin();
	for(size_t i = 0; i < ms_mruLength; i++)
	{
		const wstring l_valName = FORMAT(L"%d", i);

		if(l_it != m_mruPaths.end())
		{
			l_sect->WriteString(l_valName.c_str(), *l_it);
			l_it++;
		}
		else
		{
			l_sect->DeletePair(l_valName.c_str());
		}
	}
}


void CMainFrame::SavePositionSettings()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForWriting(L"Frame Settings", l_sect))
	{
		return;
	}

	WINDOWPLACEMENT l_wpl = { sizeof(WINDOWPLACEMENT), 0 };
	if(GetWindowPlacement(l_wpl))
	{
		CRect rc = l_wpl.rcNormalPosition;

		l_sect->WriteDword(L"Top", rc.top);
		l_sect->WriteDword(L"Left", rc.left);
		l_sect->WriteDword(L"Width", MAX(rc.Width(), ms_minWidth));
		l_sect->WriteDword(L"Height", MAX(rc.Height(), ms_minHeight));

		l_sect->WriteBool(L"Maximized", l_wpl.showCmd == SW_MAXIMIZE);
	}

	l_sect->WriteBool(L"Toolbar", m_bShowToolbar != FALSE);
	l_sect->WriteBool(L"Statusbar", m_bShowStatusbar != FALSE);
}


void CMainFrame::OnClose()
{
	SavePositionSettings();

	if(!m_settings->bKeepOpenMRU)
	{
		m_mruPaths.clear();
		SaveOpenMruList();
	}

	CFrame::OnClose();
}


CMainFrame::~CMainFrame()
{
	if(CCudaUtil::GetInstance()->IsCudaThreadInitialized())
	{
		CCudaUtil::GetInstance()->UnInitCudaThread();
	}
}


/************************************************************************/
/* CMAINSETTINGS                                                        */
/************************************************************************/

bool CMainSettings::SaveToRegistry()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForWriting(L"MainSettings", l_sect))
	{
		return false;
	}

	l_sect->WriteDword(L"DefaultView", this->iDefaultView);
	l_sect->WriteDword(L"LastView", this->iLastView);
	l_sect->WriteBool(L"CopyOnSelect", this->bCopyOnSelect);
	l_sect->WriteBool(L"AlwaysOnTop", this->bAlwaysOnTop);
	l_sect->WriteBool(L"AlwaysShowMenubar", this->bAlwaysShowMenubar);
	l_sect->WriteBool(L"CheckDefViewOnStart", this->bCheckDefaultOnStartup);
	l_sect->WriteBool(L"KeepOpenMRU", this->bKeepOpenMRU);

	l_sect->WriteBool(L"CenterWindow", this->bCenterWindow);
	l_sect->WriteBool(L"AutoWidth", this->bAutoWidth);
	l_sect->WriteBool(L"CenterNFO", this->bCenterNFO);
	l_sect->WriteBool(L"DefaultExportToNFODir", this->bDefaultExportToNFODir);

	// "deputy" return value:
	return l_sect->WriteBool(L"SingleInstanceMode", this->bSingleInstanceMode);
}


bool CMainSettings::LoadFromRegistry()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(L"MainSettings", l_sect))
	{
		return false;
	}

	DWORD dwDefaultView = l_sect->ReadDword(L"DefaultView"),
		dwLastView = l_sect->ReadDword(L"LastView");

	if(dwDefaultView == -1 || (dwDefaultView >= MAIN_VIEW_RENDERED && dwDefaultView < _MAIN_VIEW_MAX))
	{
		this->iDefaultView = dwDefaultView;
	}

	if(dwLastView >= MAIN_VIEW_RENDERED && dwLastView < _MAIN_VIEW_MAX)
	{
		this->iLastView = dwLastView;
	}

	this->bCopyOnSelect = l_sect->ReadBool(L"CopyOnSelect", false);
	this->bAlwaysOnTop = l_sect->ReadBool(L"AlwaysOnTop", false);
	this->bAlwaysShowMenubar = l_sect->ReadBool(L"AlwaysShowMenubar", false);
	this->bCheckDefaultOnStartup = l_sect->ReadBool(L"CheckDefViewOnStart", true);
	this->bSingleInstanceMode = l_sect->ReadBool(L"SingleInstanceMode", false);
	this->bKeepOpenMRU = l_sect->ReadBool(L"KeepOpenMRU", true);

	this->bCenterWindow = l_sect->ReadBool(L"CenterWindow", true);
	this->bAutoWidth = l_sect->ReadBool(L"AutoWidth", false);
	this->bCenterNFO = l_sect->ReadBool(L"CenterNFO", true);
	this->bDefaultExportToNFODir = l_sect->ReadBool(L"DefaultExportToNFODir", false);

	return true;
}


/************************************************************************/
/* Drop Target Helper                                                   */
/************************************************************************/

CMainDropTargetHelper::CMainDropTargetHelper(HWND a_hwnd)
{
	m_hwnd = a_hwnd;
	m_refCount = 1;
	m_allowDrop = false;
}


HRESULT _stdcall CMainDropTargetHelper::QueryInterface(REFIID iid, void** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
}

ULONG _stdcall CMainDropTargetHelper::AddRef()
{
	return ::InterlockedIncrement(&m_refCount);
}

ULONG _stdcall CMainDropTargetHelper::Release()
{
	LONG l_new = ::InterlockedDecrement(&m_refCount);

	if(l_new == 0)
	{
		delete this;
	}

	return l_new;
}


HRESULT _stdcall CMainDropTargetHelper::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		*pdwEffect = DROPEFFECT_MOVE;

		m_allowDrop = true;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;

		m_allowDrop = false;
	}

	return S_OK;
}

HRESULT _stdcall CMainDropTargetHelper::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect = (m_allowDrop ? DROPEFFECT_MOVE : DROPEFFECT_NONE);

	return S_OK;
}

HRESULT _stdcall CMainDropTargetHelper::DragLeave()
{
	return S_OK;
}

HRESULT _stdcall CMainDropTargetHelper::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if(!m_allowDrop)
	{
		return E_UNEXPECTED;
	}

	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
	{
		HDROP l_hDrop = (HDROP)::GlobalLock(stgmed.hGlobal);

		if(l_hDrop)
		{
			UINT l_numFiles = ::DragQueryFile(l_hDrop, 0xFFFFFFFF, NULL, 0);

			if(l_numFiles > 0)
			{
				wchar_t l_fileNameBuf[1000] = {0};
				UINT l_charsCopied = ::DragQueryFile(l_hDrop, 0, l_fileNameBuf, 999); // get the first file.

				if(l_charsCopied > 0 && l_charsCopied < 1000)
				{
					::SendMessage(m_hwnd, WM_LOAD_NFO, (WPARAM)l_fileNameBuf, l_charsCopied);
				}
			}
		}

		::GlobalUnlock(stgmed.hGlobal);

		// release the data using the COM API
		::ReleaseStgMedium(&stgmed);
	}

	return S_OK;
}



CMainDropTargetHelper::~CMainDropTargetHelper()
{
}
