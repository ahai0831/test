#include "stdafx.h"

namespace DuiLib {


	CStepperControlUI::CStepperControlUI()
	{
		m_nStep = 5;
		m_nTotal = 0;
		m_nMinValue = 0;
		m_nMaxValue = 100;
		m_pTextLabel = new CLabelUI();
		m_pVertical = new CVerticalLayoutUI();
		m_pUpBtn = new CButtonElement();
		m_pDownBtn = new CButtonElement();
		m_pVertical->Add(m_pUpBtn);
		m_pVertical->Add(m_pDownBtn);
		Add(m_pTextLabel);
		Add(m_pVertical);
	}

	CStepperControlUI::~CStepperControlUI()
	{

	}

	LPCTSTR CStepperControlUI::GetClass() const
	{
		return _T("StepperControlUI");
	}

	LPVOID CStepperControlUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("StepperControl")) == 0) return static_cast<CStepperControlUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	LPCTSTR CStepperControlUI::GetNormalImage()
	{
		return m_sNormalImage;
	}

	void CStepperControlUI::SetNormalImage(LPCTSTR pStrImage)
	{
		m_sNormalImage = pStrImage;
	}

	LPCTSTR CStepperControlUI::GetHotImage()
	{
		return m_sHotImage;
	}

	void CStepperControlUI::SetHotImage(LPCTSTR pStrImage)
	{
		m_sHotImage = pStrImage;
	}

	LPCTSTR CStepperControlUI::GetPushedImage()
	{
		return m_sPushedImage;
	}

	void CStepperControlUI::SetPushedImage(LPCTSTR pStrImage)
	{
		m_sPushedImage = pStrImage;
	}

	LPCTSTR CStepperControlUI::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CStepperControlUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
	}

	void CStepperControlUI::SetEnabled(bool enabled)
	{
		__super::SetEnabled(enabled);
		     
		if (m_pTextLabel && m_pUpBtn && m_pDownBtn)
		{
			m_pTextLabel->SetEnabled(enabled);
			m_pUpBtn->SetEnabled(enabled);
			m_pDownBtn->SetEnabled(enabled);
		}
	}

	void CStepperControlUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			if (m_pParent != NULL)
				m_pParent->DoEvent(event);
			else
				CHorizontalLayoutUI::DoEvent(event);
			return;
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if (::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled()) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				if (::PtInRect(&m_rcItem, event.ptMouse)) m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				if (::PtInRect(&m_rcItem, event.ptMouse))
					Activate();
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();
			}
			if (event.pSender && IsEnabled())
			{
				if (event.pSender == m_pUpBtn)
				{
					if (m_nTotal <= m_nMaxValue)
					{
						m_nTotal += m_nStep;
					}

					if (m_nTotal > m_nMaxValue)
					{
						m_nTotal = m_nMinValue;
					}
					CStdString pValue;
					pValue.Format(_T("%02d"), m_nTotal);
					if (m_pTextLabel)
					{
						m_pTextLabel->SetText(pValue);
						if (m_pManager)
						{
							m_pManager->SendNotify(this,_T("click"));
						}
						
					}
					
				}
				else if (event.pSender == m_pDownBtn)
				{
					if (m_nTotal >= m_nMinValue)
					{
						m_nTotal -= m_nStep;
					}
					if (m_nTotal < m_nMinValue)
					{
						m_nTotal = m_nMaxValue;
					}
					CStdString pValue;
					pValue.Format(_T("%02d"), m_nTotal);
					if (m_pTextLabel)
					{
						m_pTextLabel->SetText(pValue);
					}
					if (m_pManager)
					{
						m_pManager->SendNotify(this, _T("click"));
					}

				}
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled()) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
		}
		CHorizontalLayoutUI::DoEvent(event);
	}

	void CStepperControlUI::PaintStatusImage(HDC hDC)
	{
		if (!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;

		if ((m_uButtonState & UISTATE_DISABLED) != 0) {
			if (!m_sDisabledImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sDisabledImage)) m_sDisabledImage.Empty();
				else return;
			}
		}
		else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
			if (!m_sPushedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sPushedImage)) m_sPushedImage.Empty();
				else return;
			}
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!m_sHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sHotImage)) m_sHotImage.Empty();
				else return;
			}
		}

		if (!m_sNormalImage.IsEmpty()) {
			if (!DrawImage(hDC, (LPCTSTR)m_sNormalImage)) m_sNormalImage.Empty();
			else return;
		}
	}

	void CStepperControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{

		if (_tcscmp(pstrName, _T("text")) == 0)
		{
			SetText(pstrValue);
		}	
		else if (_tcscmp(pstrName, _T("labelattr")) == 0)
		{
			if (m_pTextLabel)
				m_pTextLabel->ApplyAttributeList(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("vertattr")) == 0)
		{
			if (m_pVertical)
				m_pVertical->ApplyAttributeList(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("upbtnattr")) == 0)
		{
			if (m_pUpBtn)
				m_pUpBtn->ApplyAttributeList(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("downbtnattr")) == 0)
		{
			if (m_pDownBtn)
				m_pDownBtn->ApplyAttributeList(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("minvalue")) == 0)
		{
			m_nMinValue = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("maxvalue")) == 0)
		{
			m_nMaxValue = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("step")) == 0)
		{
			m_nStep = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("total")) == 0)
		{
			m_nTotal = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("normalimage")) == 0) SetNormalImage(pstrValue);
		else if (_tcscmp(pstrName, _T("hotimage")) == 0) SetHotImage(pstrValue);
		else if (_tcscmp(pstrName, _T("pushedimage")) == 0) SetPushedImage(pstrValue);
		else if (_tcscmp(pstrName, _T("disabledimage")) == 0) SetDisabledImage(pstrValue);
		else CHorizontalLayoutUI::SetAttribute(pstrName,pstrValue);
	}

	DuiLib::CStdString CStepperControlUI::GetText() const
	{
		if (m_pTextLabel)
			return m_pTextLabel->GetText();
	}

	void CStepperControlUI::SetText(LPCTSTR pstrText)
	{
		if (m_pTextLabel)
			m_pTextLabel->SetText(pstrText);
		 

		int num = WideCharToMultiByte(CP_OEMCP, NULL, pstrText, -1, NULL, 0, NULL, FALSE);
		char *pchar = new char[num];
		WideCharToMultiByte(CP_OEMCP, NULL, pstrText, -1, pchar, num, NULL, FALSE);
		 
	    SetTolValue(atoi(pchar));
	}

	int CStepperControlUI::GetToTalValue()
	{
		return m_nTotal;
	}


	void CStepperControlUI::SetTolValue(int value)
	{
		m_nTotal = value;
	}
}
