/**
 * Copyright (C) 2010 cxxjoe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#include "stdafx.h"
#include "app.h"
#include "nfo_view_ctrl.h"
#include "resource.h"
#include "plugin_manager.h"

#define INFOBAR_MINIMUM_HEIGHT 50


CViewContainer::CViewContainer()
{
	m_contextMenuHandle = NULL;
	m_resized = true;
	m_curViewType = _MAIN_VIEW_MAX;
	m_showInfoBar = false;
	m_cursor = IDC_ARROW;

	m_infoBarHeight = 200;
	m_infoBarResizing = false;
}


void CViewContainer::OnCreate()
{
	m_renderControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd()));
	m_classicControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd(), true));
	m_textOnlyControl = PNFOViewControl(new CNFOViewControl(g_hInstance, GetHwnd(), true));
	m_textOnlyControl->SetCenterNfo(false);

	// this context menu will be used for all three view controls.
	m_contextMenuHandle = ::LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT_MENU));

	m_infoBar = boost::shared_ptr<CInfektInfoBar>(new CInfektInfoBar(g_hInstance, GetHwnd()));
	m_infoBar->CreateControl(0, 0, 100, m_infoBarHeight);
	if(!m_showInfoBar) ::ShowWindow(m_infoBar->GetHwnd(), SW_HIDE);
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

	CPluginManager::GetInstance()->TriggerNfoLoad(true, a_filePath.c_str());

	if(m_nfoData->LoadFromFile(a_filePath))
	{
		if(m_curViewType != MAIN_VIEW_RENDERED) m_renderControl->UnAssignNFO();
		if(m_curViewType != MAIN_VIEW_CLASSIC) m_classicControl->UnAssignNFO();
		if(m_curViewType != MAIN_VIEW_TEXTONLY) m_textOnlyControl->UnAssignNFO();

		CPluginManager::GetInstance()->TriggerNfoLoad(false, a_filePath.c_str());

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


bool CViewContainer::OpenLoadedFile(const std::wstring& a_filePath, PNFOData a_nfoData)
{
	::SetCursor(::LoadCursor(NULL, IDC_WAIT));

	if(m_nfoData)
	{
		m_nfoData.reset();
	}

	m_nfoData = a_nfoData;

	CPluginManager::GetInstance()->TriggerNfoLoad(true, a_filePath.c_str());

	if(m_curViewType != MAIN_VIEW_RENDERED) m_renderControl->UnAssignNFO();
	if(m_curViewType != MAIN_VIEW_CLASSIC) m_classicControl->UnAssignNFO();
	if(m_curViewType != MAIN_VIEW_TEXTONLY) m_textOnlyControl->UnAssignNFO();

	CPluginManager::GetInstance()->TriggerNfoLoad(false, a_filePath.c_str());

	bool b = CurAssignNfo();

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	return b;
}


bool CViewContainer::ReloadFile()
{
	if(m_nfoData)
	{
		if(OpenFile(m_nfoData->GetFilePath()))
		{
			// redraw scrollbars and such:
			SendMessage(WM_SETREDRAW, 1);
			::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

			return true;
		}
	}

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
	case WM_PAINT:
		if(m_showInfoBar)
		{
			// we need to paint the splitter manually because
			// we intercept WM_ERASEBKGND.
			RECT l_viewArea = GetClientRect();
			int l_nfoAreaHeight = l_viewArea.bottom - l_viewArea.top - m_infoBarHeight;

			PAINTSTRUCT l_ps;
			HDC l_dc = ::BeginPaint(GetHwnd(), &l_ps);

			RECT l_rect = { 0, l_nfoAreaHeight - ::GetSystemMetrics(SM_CYSIZEFRAME),
				l_viewArea.right - l_viewArea.left, l_nfoAreaHeight };
			::FillRect(l_dc, &l_rect, ::GetSysColorBrush(COLOR_BTNFACE));

			::EndPaint(GetHwnd(), &l_ps);

			return 0;
		}
		else break;
	case WM_LBUTTONDOWN:
		if(IsYOnSplitter(GET_Y_LPARAM(lParam)))
		{
			m_infoBarResizing = true;
			::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
			SetCapture();
		}
		break;
	case WM_LBUTTONUP:
		if(IsYOnSplitter(GET_Y_LPARAM(lParam)))
		{
			m_infoBarResizing = false;
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		if(m_infoBarResizing)
		{
			RECT l_client = GetClientRect();
			int l_infoBarHeight = (l_client.bottom - l_client.top -
				::GetSystemMetrics(SM_CYSIZEFRAME) / 2) - GET_Y_LPARAM(lParam);

			if(l_infoBarHeight > INFOBAR_MINIMUM_HEIGHT && l_infoBarHeight < l_client.bottom - l_client.top - INFOBAR_MINIMUM_HEIGHT)
			{
				m_infoBarHeight = l_infoBarHeight;
				OnAfterResize(false);
			}
		}
		else if(IsYOnSplitter(GET_Y_LPARAM(lParam)))
		{
			m_cursor = IDC_SIZENS;
		}
		else
		{
			m_cursor = IDC_ARROW;
		}
		break;
	case WM_CAPTURECHANGED:
		m_infoBarResizing = false;
		break;
	case WM_SETCURSOR:
		return CUtil::GenericOnSetCursor(m_cursor, lParam);
	}

	return WndProcDefault(uMsg, wParam, lParam);
}


void CViewContainer::OnAfterResize(bool a_fake)
{
	if(m_curViewCtrl)
	{
		RECT l_viewArea = GetClientRect();
		int l_viewAreaHeight = l_viewArea.bottom - l_viewArea.top;
		int l_splitHeight = ::GetSystemMetrics(SM_CYSIZEFRAME);

		::MoveWindow(m_curViewCtrl->GetHwnd(), 0, 0, l_viewArea.right - l_viewArea.left,
			l_viewAreaHeight - (m_showInfoBar ? m_infoBarHeight + l_splitHeight : 0), TRUE);

		if(m_showInfoBar)
		{
			::MoveWindow(m_infoBar->GetHwnd(), 0, l_viewAreaHeight - m_infoBarHeight,
				l_viewArea.right - l_viewArea.left, m_infoBarHeight, TRUE);
		}
	}
}


bool CViewContainer::IsYOnSplitter(int a_y)
{
	if(!m_showInfoBar)
	{
		return false;
	}

	RECT l_viewArea = GetClientRect();
	int l_nfoAreaHeight = l_viewArea.bottom - l_viewArea.top - m_infoBarHeight;
	int l_splitHeight = ::GetSystemMetrics(SM_CYSIZEFRAME);

	return (a_y >= l_nfoAreaHeight - l_splitHeight && a_y <= l_nfoAreaHeight);
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
	bool l_force = false;

	if(a_view == MAIN_VIEW_TEXTONLY && m_textOnlyControl && m_textOnlyControl->HasNfoData())
	{
		if(m_textOnlyControl->GetNfoData()->GetWrapLines() != m_textOnlyControl->GetWrapLines())
		{
			m_textOnlyControl->UnAssignNFO();
			l_force = true;
		}
	}

	if(a_view >= _MAIN_VIEW_MAX || (m_curViewType == a_view && !l_force))
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
		::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		CurAssignNfo();
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	if(m_resized)
	{
		OnAfterResize(true);
	}

	m_renderControl->Show(a_view == MAIN_VIEW_RENDERED);
	m_classicControl->Show(a_view == MAIN_VIEW_CLASSIC);
	m_textOnlyControl->Show(a_view == MAIN_VIEW_TEXTONLY);

#if 0
	// we have three controls so we can't reset this here, or
	// 1. load NFO
	// 2. switch to 3rd view
	// 3. switch to 2nd view
	// 4. switch to first view
	// 5. resize window
	// 6. switch to 2nd view
	// 7. switch to 3rd view
	// would cause a wrong control size of the 3rd view control.
	m_resized = false;
#endif

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
		l_data->SetWrapLines(m_curViewCtrl->GetWrapLines());
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
