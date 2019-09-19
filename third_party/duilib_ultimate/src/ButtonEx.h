#ifndef __BUTTON_EX_H__
#define __BUTTON_EX_H__
#pragma once

namespace DuiLib {
class UILIB_API CButtonExUI :public CButtonUI
{
public:
	CButtonExUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	SIZE EstimateSize(SIZE szAvailable);

protected:
	int m_uControlNeedLen;   //显示控件需要的长度
};
}
#endif //__BUTTON_EX_H__