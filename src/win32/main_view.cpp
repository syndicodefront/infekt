#include "stdafx.h"
#include "app.h"
#include "nfo_view_ctrl.h"


CViewContainer::CViewContainer()
{

}


void CViewContainer::OnCreate()
{
	m_renderControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd()));
	// :TODO: remove this / use settings instead...
	m_renderControl->SetGaussColor(S_COLOR_T(0xBF, 0x17, 0x17));
	m_renderControl->SetArtColor(S_COLOR_T(0xBF, 0x17, 0x17));
	m_renderControl->CreateControl(0, 0, 100, 100);
}


bool CViewContainer::OpenFile(const std::wstring& a_filePath)
{
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	if(m_nfoData)
	{
		// reset so we don't run into bugs :P :P
		m_nfoData.reset();
	}

	m_nfoData = PNFOData(new CNFOData());

	if(m_nfoData->LoadFromFile(a_filePath))
	{
		if(m_renderControl->AssignNFO(m_nfoData))
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			return true;
		}
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));

	return false;
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
	// return false if the message is not supposed to bubble up.

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
