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

#ifndef _MAIN_VIEW_H
#define _MAIN_VIEW_H

#include "nfo_view_ctrl.h"

typedef enum _main_view_view_t
{
	MAIN_VIEW_RENDERED = 1,
	MAIN_VIEW_CLASSIC,
	MAIN_VIEW_TEXTONLY,

	_MAIN_VIEW_MAX
} EMainView;


class CViewContainer : public CWnd
{
public:
	CViewContainer();
	virtual ~CViewContainer();

	bool ForwardFocusTypeMouseKeyboardEvent(const MSG* pMsg);
	PNFOViewControl& GetRenderCtrl() { return m_renderControl; }
	PNFOViewControl& GetClassicCtrl() { return m_classicControl; }
	PNFOViewControl& GetTextOnlyCtrl() { return m_textOnlyControl; }
	PNFOData& GetNfoData() { return m_nfoData; }

	bool OpenFile(const std::wstring& a_filePath);
	void SwitchView(EMainView a_view);

	const std::wstring GetSelectedText() const;
	void CopySelectedTextToClipboard() const;
	void SelectAll();
protected:
	PNFOViewControl m_renderControl;
	PNFOViewControl m_classicControl;
	PNFOViewControl m_textOnlyControl;

	EMainView m_curViewType;
	PNFOViewControl m_curViewCtrl;
	PNFOData m_nfoData;

	HMENU m_contextMenuHandle;

	virtual void OnCreate();
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnAfterResize();
};

#endif /* !_MAIN_VIEW_H */
