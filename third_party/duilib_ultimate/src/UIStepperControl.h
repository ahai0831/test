#ifndef __UISTEPPERCONTROL_H__
#define __UISTEPPERCONTROL_H__

#pragma once
namespace DuiLib {
	class UILIB_API CStepperControlUI : public CHorizontalLayoutUI
	{
	public:
		CStepperControlUI();
		~CStepperControlUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		LPCTSTR GetNormalImage();
		void SetNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetHotImage();
		void SetHotImage(LPCTSTR pStrImage);
		LPCTSTR GetPushedImage();
		void SetPushedImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		void SetDisabledImage(LPCTSTR pStrImage);

		void SetEnabled(bool enabled = true);

		// 文本相关
		virtual CStdString GetText() const;
		virtual void SetText(LPCTSTR pstrText);
		int    GetToTalValue();
		void    SetTolValue(int value);

		void DoEvent(TEventUI& event);

		void PaintStatusImage(HDC hDC);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	protected:
		CStdString m_sNormalImage;
		CStdString m_sHotImage;
		CStdString m_sPushedImage;
		CStdString m_sFocusedImage;
		CStdString m_sDisabledImage;

		CLabelUI*				m_pTextLabel;
		CVerticalLayoutUI*	m_pVertical;
		CButtonElement*    m_pUpBtn;
		CButtonElement*	   m_pDownBtn;
		int			m_nStep;
		int			m_nTotal;
		int			m_nMinValue;
		int			m_nMaxValue;

	};

}

#endif