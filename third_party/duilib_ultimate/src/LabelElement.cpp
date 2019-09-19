#include "stdafx.h"
#include "LabelElement.h"

namespace DuiLib {
	CLabelElement::CLabelElement(void)
	{

	}

	LPCTSTR CLabelElement::GetClass() const
	{
		return _T("LabelElementUI");
	}

	LPVOID CLabelElement::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("LabelElementUI")) == 0) return static_cast<CLabelElement*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CLabelElement::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			if (m_pParent != NULL)
				m_pParent->DoEvent(event);
			else
				CLabelUI::DoEvent(event);
			return;
		}

		if (m_pParent == NULL)
		{
			return;
		}

		if (event.Type == UIEVENT_BUTTONUP)
		{
			event.wParam = UISTATE_SELECTED;
			return m_pParent->DoEvent(event);;
		}
		
		CLabelUI::DoEvent(event);
	}


}


