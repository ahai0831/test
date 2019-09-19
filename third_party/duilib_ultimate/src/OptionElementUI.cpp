#include "StdAfx.h"
#include "OptionElementUI.h"

namespace DuiLib{
COptionElementUI::COptionElementUI(void)
{
	m_pEvent = NULL;
	m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
}

LPCTSTR COptionElementUI::GetClass() const
{
	return _T("OptionElementUI");
}

LPVOID COptionElementUI::GetInterface( LPCTSTR pstrName )
{
	if( _tcscmp(pstrName, _T("OptionElementUI")) == 0 ) return static_cast<COptionElementUI*>(this);
	return CLabelUI::GetInterface(pstrName);
}

UINT COptionElementUI::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

bool COptionElementUI::Activate()
{
	if( !CButtonUI::Activate() ) return false;
	if( !m_sGroupName.IsEmpty() ) Selected(true);
	else Selected(!m_bSelected);

	return true;
}

void COptionElementUI::DoEvent( TEventUI& event )
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) 
	{
		if( m_pParent != NULL ) 
			m_pParent->DoEvent(event);
		else
			CButtonUI::DoEvent(event);
		return;
	}

	if (m_pParent == NULL)
	{
		return;
	}

	if( event.Type == UIEVENT_SETFOCUS ) 
	{
		m_bFocused = true;
		Invalidate();
		return;
	}
	if( event.Type == UIEVENT_KILLFOCUS ) 
	{
		m_bFocused = false;
		Invalidate();
		return;
	}
	if( event.Type == UIEVENT_KEYDOWN )
	{
		if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
			Activate();
			return m_pParent->DoEvent(event);
		}
	}
	if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
	{
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
			m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			m_pEvent = &event;
			Invalidate();
		}
		event.wParam = UISTATE_SELECTED;

		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
			else m_uButtonState &= ~UISTATE_PUSHED;
			Invalidate();
		}
		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_BUTTONUP )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) )
				Activate();
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}
		event.wParam = UISTATE_SELECTED;
		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_CONTEXTMENU )
	{
		if( IsContextMenuUsed() ) {
			m_pManager->SendNotify(this, _T("menu"), DUILIB_MENU_CLICK, event.wParam, event.lParam);
		}
		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_SETCURSOR ) {
		if (IsEnabled()) {
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
			return;
		}
		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( IsEnabled() ) {
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( IsEnabled() ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return m_pParent->DoEvent(event);
	}
	CButtonUI::DoEvent(event);
}

void COptionElementUI::PaintText( HDC hDC )
{
	if( (m_uButtonState & UISTATE_SELECTED) != 0 )
	{
		DWORD oldTextColor = m_dwTextColor;
		if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			NULL, NULL, nLinks, m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			m_iFont, m_uTextStyle);

		m_dwTextColor = oldTextColor;
	}
	else
		CButtonUI::PaintText(hDC);
}

void COptionElementUI::PaintStatusImage( HDC hDC )
{
	m_uButtonState &= ~UISTATE_PUSHED;

	if( (m_uButtonState & UISTATE_SELECTED) != 0 ) {
		if( !m_sSelectedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sSelectedImage) ) m_sSelectedImage.Empty();
			else goto Label_ForeImage;
		}
	}

	CButtonUI::PaintStatusImage(hDC);

Label_ForeImage:
	if( !m_sForeImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
	}
}

}