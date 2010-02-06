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


void CViewContainer::ForwardFocusTypeMouseKeyboardEvent(const MSG* pMsg)
{
	// if type == ...
	PostMessage(m_renderControl->GetHwnd(), pMsg->message, pMsg->wParam, pMsg->lParam);
}


CViewContainer::~CViewContainer()
{

}
