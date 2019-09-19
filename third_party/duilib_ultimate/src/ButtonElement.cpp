#include "stdafx.h"
#include "ButtonElement.h"

namespace DuiLib {

CButtonElement::CButtonElement() : m_uButtonState(0)

{
	m_pEvent = NULL;
	m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
}

LPCTSTR CButtonElement::GetClass() const
{
	return _T("ButtonElementUI");
}

LPVOID CButtonElement::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ButtonElementUI")) == 0 ) return static_cast<CButtonElement*>(this);
	return CLabelUI::GetInterface(pstrName);
}

UINT CButtonElement::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	//    return UIFLAG_TABSTOP | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
}

bool CButtonElement::Activate()
{
	if( !CControlUI::Activate() )
		return false;

	if( m_pManager != NULL )
		m_pManager->SendNotify(this, _T("click"), DUILIB_BN_CLICKED);
	return true;
}


void CButtonElement::DoEvent(TEventUI& event)
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
		return m_pParent->DoEvent(event);;
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
			if (m_bLinkStype && (m_iHoverFont != -1))
			{
				SetFont(m_iHoverFont);
			}
			Invalidate();
		}
		return m_pParent->DoEvent(event);
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( IsEnabled() ) {
			m_uButtonState &= ~UISTATE_HOT;
			if (m_bLinkStype && (m_iNormalFont != -1))
			{
				SetFont(m_iNormalFont);
			}
			Invalidate();
		}
		return m_pParent->DoEvent(event);
	}
	CButtonUI::DoEvent(event);
}

void CButtonElement::PaintText(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
	if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

	if( m_sText.IsEmpty() ) return;
	int nLinks = 0;
	RECT rc = m_rcItem;
	rc.left += m_rcTextPadding.left;
	rc.right -= m_rcTextPadding.right;
	rc.top += m_rcTextPadding.top;
	rc.bottom -= m_rcTextPadding.bottom;

	DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

	if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
		clrColor = GetPushedTextColor();
	else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
		clrColor = GetHotTextColor();
	else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
		clrColor = GetFocusedTextColor();

	if( m_bShowHtml )
		CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
		NULL, NULL, nLinks, m_uTextStyle);
	else
		CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, clrColor, \
		m_iFont, m_uTextStyle);
}

void CButtonElement::PaintStatusImage(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
		if( !m_sDisabledImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
		if( !m_sPushedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ) m_sPushedImage.Empty();
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0) {
		if( !m_sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
			else return;
		}
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
		if( !m_sFocusedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
			else return;
		}
	}

	if( !m_sNormalImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
		else return;
	}
}


}
