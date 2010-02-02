#include "stdafx.h"
#include "main_frame.h"


CMainFrame::CMainFrame()
{
	SetView(m_view);
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


CMainFrame::~CMainFrame()
{

}
