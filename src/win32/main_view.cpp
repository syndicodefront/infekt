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
	m_renderControl->AssignNFO(n);
	m_renderControl->Render();
	m_renderControl->CreateControl(0, 0, 450, 500);
}


CViewContainer::~CViewContainer()
{

}
