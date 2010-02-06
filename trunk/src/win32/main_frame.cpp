#include "stdafx.h"
#include "main_frame.h"


CMainFrame::CMainFrame()
{
	SetView(m_view);
}


void CMainFrame::OnInitialUpdate()
{
	SetWindowText(_T("iNFEKT NFO Viewer"));
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


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_MOUSEHWHEEL)
	{
		m_view.ForwardFocusTypeMouseKeyboardEvent(pMsg);
		return FALSE;
	}

	return TRUE;
}


CMainFrame::~CMainFrame()
{

}
