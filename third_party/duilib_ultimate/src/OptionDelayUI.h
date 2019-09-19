#ifndef __OPTIONDELAYUI_H__
#define __OPTIONDELAYUI_H__

#pragma once
//延迟绘制的Tab类，支持覆盖其他非选中项目
namespace DuiLib {

class UILIB_API COptionDelayUI :public COptionUI
{
public:
	COptionDelayUI(void);
	~COptionDelayUI(void);
	
	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;
    bool Activate();
	void Selected(bool bSelected);
	void DoPostPaint(HDC hDC, const RECT& rcPaint);
};

}

#endif //__OPTIONDELAYUI_H__
