#include "stdafx.h"
#include "app.h"
#include "nfo_view_ctrl.h"
#include "resource.h"


CViewContainer::CViewContainer()
{
	m_contextMenuHandle = NULL;
	m_resized = true;
	m_curViewType = _MAIN_VIEW_MAX;
}


void CViewContainer::OnCreate()
{
	m_renderControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd()));
	m_classicControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd(), true));
	m_textOnlyControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd(), true));

	// this context menu will be used for all three view controls.
	m_contextMenuHandle = ::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT_MENU));
}


bool CViewContainer::OpenFile(const std::wstring& a_filePath)
{
	::SetCursor(::LoadCursor(NULL, IDC_WAIT));

	if(m_nfoData)
	{
		// reset so we don't run into bugs :P :P
		m_nfoData.reset();
	}

	m_nfoData = PNFOData(new CNFOData());

	if(m_nfoData->LoadFromFile(a_filePath))
	{
		if(m_curViewType != MAIN_VIEW_RENDERED) m_renderControl->UnAssignNFO();
		if(m_curViewType != MAIN_VIEW_CLASSIC) m_classicControl->UnAssignNFO();
		if(m_curViewType != MAIN_VIEW_TEXTONLY) m_textOnlyControl->UnAssignNFO();

		if(CurAssignNfo())
		{
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));

			return true;
		}
	}
	else
	{
		this->MessageBox(m_nfoData->GetLastErrorDescription().c_str(), _T("Fail"), MB_ICONEXCLAMATION);
		// :TODO: better error messages blah blah blah
	}

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	return false;
}


LRESULT CViewContainer::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_SIZE:
		OnAfterResize(false);
		m_resized = true;
		break;
	case WM_ERASEBKGND:
		// intercept message to avoid flicker during resize
		return 1;
	}

	return WndProcDefault(uMsg, wParam, lParam);
}


void CViewContainer::OnAfterResize(bool a_fake)
{
	if(m_curViewCtrl)
	{
		RECT l_viewArea = GetClientRect();

		::MoveWindow(m_curViewCtrl->GetHwnd(), 0, 0, l_viewArea.right - l_viewArea.left,
			l_viewArea.bottom - l_viewArea.top, TRUE);
	}
}


bool CViewContainer::ForwardFocusTypeMouseKeyboardEvent(const MSG* pMsg)
{
	// return false if the message is not supposed to bubble up.

	HWND hScrollTarget = m_curViewCtrl->GetHwnd();

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


void CViewContainer::SwitchView(EMainView a_view)
{
	if(m_curViewType == a_view)
	{
		return;
	}

	m_curViewType = a_view;

	SendMessage(WM_SETREDRAW, 0);

	switch(a_view)
	{
	case MAIN_VIEW_RENDERED: m_curViewCtrl = m_renderControl; break;
	case MAIN_VIEW_CLASSIC: m_curViewCtrl = m_classicControl; break;
	case MAIN_VIEW_TEXTONLY: m_curViewCtrl = m_textOnlyControl; break;
	}

	if(!m_curViewCtrl->ControlCreated())
	{
		RECT l_viewArea = GetClientRect();
		m_curViewCtrl->CreateControl(0, 0,
			l_viewArea.right - l_viewArea.left, l_viewArea.bottom - l_viewArea.top);
		m_curViewCtrl->SetContextMenu(m_contextMenuHandle, GetParent());
	}

	if(m_nfoData && !m_curViewCtrl->HasNfoData())
	{
		CurAssignNfo();
	}

	if(m_resized)
	{
		OnAfterResize(true);
	}

	m_renderControl->Show(a_view == MAIN_VIEW_RENDERED);
	m_classicControl->Show(a_view == MAIN_VIEW_CLASSIC);
	m_textOnlyControl->Show(a_view == MAIN_VIEW_TEXTONLY);

	m_resized = false;

	SendMessage(WM_SETREDRAW, 1);
	::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}


bool CViewContainer::CurAssignNfo()
{
	if(m_curViewType != MAIN_VIEW_TEXTONLY)
	{
		return m_curViewCtrl->AssignNFO(m_nfoData);
	}
	else
	{
		const std::string l_stripped = CNFOData::GetStrippedTextUtf8(m_nfoData->GetTextWide());
		PNFOData l_data(new CNFOData());
		l_data->SetCharsetToTry(NFOC_UTF8);
		if(l_data->LoadFromMemory((const unsigned char*)l_stripped.c_str(), l_stripped.size()))
		{
			return m_curViewCtrl->AssignNFO(l_data);
		}
	}

	return false;
}


const std::wstring CViewContainer::GetSelectedText() const
{
	return (m_curViewCtrl ? L"" : m_curViewCtrl->GetSelectedText());
}


void CViewContainer::CopySelectedTextToClipboard() const
{
	if(m_curViewCtrl)
	{
		m_curViewCtrl->CopySelectedTextToClipboard();
	}
}


void CViewContainer::SelectAll()
{
	if(m_curViewCtrl)
	{
		m_curViewCtrl->SelectAll();
	}
}


CViewContainer::~CViewContainer()
{
	if(m_contextMenuHandle)
	{
		::DestroyMenu(m_contextMenuHandle);
	}
}
