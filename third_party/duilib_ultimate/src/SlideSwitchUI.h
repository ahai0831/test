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

		//����ͼƬ
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

		//������߼�����
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
		BOOL		m_bSwitchOpen;	//���鿪�ؿ���
		CStdString m_sBkCloseImage;
		CStdString m_sBkOpenImage;

		POINT m_ptLastMouse;	//����ǰ���λ��
		int m_nLastPos;			//����ǰ�����λ��
		INT m_nThumbPosX;		//����λ��
		INT m_nThumbWidth;		//������
		
		int m_nTotalPos;        //�ܳ���
		bool m_bLoop;           //�Ƿ�ѭ��
		int m_nStep;            //����
		int m_nElapseTime;      //��ʱ�����ʱ��
		DWORD m_dwTimerId;      //��ʱ��ID

		UINT m_uThumbState;		//��¼����״̬
		CStdString m_sThumbNormalImage;
		CStdString m_sThumbHotImage;
		CStdString m_sThumbPushedImage;
		CStdString m_sThumbDisabledImage;

		CStdString m_sImageModify;
	};
}
#endif