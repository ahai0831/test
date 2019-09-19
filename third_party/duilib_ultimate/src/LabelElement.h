#ifndef _LABELELEMENT_HEADER__
#define _LABELELEMENT_HEADER__
#pragma once

namespace DuiLib {
	class UILIB_API CLabelElement : public CLabelUI
	{
	public:
		CLabelElement(void);


		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void DoEvent(TEventUI& event);

	};
}
#endif  //_LABELELEMENT_HEADER__