#include "stdafx.h"
#include "UIButtonElementEx.h"
namespace DuiLib
{


	CButtonElementEx::CButtonElementEx(void)
	{

	}

	LPCTSTR CButtonElementEx::GetClass() const
	{
		return _T("ButtonElementExUI");
	}

	LPVOID CButtonElementEx::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("ButtonElementEx")) == 0) return static_cast<CButtonElementEx*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CButtonElementEx::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			if (m_pParent != NULL)
				m_pParent->DoEvent(event);
			else
				CButtonUI::DoEvent(event);
			return;
		}

		if (m_pParent == NULL)
		{
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			m_bFocused = true;
			Invalidate();
			return;
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			m_bFocused = false;
			Invalidate();
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN)
		{
			if (event.chKey == VK_SPACE || event.chKey == VK_RETURN) {
				Activate();
				return m_pParent->DoEvent(event);
			}
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if (::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled()) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				//Activate();
				m_pEvent = &event;
				Invalidate();
			}
			event.wParam = UISTATE_SELECTED;

			return m_pParent->DoEvent(event);
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				if (::PtInRect(&m_rcItem, event.ptMouse)) m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return m_pParent->DoEvent(event);
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				if (::PtInRect(&m_rcItem, event.ptMouse))
					Activate();
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();
			}
			event.wParam = UISTATE_SELECTED;
			return m_pParent->DoEvent(event);;
		}
		if (event.Type == UIEVENT_CONTEXTMENU)
		{
			if (IsContextMenuUsed()) {
				m_pManager->SendNotify(this, _T("menu"), DUILIB_MENU_CLICK, event.wParam, event.lParam);
			}
			return m_pParent->DoEvent(event);
		}
		if (event.Type == UIEVENT_SETCURSOR) {
			if (IsEnabled()) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
			return m_pParent->DoEvent(event);
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return m_pParent->DoEvent(event);
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled()) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return m_pParent->DoEvent(event);
		}
		CButtonUI::DoEvent(event);
	}

}
