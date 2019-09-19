#include "stdafx.h"
#include "ButtonNonUI.h"

namespace DuiLib {
	CButtonNonUI::CButtonNonUI(void)
	{

	}


	CButtonNonUI::~CButtonNonUI(void)
	{


	}
	UINT CButtonNonUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	LPCTSTR CButtonNonUI::GetClass() const
	{
		return _T("NonButton");
	}

	LPVOID CButtonNonUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("NonButton")) == 0) return static_cast<CButtonNonUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}


	void CButtonNonUI::DoEvent(TEventUI& event)
	{

		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			if (m_pParent != NULL)
				m_pParent->DoEvent(event);
			else
				CButtonUI::DoEvent(event);
			return;
		}


		if (event.Type == UIEVENT_SETCURSOR) {
			//::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				//::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled()) {
				//::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
				m_uButtonState &= ~UISTATE_HOT; 
				Invalidate();
			}
			return;
		}
	 
		return CButtonUI::DoEvent(event);
	}

}