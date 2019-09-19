#ifndef __UIBACKGROUNDANIMATIONCONTROL_H__
#define __UIBACKGROUNDANIMATIONCONTROL_H__

#include "UIAnimation.h"

#pragma once

namespace DuiLib {

	class UILIB_API CBackgroundAnimationControlUI : public CLabelUI, public CUIAnimation
	{
	public:
		CBackgroundAnimationControlUI(void);
		~CBackgroundAnimationControlUI(void);

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	public:
		void PaintBkImage(HDC hDC);
		//	void PaintStatusImage(HDC hDC);

		void SetElapseTime(UINT uElapse = 40);  // ����ˢ��ʱ��
		void SetListBkImages(LPCTSTR pStrImage);

		void DoEvent(TEventUI& event);
		void OnTimer(int nTimerID);

		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		virtual void OnAnimationStop(INT nAnimationID);
	protected:
		
		UINT     m_nElapse;      //ˢ��ʱ�䣬�Ժ���Ϊ��λ
		vector<wstring> m_listBkImages;
	};
}

#endif //__UIBACKGROUNDANIMATIONCONTROL_H__
