#ifndef __CHILD_HORIZONTAL_LAYOUT_
#define __CHILD_HORIZONTAL_LAYOUT_

#pragma once

// 由子控件长度决定容器长度
namespace DuiLib {

class UILIB_API CChildHorizontalLayout : public CHorizontalLayoutUI
{
public:
	CChildHorizontalLayout();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	virtual SIZE EstimateSize(SIZE szAvailable);   // 获取容器长度


};

}// namespace DuiLib
#endif   //__UIANIMATIONTABLAYOUTUI_H__
