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
	RECT m_rcSrc;    //ԭʼ��С
	bool m_bShowAni;  //��껬���Ƿ񶯻�
	bool m_bTimer;     //��ʱ����������
	bool m_bLeaveTimer;   //����뿪ʱ�Ķ�ʱ��
	int m_nEliipsetime;   //��ʱ��ˢ��ʱ��
	int  m_nStep;         //�ƶ�����
	int  m_nTotalStep;    //�ƶ�����
	int  m_nCountStep;     //�ƶ�����
};

}

#endif //__BUTTONANIUI_H__