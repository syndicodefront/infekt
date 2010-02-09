#include "stdafx.h"
#include "main_frame.h"

using namespace std;


CMainFrame::CMainFrame()
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

	// add buttons...
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
