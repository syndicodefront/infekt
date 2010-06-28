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
	TBBID_EDITCOPY,
	TBBID_ABOUT
};


CMainFrame::CMainFrame() : CFrame(),
	m_showingAbout(false)
{
	SetView(m_view);

	LoadRegistrySettings(_T("cxxjoe\\iNFEKT"));
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

	GetStatusbar().SetPartText(0, _T("Hit the Alt key to toggle the menu bar."));

	SwitchView(MAIN_VIEW_CLASSIC);

	ShowWindow();

	std::_tstring l_path = dynamic_cast<CNFOApp*>(GetApp())->GetStartupFilePath();
	if(!l_path.empty())
	{
		::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		m_view.OpenFile(l_path);
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

	// hide menubar by default:
	GetRebar().ShowBand(0, FALSE);
	m_menuBarVisible = false;

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
		ICO_EDITCOPY,
		_ICOMAX
	};

	HIMAGELIST l_imgLst = ImageList_Create(22, 22, ILC_COLOR32, _ICOMAX, 0);
	GetToolbar().SendMessage(TB_SETIMAGELIST, IML, (LPARAM)l_imgLst);
	// CToolbar::OnDestroy destroys the image list...

	int l_icoId[_ICOMAX] = {0};
	l_icoId[ICO_FILEOPEN] =	CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_FILEOPEN,	22, 22);
	l_icoId[ICO_INFO] =		CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_INFO,		22, 22);
	l_icoId[ICO_SETTINGS] =	CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_SETTINGS,	22, 22);
	l_icoId[ICO_EDITCOPY] =	CUtil::AddPngToImageList(l_imgLst, g_hInstance, IDB_PNG_EDITCOPY,	22, 22);

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
		_TBBTN(ICO_EDITCOPY, TBBID_EDITCOPY, 0, defStyle, "Copy (Ctrl+C)"),
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


BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
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

	case IDM_VIEW_RENDERED:
		SwitchView(MAIN_VIEW_RENDERED);
		break;

	case IDM_VIEW_CLASSIC:
		SwitchView(MAIN_VIEW_CLASSIC);
		break;

	case IDM_VIEW_TEXTONLY:
		SwitchView(MAIN_VIEW_TEXTONLY);
		break;
	}

	return FALSE;
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

	HMENU l_hPopup = ::GetSubMenu(GetMenubar().GetMenu(), 1);

	if(l_hPopup)
	{
		::CheckMenuRadioItem(l_hPopup, 0, 2,
			(a_view == MAIN_VIEW_RENDERED ? 0 : (a_view == MAIN_VIEW_CLASSIC ? 1 : 2)),
			MF_BYPOSITION);
	}

	m_view.SwitchView(a_view);
}


void CMainFrame::UpdateCaption()
{
	wstring l_caption;

	if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
	{
		l_caption = m_view.GetNfoData()->GetFileName();
	}

	if(!l_caption.empty()) l_caption += _T(" - ");
	l_caption += _T("iNFEKT v") + InfektVersionAsString();

	SetWindowText(l_caption.c_str());
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
			GetRebar().ShowBand(GetRebar().IDToIndex(IDW_MENUBAR), (m_menuBarVisible ? FALSE : TRUE));
			m_menuBarVisible = !m_menuBarVisible;

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
		MINMAXINFO* l_info = (MINMAXINFO*)lParam;
		l_info->ptMinTrackSize.x = ms_minWidth;
		l_info->ptMinTrackSize.y = ms_minHeight;
		return 0; }
	}

	return WndProcDefault(uMsg, wParam, lParam);
}


void CMainFrame::OpenChooseFileName()
{
	_tstring l_fileName = CUtil::OpenFileDialog(g_hInstance, GetHwnd(),
		_T("NFO Files\0*.nfo;*.diz\0Text Files\0*.txt;*.nfo;*.diz;*.sfv\0All Files\0*\0\0"));

	if(!l_fileName.empty())
	{
		m_view.OpenFile(l_fileName);

		UpdateCaption();
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
			CNFORenderSettings l_settings = m_view.GetActiveControl()->GetSettings();

			if(a_id == IDM_EXPORT_PNG_TRANSP)
			{
				l_settings.cBackColor.A = 0;
			}

			bool l_internalError = true;

			::SetCursor(::LoadCursor(NULL, IDC_WAIT));

			l_renderer.InjectSettings(l_settings);

			if(l_renderer.AssignNFO(m_view.GetNfoData()))
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
			m_view.GetNfoData()->SaveToFile(l_filePath, l_utf8);
		}
	}
	else if(a_id == IDM_EXPORT_XHTML)
	{
		// :TODO:
	}
	else if(a_id == IDM_EXPORT_PDF)
	{
		// :TODO:
	}
}


bool CMainFrame::SaveRenderSettingsToRegistry(const std::_tstring& a_key,
	const CNFORenderSettings& a_settings)
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
		dwLinkColor = a_settings.cHyperlinkColor.AsWord(),
		dwGaussColor = a_settings.cGaussColor.AsWord();

	RegSetValueEx(l_hKey, _T("ClrText"),	0, REG_DWORD, (LPBYTE)&dwTextColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrBack"),	0, REG_DWORD, (LPBYTE)&dwBackColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrArt"),		0, REG_DWORD, (LPBYTE)&dwArtColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrLink"),	0, REG_DWORD, (LPBYTE)&dwLinkColor,		sizeof(uint32_t));
	RegSetValueEx(l_hKey, _T("ClrGauss"),	0, REG_DWORD, (LPBYTE)&dwGaussColor,	sizeof(uint32_t));

	int32_t dwGaussShadow = (a_settings.bGaussShadow ? 1 : 0),
		dwHilightHyperlinks = (a_settings.bHilightHyperlinks ? 1 : 0),
		dwUnderlineHyperlinks = (a_settings.bUnderlineHyperlinks ? 1 : 0),
		dwBlockHeight = a_settings.uBlockHeight,
		dwBlockWidth = a_settings.uBlockWidth,
		dwGaussBlurRadius = a_settings.uGaussBlurRadius;

	RegSetValueEx(l_hKey, _T("GaussShadow"),			0, REG_DWORD, (LPBYTE)&dwGaussShadow,			sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("HilightHyperlinks"),		0, REG_DWORD, (LPBYTE)&dwHilightHyperlinks,		sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("UnderlineHyperlinks"),	0, REG_DWORD, (LPBYTE)&dwUnderlineHyperlinks,	sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("BlockHeight"),			0, REG_DWORD, (LPBYTE)&dwBlockHeight,			sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("BlockWidth"),				0, REG_DWORD, (LPBYTE)&dwBlockWidth,			sizeof(int32_t));
	RegSetValueEx(l_hKey, _T("GaussBlurRadius"),		0, REG_DWORD, (LPBYTE)&dwGaussBlurRadius,		sizeof(int32_t));

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

	uint32_t dwTextColor, dwBackColor, dwArtColor, dwLinkColor, dwGaussColor;

	DWORD l_dwType = REG_DWORD;
	DWORD l_dwBufSz = sizeof(int32_t);

	RegQueryValueEx(l_hKey, _T("ClrText"),	NULL, &l_dwType,	(LPBYTE)&dwTextColor,	&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("ClrBack"),	NULL, &l_dwType,	(LPBYTE)&dwBackColor,	&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("ClrArt"),	NULL, &l_dwType,	(LPBYTE)&dwArtColor,	&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("ClrLink"),	NULL, &l_dwType,	(LPBYTE)&dwLinkColor,	&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("ClrGauss"),	NULL, &l_dwType,	(LPBYTE)&dwGaussColor,	&l_dwBufSz);

	int32_t dwGaussShadow, dwHilightHyperlinks, dwUnderlineHyperlinks, dwBlockHeight, dwBlockWidth, dwGaussBlurRadius;

	RegQueryValueEx(l_hKey, _T("GaussShadow"),			NULL, &l_dwType, (LPBYTE)&dwGaussShadow,			&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("HilightHyperlinks"),	NULL, &l_dwType, (LPBYTE)&dwHilightHyperlinks,		&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("UnderlineHyperlinks"),	NULL, &l_dwType, (LPBYTE)&dwUnderlineHyperlinks,	&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("BlockHeight"),			NULL, &l_dwType, (LPBYTE)&dwBlockHeight,			&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("BlockWidth"),			NULL, &l_dwType, (LPBYTE)&dwBlockWidth,				&l_dwBufSz);
	RegQueryValueEx(l_hKey, _T("GaussBlurRadius"),		NULL, &l_dwType, (LPBYTE)&dwGaussBlurRadius,		&l_dwBufSz);

	RegCloseKey(l_hKey);

	CNFORenderSettings l_newSets;

	l_newSets.cTextColor = _s_color_t(dwTextColor);
	l_newSets.cBackColor = _s_color_t(dwBackColor);
	l_newSets.cArtColor = _s_color_t(dwArtColor);
	l_newSets.cHyperlinkColor = _s_color_t(dwLinkColor);
	l_newSets.cGaussColor = _s_color_t(dwGaussColor);

	l_newSets.bGaussShadow = (dwGaussShadow != 0);
	l_newSets.bHilightHyperlinks = (dwHilightHyperlinks != 0);
	l_newSets.bUnderlineHyperlinks = (dwUnderlineHyperlinks != 0);
	l_newSets.uBlockHeight = dwBlockHeight;
	l_newSets.uBlockWidth = dwBlockWidth;
	l_newSets.uGaussBlurRadius = dwGaussBlurRadius;

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

		const _tstring l_msg = _T("Attention! There is a new version (iNFEKT v") + l_serverVersion + _T(") available!") +
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


CMainFrame::~CMainFrame()
{

}