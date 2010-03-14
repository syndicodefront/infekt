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
#include "resource.h"

using namespace std;


enum _toolbar_button_ids {
	TBBID_OPEN = 30000,
	TBBID_SETTINGS,
	TBBID_VIEW_RENDERED,
	TBBID_VIEW_CLASSIC,
	TBBID_VIEW_TEXTONLY,
	TBBID_ABOUT,
	TBBID_CLEARMRU,
	TBBID_OPENMRUSTART // must be the last item in this list.
};

#define VIEW_MENU_POS 1


CMainFrame::CMainFrame() : CFrame(),
	m_showingAbout(false), m_dropHelper(NULL)
{
	SetView(m_view);

	LoadRegistrySettings(_T("cxxjoe\\iNFEKT"));
	LoadOpenMruList();

	m_settings = PMainSettings(new CMainSettings(true));
}


void CMainFrame::PreCreate(CREATESTRUCT& cs)
{
	// allow width+height to be read from registry
	// also allow class flags to be set.
	CFrame::PreCreate(cs);

	if(cs.cx == 0 && cs.cy == 0)
	{
		// default dimensions:
		cs.cx = 630;
		cs.cy = 680;
	}

	// avoid stupid things:
	if(cs.cx < ms_minWidth)
		cs.cx = ms_minWidth;
	if(cs.cy < ms_minHeight)
		cs.cy = ms_minHeight;

	// we center the window in OnInitialUpdate() anyway:
	cs.x = cs.y = CW_USEDEFAULT;

	// hide the window to avoid flicker:
	cs.style &= ~WS_VISIBLE;

	// we need a unique class name for the "single window" functionality:
	cs.lpszClass = INFEKT_MAIN_WINDOW_CLASS_NAME;
}


void CMainFrame::OnCreate()
{
	m_bUseThemes = FALSE;
	m_bShowIndicatorStatus = FALSE;
	m_bShowMenuStatus = FALSE;

	CFrame::OnCreate();

	SetIconLarge(IDI_APPICON);
	SetIconSmall(IDI_APPICON);
}


void CMainFrame::OnInitialUpdate()
{
	CenterWindow();
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

	m_dropHelper = new CMainDropTargetHelper(m_hWnd);
	::CoLockObjectExternal(m_dropHelper, TRUE, FALSE);
	::RegisterDragDrop(m_hWnd, m_dropHelper);

	ShowWindow();

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

	// always show grippers:
	GetRebar().ShowGripper(GetRebar().IDToIndex(IDW_TOOLBAR), TRUE);
	GetRebar().ShowGripper(GetRebar().IDToIndex(IDW_MENUBAR), TRUE);

	ShowMenuBar(m_settings->bAlwaysShowMenubar);

	AddToolbarButtons();
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
	GetRebar().ShowBand(GetRebar().IDToIndex(IDW_MENUBAR), (a_show ? TRUE : FALSE));
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

	case TBBID_ABOUT:
	case IDM_ABOUT:
		SendMessage(WM_HELP);
		return TRUE;

	case IDMC_COPY:
		m_view.CopySelectedTextToClipboard();
		return TRUE;

	case IDMC_SELECTALL:
		m_view.SelectAll();
		return TRUE;

	case IDMC_COPYSHORTCUT:
		this->MessageBox(_T("Not implemented!"), _T("Fail"), MB_ICONEXCLAMATION);
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

	case TBBID_CLEARMRU:
		m_mruPaths.clear();
		SaveOpenMruList();
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

		::AppendMenu(hPopupMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenu(hPopupMenu, MF_STRING | (m_mruPaths.size() == 0 ? MF_DISABLED | MF_GRAYED : 0),
			TBBID_CLEARMRU, _T("Empty Recently Viewed List"));

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

		SaveOpenMruList();

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

		const wstring l_fileName = m_view.GetNfoData()->GetFileName();
		const _tstring l_charset = CNFOData::GetCharsetName(m_view.GetNfoData()->GetCharset());
		
		_tstring l_timeInfo, l_sizeInfo;
		if(!m_view.GetNfoData()->GetFilePath().empty())
		{
			WIN32_FIND_DATA l_ff = {0};
			if(HANDLE l_hFile = ::FindFirstFile(m_view.GetNfoData()->GetFilePath().c_str(), &l_ff))
			{
				SYSTEMTIME l_sysTime = {0};
				if(::FileTimeToSystemTime(&l_ff.ftLastWriteTime, &l_sysTime))
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

				long long l_fileSize = (static_cast<long long>(l_ff.nFileSizeHigh) *
					static_cast<long long>(MAXDWORD) + 1) +
					static_cast<long long>(l_ff.nFileSizeLow);

				TCHAR l_sizeBuf[100] = {0};

				if(::StrFormatByteSizeW(l_fileSize, l_sizeBuf, 99))
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
		l_sbWidths[4] = 25;

		int l_sbParts[5] = { l_width, 0 };
		for(int i = 1; i < 5; i++) l_sbParts[0] -= l_sbWidths[i];
		for(int i = 1; i < 5; i++) l_sbParts[i] = l_sbParts[i - 1] + l_sbWidths[i];

		l_sb.CreateParts(5, l_sbParts);
		l_sb.SetPartText(0, l_fileName.c_str());
		l_sb.SetPartText(1, l_timeInfo.c_str());
		l_sb.SetPartText(2, l_sizeInfo.c_str());
		l_sb.SetPartText(3, l_charset.c_str());

		l_sb.SetSimple(FALSE);
	}
	else
	{
		l_sb.SendMessage(SB_SETTEXT, SB_SIMPLEID, (LPARAM)_T("Hit the Alt key to toggle the menu bar."));
		l_sb.SetSimple(TRUE);
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
					GetWindowPlacement(wplm);
					wplm.showCmd = SW_RESTORE;
					SetWindowPlacement(wplm);

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
	case WM_DESTROY:
		::RevokeDragDrop(m_hWnd);
		::CoLockObjectExternal(m_dropHelper, FALSE, TRUE);
		m_dropHelper->Release();
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
	_tstring l_filePath = CUtil::OpenFileDialog(g_hInstance, GetHwnd(),
		_T("NFO Files\0*.nfo;*.diz;*.asc\0Text Files\0*.txt;*.nfo;*.diz;*.sfv\0All Files\0*\0\0"));

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

	if(a_id == IDM_EXPORT_PNG || a_id == IDM_EXPORT_PNG_TRANSP)
	{
		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("PNG File\0*.png\0\0"), _T("png"), l_baseFileName + _T(".png"));

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

				if(cairo_surface_t *l_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, l_imgWidth, l_imgHeight))
				{
					if(l_renderer.DrawToSurface(l_surface, 0, 0, 0, 0, l_imgWidth, l_imgHeight))
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
		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("NFO File\0*.nfo;\0Text File\0*.txt\0\0"), _T("nfo"),
			l_baseFileName + (l_utf8 ? _T("-utf8.nfo") : _T("-utf16.nfo")));

		if(!l_filePath.empty())
		{
			m_view.GetActiveCtrl()->GetNfoData()->SaveToFile(l_filePath, l_utf8);
		}
	}
	else if(a_id == IDM_EXPORT_XHTML)
	{
		const _tstring l_filePath = CUtil::SaveFileDialog(g_hInstance, GetHwnd(),
			_T("(X)HTML File\0*.html\0\0"), _T("html"),
			l_baseFileName + _T(".html"));

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
	else if(a_id == IDM_EXPORT_PDF)
	{
		// :TODO:
	}
}


bool CMainFrame::SaveRenderSettingsToRegistry(const std::_tstring& a_key,
	const CNFORenderSettings& a_settings, bool a_classic)
{
	const _tstring l_keyPath = _T("Software\\cxxjoe\\iNFEKT\\") + a_key;

	HKEY l_hKey;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	uint32_t dwTextColor = a_settings.cTextColor.AsWord(),
		dwBackColor = a_settings.cBackColor.AsWord(),
		dwArtColor = a_settings.cArtColor.AsWord(),
		dwLinkColor = a_settings.cHyperlinkColor.AsWord();

	RegSetValueEx(l_hKey, _T("ClrText"),	0, REG_DWORD, (LPBYTE)&dwTextColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrBack"),	0, REG_DWORD, (LPBYTE)&dwBackColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrArt"),		0, REG_DWORD, (LPBYTE)&dwArtColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrLink"),	0, REG_DWORD, (LPBYTE)&dwLinkColor,		sizeof(uint32_t));

	int32_t dwHilightHyperlinks = (a_settings.bHilightHyperlinks ? 1 : 0),
		dwUnderlineHyperlinks = (a_settings.bUnderlineHyperlinks ? 1 : 0),
		dwFontAA = (a_settings.bFontAntiAlias ? 1 : 0);

	RegSetValueEx(l_hKey, _T("HilightHyperlinks"),		0, REG_DWORD, (LPBYTE)&dwHilightHyperlinks,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("UnderlineHyperlinks"),	0, REG_DWORD, (LPBYTE)&dwUnderlineHyperlinks,	sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("FontAntiAlias"),			0, REG_DWORD, (LPBYTE)&dwFontAA,				sizeof(int32_t));

	if(!a_classic)
	{
		int32_t dwBlockHeight = a_settings.uBlockHeight,
			dwBlockWidth = a_settings.uBlockWidth,
			dwGaussShadow = (a_settings.bGaussShadow ? 1 : 0),
			dwGaussBlurRadius = a_settings.uGaussBlurRadius,
			dwGaussColor = a_settings.cGaussColor.AsWord();

		RegSetValueEx(l_hKey, _T("BlockHeight"),		0, REG_DWORD, (LPBYTE)&dwBlockHeight,		sizeof(int32_t));
		RegSetValueEx(l_hKey, _T("BlockWidth"),			0, REG_DWORD, (LPBYTE)&dwBlockWidth,		sizeof(int32_t));

		RegSetValueEx(l_hKey, _T("GaussShadow"),		0, REG_DWORD, (LPBYTE)&dwGaussShadow,		sizeof(int32_t));
		RegSetValueEx(l_hKey, _T("GaussBlurRadius"),	0, REG_DWORD, (LPBYTE)&dwGaussBlurRadius,	sizeof(int32_t));

		RegSetValueEx(l_hKey, _T("ClrGauss"),			0, REG_DWORD, (LPBYTE)&dwGaussColor,		sizeof(int32_t));
	}
	else
	{
		int32_t dwFontSize = a_settings.uFontSize;

		RegSetValueEx(l_hKey, _T("FontSize"),			0, REG_DWORD, (LPBYTE)&dwFontSize,			sizeof(int32_t));
	}

	RegSetValueEx(l_hKey, _T("FontName"), 0, REG_SZ, (LPBYTE)a_settings.sFontFace,
		(wcslen(a_settings.sFontFace) + 1) * sizeof(TCHAR));

	RegCloseKey(l_hKey);

	return true;
}


bool CMainFrame::LoadRenderSettingsFromRegistry(const std::_tstring& a_key, CNFORenderer* a_target)
{
	const _tstring l_keyPath = _T("Software\\cxxjoe\\iNFEKT\\") + a_key;

	HKEY l_hKey;
	if(!a_target || RegOpenKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, KEY_READ, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	CNFORenderSettings l_newSets;

	uint32_t dwTextColor = CUtil::RegQueryDword(l_hKey, _T("ClrText")),
		dwBackColor = CUtil::RegQueryDword(l_hKey, _T("ClrBack")),
		dwArtColor = CUtil::RegQueryDword(l_hKey, _T("ClrArt")),
		dwLinkColor = CUtil::RegQueryDword(l_hKey, _T("ClrLink"));

	l_newSets.cTextColor = _s_color_t(dwTextColor);
	l_newSets.cBackColor = _s_color_t(dwBackColor);
	l_newSets.cArtColor = _s_color_t(dwArtColor);
	l_newSets.cHyperlinkColor = _s_color_t(dwLinkColor);

	l_newSets.bHilightHyperlinks = (CUtil::RegQueryDword(l_hKey, _T("HilightHyperlinks"), 1) != 0);
	l_newSets.bUnderlineHyperlinks = (CUtil::RegQueryDword(l_hKey, _T("UnderlineHyperlinks"), 1) != 0);
	l_newSets.bFontAntiAlias = (CUtil::RegQueryDword(l_hKey, _T("FontAntiAlias"), 1) != 0);

	if(!a_target->IsClassicMode())
	{
		uint32_t dwGaussColor = CUtil::RegQueryDword(l_hKey, _T("ClrGauss")),
			dwGaussShadow = CUtil::RegQueryDword(l_hKey, _T("GaussShadow")),
			dwBlockHeight = CUtil::RegQueryDword(l_hKey, _T("BlockHeight")),
			dwBlockWidth = CUtil::RegQueryDword(l_hKey, _T("BlockWidth")),
			dwGaussBlurRadius = CUtil::RegQueryDword(l_hKey, _T("GaussBlurRadius"));

		l_newSets.cGaussColor = _s_color_t(dwGaussColor);
		l_newSets.bGaussShadow = (dwGaussShadow != 0);
		l_newSets.uBlockHeight = dwBlockHeight;
		l_newSets.uBlockWidth = dwBlockWidth;
		l_newSets.uGaussBlurRadius = dwGaussBlurRadius;
	}
	else
	{
		l_newSets.uFontSize = CUtil::RegQueryDword(l_hKey, _T("FontSize"));
	}

	TCHAR l_fontFaceBuf[LF_FACESIZE + 1] = {0};
	DWORD l_dwType, l_dwSize = (LF_FACESIZE + 1) * sizeof(TCHAR);
	if(RegQueryValueEx(l_hKey, _T("FontName"), 0, &l_dwType, (LPBYTE)l_fontFaceBuf,
		&l_dwSize) == ERROR_SUCCESS && l_dwType == REG_SZ)
	{
		_tcsncpy_s(l_newSets.sFontFace, LF_FACESIZE + 1, l_fontFaceBuf, LF_FACESIZE);
	}

	RegCloseKey(l_hKey);

	a_target->InjectSettings(l_newSets);

	return true;
}


const std::_tstring CMainFrame::InfektVersionAsString()
{
	return FORMAT(_T("%d.%d.%d"), INFEKT_VERSION_MAJOR % INFEKT_VERSION_MINOR % INFEKT_VERSION_REVISION);
}


void CMainFrame::CheckForUpdates()
{
	const _tstring l_url(_T("http://infekt.googlecode.com/svn/wiki/CurrentVersion.wiki"));
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
	m_mruPaths.clear();

	const _tstring l_keyPath = _T("Software\\cxxjoe\\iNFEKT\\OpenMRU");

	HKEY l_hKey;
	if(RegOpenKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, KEY_READ, &l_hKey) != ERROR_SUCCESS)
	{
		return;
	}

	for(size_t i = 0; i < ms_mruLength; i++)
	{
		const wstring l_valName = FORMAT(L"%d", i);
		DWORD dwType = -1;
		TCHAR dwBuf[1000] = {0};
		DWORD dwBufSize = sizeof(TCHAR) * 999;

		if(RegQueryValueEx(l_hKey, l_valName.c_str(), 0, &dwType, (LPBYTE)dwBuf, &dwBufSize) == ERROR_SUCCESS
			&& dwType == REG_SZ)
		{
			if(::PathFileExists(dwBuf))
			{
				m_mruPaths.push_back(dwBuf);
			}
		}
		else
		{
			break;
		}
	}

	RegCloseKey(l_hKey);
}


void CMainFrame::SaveOpenMruList()
{
	const _tstring l_keyPath = _T("Software\\cxxjoe\\iNFEKT\\OpenMRU");

	HKEY l_hKey;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return;
	}

	size_t i = 0;
	for(vector<_tstring>::const_iterator it = m_mruPaths.begin();
		it != m_mruPaths.end(); it++, i++)
	{
		const wstring l_valName = FORMAT(L"%d", i);

		RegSetValueEx(l_hKey, l_valName.c_str(), 0, REG_SZ,
			(LPBYTE)it->c_str(), (it->size() + 1) * sizeof(TCHAR));
	}

	RegCloseKey(l_hKey);
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
	const _tstring l_keyPath = _T("Software\\cxxjoe\\iNFEKT\\MainSettings");

	HKEY l_hKey;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	int32_t dwDefaultView = this->iDefaultView,
		dwLastView = this->iLastView,
		dwCopySelect = (this->bCopyOnSelect ? 1 : 0),
		dwAlwaysOnTop = (this->bAlwaysOnTop ? 1 : 0),
		dwAlwaysMenuBar = (this->bAlwaysShowMenubar ? 1 : 0),
		dwCheckDefault = (this->bCheckDefaultOnStartup ? 1 : 0),
		dwSingleInstance = (this->bSingleInstanceMode ? 1 : 0);

	RegSetValueEx(l_hKey, _T("DefaultView"),		0, REG_DWORD, (LPBYTE)&dwDefaultView,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("LastView"),			0, REG_DWORD, (LPBYTE)&dwLastView,			sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("CopyOnSelect"),		0, REG_DWORD, (LPBYTE)&dwCopySelect,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("AlwaysOnTop"),		0, REG_DWORD, (LPBYTE)&dwAlwaysOnTop,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("AlwaysShowMenubar"),	0, REG_DWORD, (LPBYTE)&dwAlwaysMenuBar,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("CheckDefViewOnStart"),0, REG_DWORD, (LPBYTE)&dwCheckDefault,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("SingleInstanceMode"),	0, REG_DWORD, (LPBYTE)&dwSingleInstance,	sizeof(int32_t));

	RegCloseKey(l_hKey);

	return true;
}


bool CMainSettings::LoadFromRegistry()
{
	const _tstring l_keyPath = _T("Software\\cxxjoe\\iNFEKT\\MainSettings");

	HKEY l_hKey;
	if(RegOpenKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, KEY_READ, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	int32_t dwDefaultView = CUtil::RegQueryDword(l_hKey, _T("DefaultView")),
		dwLastView = CUtil::RegQueryDword(l_hKey, _T("LastView")),
		dwCopySelect = CUtil::RegQueryDword(l_hKey, _T("CopyOnSelect")),
		dwAlwaysOnTop = CUtil::RegQueryDword(l_hKey, _T("AlwaysOnTop")),
		dwAlwaysMenuBar = CUtil::RegQueryDword(l_hKey, _T("AlwaysShowMenubar")),
		dwCheckDefault = CUtil::RegQueryDword(l_hKey, _T("CheckDefViewOnStart")),
		dwSingleInstance = CUtil::RegQueryDword(l_hKey, _T("SingleInstanceMode"));

	RegCloseKey(l_hKey);

	if(dwDefaultView == -1 || (dwDefaultView >= MAIN_VIEW_RENDERED && dwDefaultView < _MAIN_VIEW_MAX))
	{
		this->iDefaultView = dwDefaultView;
	}

	if(dwLastView >= MAIN_VIEW_RENDERED && dwLastView < _MAIN_VIEW_MAX)
	{
		this->iLastView = dwLastView;
	}

	this->bCopyOnSelect = (dwCopySelect != 0);
	this->bAlwaysOnTop = (dwAlwaysOnTop != 0);
	this->bAlwaysShowMenubar = (dwAlwaysMenuBar != 0);
	this->bCheckDefaultOnStartup = (dwCheckDefault != 0);
	this->bSingleInstanceMode = (dwSingleInstance != 0);

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
