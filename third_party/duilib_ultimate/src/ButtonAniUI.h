#ifndef __BUTTONANIUI_H__
#define __BUTTONANIUI_H__

#pragma once

namespace DuiLib {

class UILIB_API CButtonAniUI :public CButtonUI
{
public:
	CButtonAniUI(void);
	~CButtonAniUI(void);

	void DoPostPaint(HDC hDC, const RECT& rcPaint);
    void DoEvent(TEventUI& event);

private:
	RECT m_rcSrc;    //原始大小
	bool m_bShowAni;  //鼠标滑过是否动画
	bool m_bTimer;     //定时器正在运行
	bool m_bLeaveTimer;   //鼠标离开时的定时器
	int m_nEliipsetime;   //定时器刷新时间
	int  m_nStep;         //移动步长
	int  m_nTotalStep;    //移动距离
	int  m_nCountStep;     //移动步数
};

}

#endif //__BUTTONANIUI_H__