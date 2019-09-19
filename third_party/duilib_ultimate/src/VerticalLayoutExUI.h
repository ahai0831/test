#ifndef __VERTICAL_LAYOUT_EX_H__
#define __VERTICAL_LAYOUT_EX_H__
#pragma once

namespace DuiLib {
class UILIB_API CVerticalLayoutExUI :public CVerticalLayoutUI
{
public:
	CVerticalLayoutExUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	void DoEvent(TEventUI& event);

};
}
#endif //__VERTICAL_LAYOUT_EX_H__