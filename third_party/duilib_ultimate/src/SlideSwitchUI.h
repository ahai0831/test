#ifndef __SLIDESWITCHUI_H__
#define __SLIDESWITCHUI_H__

#pragma once

namespace DuiLib {
	class UILIB_API CSlideSwitchUI:public CControlUI, public CUIAnimation
	{
	public:
		CSlideSwitchUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetEnabled(bool bEnable = true);

		//设置图片
		LPCTSTR GetThumbNormalImage() const;
		void SetThumbNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbHotImage() const;
		void SetThumbHotImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbPushedImage() const;
		void SetThumbPushedImage(LPCTSTR pStrImage);

		LPCTSTR GetBkCloseImage();
		void SetBkCloseImage(LPCTSTR pStrImage);
		LPCTSTR GetBkOpenImage();
		void SetBkOpenImage(LPCTSTR pStrImage);

		//滑块的逻辑处理
		void SetThumbWidth(INT nThumbWidth);
		void SetThumbPos(INT nThumbPos);
		RECT GetThumbRect() const;
		BOOL IsSwitchOpen();
		void SetSwitchState(BOOL bOpen);
		

		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);


		void DoPaint(HDC hDC, const RECT& rcPaint);
		void PaintBk(HDC hDC);
		void PaintThumb(HDC hDC);

		void OnTimer( int nTimerID );

		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		virtual void OnAnimationStop(INT nAnimationID){}


	protected:
		BOOL		m_bSwitchOpen;	//滑块开关控制
		CStdString m_sBkCloseImage;
		CStdString m_sBkOpenImage;

		POINT m_ptLastMouse;	//滑动前鼠标位置
		int m_nLastPos;			//滑动前滑块的位置
		INT m_nThumbPosX;		//滑块位置
		INT m_nThumbWidth;		//滑块宽度
		
		int m_nTotalPos;        //总长度
		bool m_bLoop;           //是否循环
		int m_nStep;            //步长
		int m_nElapseTime;      //定时器间隔时间
		DWORD m_dwTimerId;      //定时器ID

		UINT m_uThumbState;		//记录滑块状态
		CStdString m_sThumbNormalImage;
		CStdString m_sThumbHotImage;
		CStdString m_sThumbPushedImage;
		CStdString m_sThumbDisabledImage;

		CStdString m_sImageModify;
	};
}
#endif