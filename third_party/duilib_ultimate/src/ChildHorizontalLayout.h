#ifndef __CHILD_HORIZONTAL_LAYOUT_
#define __CHILD_HORIZONTAL_LAYOUT_

#pragma once

// ���ӿؼ����Ⱦ�����������
namespace DuiLib {

class UILIB_API CChildHorizontalLayout : public CHorizontalLayoutUI
{
public:
	CChildHorizontalLayout();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	virtual SIZE EstimateSize(SIZE szAvailable);   // ��ȡ��������


};

}// namespace DuiLib
#endif   //__UIANIMATIONTABLAYOUTUI_H__
