#include "stdafx.h"
#include "VerticalLayoutExUI.h"
namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//
//

CVerticalLayoutExUI::CVerticalLayoutExUI() : CVerticalLayoutUI()
{
}

LPCTSTR CVerticalLayoutExUI::GetClass() const
{
	return _T("VerticalLayoutExUI");
}

LPVOID CVerticalLayoutExUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("VerticalLayoutEx")) == 0 ) return static_cast<CVerticalLayoutExUI*>(this);
	return CVerticalLayoutUI::GetInterface(pstrName);
}

UINT CVerticalLayoutExUI::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CVerticalLayoutExUI::DoEvent(TEventUI& event)
{
	if (m_iSepHeight == 0)
	{
		if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() && ::PtInRect(&m_rcItem, event.ptMouse))
		{
			if (m_pManager)
			{
				m_pManager->SendNotify(this, _T("verticallayoutclick"), DUILIB_VERTICAL_LAYOUT_CLICK, 0, 0, true);
			}
		}
	}
	
	return CVerticalLayoutUI::DoEvent(event);
}
}