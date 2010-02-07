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
