#include "stdafx.h"

namespace DuiLib {

COptionDelayUI::COptionDelayUI(void)
{
}

COptionDelayUI::~COptionDelayUI(void)
{
}


LPCTSTR COptionDelayUI::GetClass() const
{
	return _T("OptionDelay");
}

LPVOID COptionDelayUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("OptionDelay")) == 0 ) return static_cast<COptionDelayUI*>(this);
	return COptionUI::GetInterface(pstrName);
}

UINT COptionDelayUI::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

bool COptionDelayUI::Activate()
{
	if( !CButtonUI::Activate() ) return false;
	if( !m_sGroupName.IsEmpty() ) Selected(true);
	else Selected(!m_bSelected);

	return true;
}

void COptionDelayUI::Selected(bool bSelected)
{
	if( m_bSelected == bSelected ) return;
	m_bSelected = bSelected;
	if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
	else m_uButtonState &= ~UISTATE_SELECTED;

	if (bSelected == false)
	{
		m_pManager->RemovePostPaint(this);
	}
	else
		m_pManager->AddPostPaint(this);


	if( m_pManager != NULL ) {
		if( !m_sGroupName.IsEmpty() ) {
			if( m_bSelected ) {
				CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
				for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
					COptionDelayUI* pControl = static_cast<COptionDelayUI*>(aOptionGroup->GetAt(i));
					if( pControl != this ) {
						pControl->Selected(false);
					}
				}
				m_pManager->SendNotify(this, _T("selectchanged"), DUILIB_OP_SELECTCHANGED, m_bSelected);
			}
		}
		else {
			m_pManager->SendNotify(this, _T("selectchanged"), DUILIB_OP_SELECTCHANGED, m_bSelected);
		}
	}
	
	Invalidate();

}

void COptionDelayUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
	PaintStatusImage(hDC);
	PaintText(hDC);
}

}