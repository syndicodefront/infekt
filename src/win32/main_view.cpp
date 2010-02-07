#include "stdafx.h"
#include "app.h"
#include "nfo_view_ctrl.h"


CViewContainer::CViewContainer()
{

}

PNFOData n;

void CViewContainer::OnCreate()
{
	n = PNFOData(new CNFOData());
	n->LoadFromFile(L"C:\\temp\\utf8-2.nfo");

	m_renderControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd()));
	m_renderControl->SetGaussColor(S_COLOR_T(0xBF, 0x17, 0x17));
	m_renderControl->SetArtColor(S_COLOR_T(0xBF, 0x17, 0x17));
	m_renderControl->AssignNFO(n);
	m_renderControl->Render();
	m_renderControl->CreateControl(0, 0, 450, 500);
}


LRESULT CViewContainer::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_SIZE:
		OnAfterResize();
		break;
	}

	return WndProcDefault(uMsg, wParam, lParam);
}


void CViewContainer::OnAfterResize()
{
	RECT l_viewArea = GetClientRect();

	::MoveWindow(m_renderControl->GetHwnd(), 0, 0, l_viewArea.right - l_viewArea.left,
		l_viewArea.bottom - l_viewArea.top, TRUE);
}


bool CViewContainer::ForwardFocusTypeMouseKeyboardEvent(const MSG* pMsg)
{
	HWND hScrollTarget = 0;
	// if type == ...
	// ...
	if(m_renderControl) hScrollTarget = m_renderControl->GetHwnd();

	if(!hScrollTarget)
	{
		return true;
	}

	if(pMsg->message == WM_KEYDOWN)
	{
		WORD wScrollNotify = (WORD)-1;
		UINT uMessage = WM_VSCROLL;

		switch(pMsg->wParam)
		{
		case VK_UP: wScrollNotify = SB_LINEUP; break;
		case VK_PRIOR: wScrollNotify = SB_PAGEUP; break;
		case VK_NEXT: wScrollNotify = SB_PAGEDOWN; break;
		case VK_DOWN: wScrollNotify = SB_LINEDOWN; break;
		case VK_HOME: wScrollNotify = SB_TOP; break;
		case VK_END: wScrollNotify = SB_BOTTOM; break;
		default:
			uMessage = WM_HSCROLL;
			switch(pMsg->wParam)
			{
			case VK_LEFT: wScrollNotify = SB_LINELEFT; break;
			case VK_RIGHT: wScrollNotify = SB_LINERIGHT; break;
			}
		}

		if(wScrollNotify != (WORD)-1)
		{
			::PostMessage(hScrollTarget, uMessage, MAKELONG(wScrollNotify, 0), 0L);

			return false;
		}

		return true;
	}
	else
	{
		::PostMessage(hScrollTarget, pMsg->message, pMsg->wParam, pMsg->lParam);

		return false;
	}
}


CViewContainer::~CViewContainer()
{

}
