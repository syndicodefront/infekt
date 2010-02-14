#include "stdafx.h"
#include "main_frame.h"
#include "app.h"
#include "resource.h"

using namespace std;


CMainFrame::CMainFrame() : CFrame()
{
	SetView(m_view);
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
	UpdateCaption();
}


void CMainFrame::SetupToolbar()
{
	// turn off Win32++ "theme":
	CRebar& RB = GetRebar();
	ThemeRebar RBTheme = RB.GetRebarTheme();
	RBTheme.UseThemes = FALSE;
	RB.SetRebarTheme(RBTheme);

	// add menu to rebar:
	GetMenubar().SetMenu(::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAIN_MENU)));

	// adjust some stupid rebar stuff, argh:
	REBARBANDINFO l_rbbi;
	l_rbbi.fMask = RBBIM_STYLE;
	GetRebar().GetBandInfo(0, l_rbbi);
	l_rbbi.fStyle |= RBBS_TOPALIGN | RBBS_BREAK | RBBS_NOGRIPPER;
	GetRebar().SetBandInfo(0, l_rbbi);

	GetRebar().GetBandInfo(1, l_rbbi);
	l_rbbi.fStyle |= RBBS_BREAK | RBBS_BREAK | RBBS_NOGRIPPER;
	GetRebar().SetBandInfo(1, l_rbbi);

	// hide menubar by default:
	GetRebar().ShowBand(0, FALSE);
	m_menuShowing = false;

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
		_TBBTN(ICO_FILEOPEN, 123, defState, defStyle | BTNS_DROPDOWN, "Open (Ctrl+O)"),
		_TBSEP,
		_TBBTN(ICO_SETTINGS, 123, defState, defStyle, "Settings"),
		_TBSEP,
		_TBBTN(ICO_EDITCOPY, 123, 0, defStyle, "Copy"),
		_TBSEP,
		_TBBTN(ICO_INFO, 123, defState, defStyle, "About"),
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
	}

	return FALSE;
}


void CMainFrame::UpdateCaption()
{
	wstring l_caption;

	//if(NFO loaded ... add file name ... :TODO:

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
			GetRebar().ShowBand(0, (m_menuShowing ? FALSE : TRUE));
			m_menuShowing = !m_menuShowing;
		}
		break;
	}

	return TRUE;
}


LRESULT CMainFrame::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CLOSE: // :TODO: why is Alt+F4 not sending this?!
		DestroyWindow(GetHwnd());
		return 0;
	}

	return WndProcDefault(uMsg, wParam, lParam);
}


CMainFrame::~CMainFrame()
{

}
