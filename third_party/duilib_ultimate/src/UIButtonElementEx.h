#ifndef  _BUTTONELEMENTEX_HEADER__
#define _BUTTONELEMENTEX_HEADER__
#pragma once

namespace DuiLib {
	class UILIB_API CButtonElementEx : public CButtonElement
	{
	public:
		CButtonElementEx(void);


		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void DoEvent(TEventUI& event);

	};
}
#endif  //_BUTTONELEMENTEX_HEADER__