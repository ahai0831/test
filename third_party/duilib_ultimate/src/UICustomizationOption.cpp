#include "StdAfx.h"

namespace DuiLib
{
	CCustomizationOptionUI::CCustomizationOptionUI() : m_bSelected(false), m_dwSelectedTextColor(0), m_dwSelectedBkColor(0)
	{
	}

	CCustomizationOptionUI::~CCustomizationOptionUI()
	{
		if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
	}

	LPCTSTR CCustomizationOptionUI::GetClass() const
	{
		return _T("CustomizationOptionUI");
	}

	LPVOID CCustomizationOptionUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("CustomizationOption")) == 0) return static_cast<CCustomizationOptionUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	UINT CCustomizationOptionUI::GetControlFlags() const
	{
		if (!IsEnabled()) return CControlUI::GetControlFlags();
		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CCustomizationOptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( bInit && !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
	}

	LPCTSTR CCustomizationOptionUI::GetGroup() const
	{
		return m_sGroupName;
	}

	void CCustomizationOptionUI::SetGroup(LPCTSTR pStrGroupName)
	{
		if( pStrGroupName == NULL ) {
			if( m_sGroupName.IsEmpty() ) return;
			m_sGroupName.Empty();
		}
		else {
			if( m_sGroupName == pStrGroupName ) return;
			if (!m_sGroupName.IsEmpty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
			m_sGroupName = pStrGroupName;
		}

		if( !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
		else {
			if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
		}

		Selected(m_bSelected);
	}

	bool CCustomizationOptionUI::IsSelected() const
	{
		return m_bSelected;
	}

	void CCustomizationOptionUI::Selected(bool bSelected, bool bNotify)
	{
//		if( m_bSelected == bSelected ) return;
		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.IsEmpty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						CCustomizationOptionUI* pControl = static_cast<CCustomizationOptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this ) {
							pControl->Selected(false);
						}
					}
					if (bNotify)
					{
						m_pManager->SendNotify(this, _T("selectchanged"), DUILIB_OP_SELECTCHANGED, m_bSelected);
					}
					
				}
			}
			else {
				if (bNotify)
				{
					m_pManager->SendNotify(this, _T("selectchanged"), DUILIB_OP_SELECTCHANGED, m_bSelected);
				}
			}
		}

		Invalidate();
	}

	bool CCustomizationOptionUI::Activate()
	{
		if( !CButtonUI::Activate() ) return false;
		if( !m_sGroupName.IsEmpty() ) Selected(true);
		else Selected(!m_bSelected);

		return true;
	}

	void CCustomizationOptionUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			if( m_bSelected ) m_uButtonState = UISTATE_SELECTED;
			else m_uButtonState = 0;
		}
	}

	LPCTSTR CCustomizationOptionUI::GetSelectedImage()
	{
		return m_sSelectedImage;
	}

	void CCustomizationOptionUI::SetSelectedImage(LPCTSTR pStrImage)
	{
		m_sSelectedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CCustomizationOptionUI::GetSelectedHotImage()
	{
		return m_sSelectedHotImage;
	}

	void CCustomizationOptionUI::SetSelectedHotImage(LPCTSTR pStrImage)
	{
		m_sSelectedHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CCustomizationOptionUI::GetSelectedPushedImage()
	{
		return m_sSelectedPushedImage;
	}

	void CCustomizationOptionUI::SetSelectedPushedImage(LPCTSTR pStrImage)
	{
		m_sSelectedPushedImage = pStrImage;
		Invalidate();
	}

	void CCustomizationOptionUI::SetSelectedTextColor(DWORD dwTextColor)
	{
		m_dwSelectedTextColor = dwTextColor;
	}

	DWORD CCustomizationOptionUI::GetSelectedTextColor()
	{
		if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
		return m_dwSelectedTextColor;
	}

	void CCustomizationOptionUI::SetSelectedBkColor(DWORD dwBkColor)
	{
		m_dwSelectedBkColor = dwBkColor;
	}

	DWORD CCustomizationOptionUI::GetSelectBkColor()
	{
		return m_dwSelectedBkColor;
	}

	LPCTSTR CCustomizationOptionUI::GetForeImage()
	{
		return m_sForeImage;
	}

	void CCustomizationOptionUI::SetForeImage(LPCTSTR pStrImage)
	{
		m_sForeImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CCustomizationOptionUI::GetSelectedForedImage()
	{
		return m_sSelectedForeImage;
	}

	void CCustomizationOptionUI::SetSelectedForedImage(LPCTSTR pStrImage)
	{
		m_sSelectedForeImage = pStrImage;
		Invalidate();
	}

	SIZE CCustomizationOptionUI::EstimateSize(SIZE szAvailable)
	{
		/*if (m_cxyFixed.cy == 0) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 8);
		return CControlUI::EstimateSize(szAvailable);*/
		SIZE cXY = { 0, 0 };

		//计算字符需要的长度
		RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
		CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		cXY.cx = rcText.right - rcText.left + m_rcTextPadding.left + m_rcTextPadding.right;
		cXY.cy = rcText.bottom - rcText.left + m_rcTextPadding.top + m_rcTextPadding.bottom;
		cXY.cx = MAX(cXY.cx, m_cxyFixed.cx);
		if (m_cxyFixed.cy != 0) cXY.cy = m_cxyFixed.cy;
		return cXY;
	}

	void CCustomizationOptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("group")) == 0 ) SetGroup(pstrValue);
		else if( _tcscmp(pstrName, _T("selected")) == 0 ) Selected(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("selectedimage")) == 0 ) SetSelectedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("selectedhotimage")) == 0 ) SetSelectedHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("selectedpushedimage")) == 0 ) SetSelectedPushedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("selectedforeimage")) == 0 ) SetSelectedForedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("selectedbkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("selectedtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedTextColor(clrColor);
		}
		else CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	void CCustomizationOptionUI::PaintStatusImage(HDC hDC)
	{

		if( (m_uButtonState & UISTATE_PUSHED) != 0 && IsSelected() && !m_sSelectedPushedImage.IsEmpty()) {
			if( !DrawImage(hDC, (LPCTSTR)m_sSelectedPushedImage) )
				m_sSelectedPushedImage.Empty();
			else goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 && IsSelected() && !m_sSelectedHotImage.IsEmpty()) {
			if( !DrawImage(hDC, (LPCTSTR)m_sSelectedHotImage) )
				m_sSelectedHotImage.Empty();
			else goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_SELECTED) != 0 ) {
			if( !m_sSelectedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sSelectedImage) ) m_sSelectedImage.Empty();
				else goto Label_ForeImage;
			}
			else if(m_dwSelectedBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_pManager, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
				return;
			}	
		}

		CButtonUI::PaintStatusImage(hDC);

Label_ForeImage:
		if ( IsSelected() && !m_sSelectedForeImage.IsEmpty())
		{
			if( !DrawImage(hDC, (LPCTSTR)m_sSelectedForeImage) ) m_sSelectedForeImage.Empty();
		}
		else if(  !m_sForeImage.IsEmpty() ) 
		{
			if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
		}

	}

	void CCustomizationOptionUI::PaintText(HDC hDC)
	{
		/*if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
		DWORD oldTextColor = m_dwTextColor;
		if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		if( m_bShowHtml )
		CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
		NULL, NULL, nLinks, m_uTextStyle);
		else
		CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
		m_iFont, m_uTextStyle);

		m_dwTextColor = oldTextColor;
		}
		else
		CButtonUI::PaintText(hDC);*/
	}
}