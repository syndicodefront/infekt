#include "stdafx.h"
#include "app.h"
#include "main_frame.h"
#include "settings_dlg.h"
#include "resource.h"

using namespace std;


enum _toolbar_button_ids {
	TBBID_OPEN = 30000,
	TBBID_SETTINGS,
	TBBID_EDITCOPY,
	TBBID_ABOUT
};


CMainFrame::CMainFrame() : CFrame()
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
}


void CMainFrame::OnInitialUpdate()
{
	CenterWindow();
	UpdateCaption();

	ShowWindow();
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
		_TBBTN(ICO_SETTINGS, TBBID_SETTINGS, defState, defStyle, "Settings"),
		_TBSEP,
		_TBBTN(ICO_EDITCOPY, TBBID_EDITCOPY, 0, defStyle, "Copy"),
		_TBSEP,
		_TBBTN(ICO_INFO, TBBID_ABOUT, defState, defStyle, "About"),
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

	case TBBID_SETTINGS: {
		CSettingsWindowDialog l_dlg(IDD_DLG_SETTINGS, m_hWnd);
		l_dlg.DoModal();
		return TRUE; }

	case TBBID_ABOUT:
	case IDM_ABOUT:
		this->MessageBox(_T("Rebecca, you are the love of my life."), _T("About"), MB_ICONINFORMATION);
		return TRUE;
	}

	return FALSE;
}


void CMainFrame::UpdateCaption()
{
	wstring l_caption;

	if(m_view.GetNfoData() && m_view.GetNfoData()->HasData())
	{
		l_caption = m_view.GetNfoData()->GetFileName();
	}

	if(!l_caption.empty()) l_caption += _T(" - ");
	l_caption += FORMAT(_T("iNFEKT v%d.%d.%d"), INFEKT_VERSION_MAJOR % INFEKT_VERSION_MINOR % INFEKT_VERSION_REVISION);

	SetWindowText(l_caption.c_str());
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
	case WM_KEYDOWN:
		if(!m_view.ForwardFocusTypeMouseKeyboardEvent(pMsg))
		{
			return FALSE;
		}
		break;
	case WM_SYSKEYUP:
		if(pMsg->wParam == VK_MENU)
		{
			GetRebar().ShowBand(GetRebar().IDToIndex(IDW_MENUBAR), (m_menuBarVisible ? FALSE : TRUE));
			m_menuBarVisible = !m_menuBarVisible;
		}
		break;
	}

	return TRUE;
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
	OPENFILENAME ofn = {0};
	TCHAR wszBuf[1000] = {0};

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInstance;
	ofn.hwndOwner = GetHwnd();
	ofn.lpstrFilter = _T("NFO Files\0*.nfo;*.diz\0Text Files\0*.txt;*.nfo;*.diz;*.sfv\0All Files\0*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = wszBuf;
	ofn.nMaxFile = 999;
	ofn.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	if(GetOpenFileName(&ofn))
	{
		m_view.OpenFile(ofn.lpstrFile);

		UpdateCaption();
	}
}


CMainFrame::~CMainFrame()
{

}
