#ifndef  _BUTTONELEMENT_HEADER__
#define _BUTTONELEMENT_HEADER__
#pragma once

namespace DuiLib {
class UILIB_API CButtonElement : public CButtonUI 
{
public:
	CButtonElement(void);


	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	bool Activate();
	void DoEvent(TEventUI& event);

	void PaintText(HDC hDC);
	void PaintStatusImage(HDC hDC);

protected:
	UINT m_uButtonState;
	TEventUI *m_pEvent;
};
}
#endif  //_BUTTONELEMENT_HEADER__
