#include "StdAfx.h"
#include "ListElementEx.h"

namespace DuiLib {
	CListElementEx::CListElementEx():m_uTextStyle(DT_VCENTER), m_dwTextColor(1), 
	m_dwDisabledTextColor(1), m_iFont(-1), m_bShowHtml(false)
{
	::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	m_nLeavePos = 0;
	EnableScrollBar(false, false);
	m_bSignalLine = true;
	m_bShowLeft_to_right = false;
}

CListElementEx::~CListElementEx()
{

}

LPCTSTR CListElementEx::GetClass() const
{
	return _T("ListElementEx");
}

LPVOID CListElementEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ListElementEx")) == 0 ) return static_cast<CListElementEx*>(this);
	return CHorizontalLayoutUI::GetInterface(pstrName);
}

bool CListElementEx::Add(CControlUI* pControl)
{
	bool bRes = CHorizontalLayoutUI::Add(pControl);
	if (bRes == true)
	{
		m_VisibleArray.push_back(pControl->IsVisible());
	}
	return bRes;
}

LPCTSTR CListElementEx::GetForeImage()
{
	return m_sForeImage;
}

void CListElementEx::SetForeImage(LPCTSTR pStrImage)
{
	m_sForeImage = pStrImage;
	Invalidate();
}

LPCTSTR CListElementEx::GetFgBkImage()
{
	return m_sFgBkImage;
}

void CListElementEx::SetFgBkImage(LPCTSTR pStrImage)
{
	m_sFgBkImage = pStrImage;
	Invalidate();
}


void CListElementEx::PaintText(HDC hDC)
{
	if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
	if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

	DWORD dwTextColor = 0;
	if (IsEnabled())
	{
		dwTextColor = m_dwTextColor ;
	}
	else
		dwTextColor = m_dwDisabledTextColor;

	if( m_sText.IsEmpty() ) return;
	int nLinks = 0;
	RECT rc = m_rcItem;
	if (m_nButtonSize != 0)
	{
		rc.right = m_nButtonSize;
	}
	rc.left += m_rcTextPadding.left;
	rc.right -= m_rcTextPadding.right;
	rc.top += m_rcTextPadding.top;
	rc.bottom -= m_rcTextPadding.bottom;

	if (m_bSignalLine)
	{
		CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, dwTextColor, \
			m_iFont, DT_SINGLELINE | m_uTextStyle);
		return;
	}
	else
	{
		RECT rcText = { 0, 0, 9999, 9999 };
//		CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		HWND hDestopWnd = ::GetDesktopWindow();
		if (hDestopWnd != NULL)
		{
			HDC hdesktopDc = ::GetDC(hDestopWnd);   // 使用桌面窗口上下文句柄，计算文本大小
			CRenderEngine::DrawText(hdesktopDc, m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
			::ReleaseDC(hDestopWnd, hdesktopDc);
		}
		
		int nWidth = rc.right - rc.left;
		int nNeedWidth = rcText.right - rcText.left;
		int nLines = nNeedWidth/nWidth;
		if (nNeedWidth%nWidth != 0)
		{
			nLines++;
		}
		if (nLines <= 1)
		{
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, dwTextColor, \
				m_iFont, DT_WORDBREAK | m_uTextStyle | DT_EXTERNALLEADING);
		}
		else
		{
			// 多行显示，目前只支持2行
			HFONT hOldFont = (HFONT)::SelectObject(hDC, m_pManager->GetFont(m_iFont));
			TEXTMETRIC tm; 
			GetTextMetrics(hDC, &tm);	   //获取当个字体大小
			::SelectObject(hDC, hOldFont);

			int nCharLines = nWidth/tm.tmAveCharWidth;
			LPCTSTR  pChar = m_sText.GetData();
			int nRealLine1 = 0;
			int nPosx = 0;
			std::vector<int> m_vecLines;   //统计行数
			for(; *pChar; )   
			{
				if(*pChar == '\t')   
				{   
					nRealLine1 ++;
					nPosx += 4*tm.tmAveCharWidth;
				}   
				else   
				{   
					if(*pChar == '\r' || *pChar == '\n')   
					{   
						nRealLine1 ++;
						m_vecLines.push_back(nRealLine1);
						nRealLine1 = 0; // 重置，第二行
						nPosx = 0;
						*pChar ++;
						continue;
//						break;
					}   
					else   
						if( (*pChar > 0x7F))     //中文字符
						{   
							nRealLine1 ++;
							nPosx += tm.tmMaxCharWidth;
//							pChar ++;   
						}   
						else   
						{   
							nRealLine1 ++;
							nPosx += tm.tmAveCharWidth;					
						}   
				} 
				if (nPosx > nWidth)
				{
					nRealLine1 --;
					m_vecLines.push_back(nRealLine1);
					nRealLine1 = 0; // 重置，第二行
					nPosx = 0;
				}
				else
					pChar++;
			}   
			if (nRealLine1 != 0)
			{
				m_vecLines.push_back(nRealLine1);
			}
			RECT rcLine = rc;
			int nPos = 0;
			for (int i = 0;i < m_vecLines.size(); i++)
			{
				rcLine.top = rc.top + (rcText.bottom + m_nLineSpace)*i;
				rcLine.bottom = rcLine.top + rcText.bottom;
				int nlength = m_vecLines.at(i);
				if (i >= (m_nShowLines - 1) )   //超过用户设置的显示行数
				{
					CStdString strLine = m_sText.Right(m_sText.GetLength() - nPos);
					CRenderEngine::DrawText(hDC, m_pManager, rcLine, strLine, dwTextColor, \
						m_iFont, DT_WORDBREAK | m_uTextStyle | DT_EXTERNALLEADING);
					break;
				}
				else
				{
					CStdString strLine = m_sText.Mid(nPos, nlength);
					CRenderEngine::DrawText(hDC, m_pManager, rcLine, strLine, dwTextColor, \
						m_iFont, DT_WORDBREAK | m_uTextStyle | DT_EXTERNALLEADING);
					nPos += nlength;
				}
			}
			
// 			rcLine1.bottom = rc.top + rcText.bottom;
// 			CStdString strLine1 = m_sText.Left(nRealLine1);
// 			CRenderEngine::DrawText(hDC, m_pManager, rcLine1, strLine1, dwTextColor, \
// 				m_iFont, DT_WORDBREAK | m_uTextStyle | DT_EXTERNALLEADING);
// 
// 			RECT rcLine2 = rc;
// 			rcLine2.top = rcLine1.bottom + m_nLineSpace;	
// 			CStdString strLine2 = m_sText.Right(m_sText.GetLength() - nRealLine1);
// 			CRenderEngine::DrawText(hDC, m_pManager, rcLine2, strLine2, dwTextColor, \
// 				m_iFont, DT_WORDBREAK | m_uTextStyle | DT_EXTERNALLEADING);

			int i = 0;
		}
		
	}

}

void CListElementEx::PaintStatusImage(HDC hDC)
{
	CHorizontalLayoutUI::PaintStatusImage(hDC);

	if (!m_sFgBkImage.IsEmpty())
	{
		if( !DrawImage(hDC, (LPCTSTR)m_sFgBkImage) ) m_sFgBkImage.Empty();
	}

	if( !m_sForeImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
	}

}

void CListElementEx::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_SETFOCUS ) 
	{
		m_bFocused = true;
		return;
	}
	if( event.Type == UIEVENT_KILLFOCUS ) 
	{
		m_bFocused = false;
		return;
	}
	CControlUI::DoEvent(event);
}

void CListElementEx::SetTextStyle(UINT uStyle)
{
	m_uTextStyle = uStyle;
	Invalidate();
}

UINT CListElementEx::GetTextStyle() const
{
	return m_uTextStyle;
}

void CListElementEx::SetTextColor(DWORD dwTextColor)
{
	m_dwTextColor = dwTextColor;
}

DWORD CListElementEx::GetTextColor() const
{
	return m_dwTextColor;
}

void CListElementEx::SetDisabledTextColor(DWORD dwTextColor)
{
	m_dwDisabledTextColor = dwTextColor;
}

DWORD CListElementEx::GetDisabledTextColor() const
{
	return m_dwDisabledTextColor;
}

void CListElementEx::SetFont(int index)
{
	m_iFont = index;
}

int CListElementEx::GetFont() const
{
	return m_iFont;
}

RECT CListElementEx::GetTextPadding() const
{
	return m_rcTextPadding;
}

void CListElementEx::SetTextPadding(RECT rc)
{
	m_rcTextPadding = rc;
	Invalidate();
}

bool CListElementEx::IsShowHtml()
{
	return m_bShowHtml;
}

void CListElementEx::SetShowHtml(bool bShowHtml)
{
	if( m_bShowHtml == bShowHtml ) return;

	m_bShowHtml = bShowHtml;
	Invalidate();
}

void CListElementEx::SetShowLines(int nShowLines /* = 1 */)
{
	m_nShowLines = nShowLines;
	if (m_nShowLines == 1)
	{
		m_bSignalLine = true;
	}
	else
		m_bSignalLine = false;
}

int  CListElementEx::GetShowLines() const
{
	return m_nShowLines;
}

void CListElementEx::SetLineSpace(int nLineSpace /*= 5*/)
{
	m_nLineSpace = nLineSpace;
}

int CListElementEx::GetLineSpace() const
{
	return m_nLineSpace;
}


SIZE CListElementEx::EstimateSize(SIZE szAvailable)
{
	if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 4);
	return CControlUI::EstimateSize(szAvailable);
}

void CListElementEx::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("align")) == 0 ) {
		if( _tcsstr(pstrValue, _T("left")) != NULL ) {
			m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_LEFT;
		}
		if( _tcsstr(pstrValue, _T("center")) != NULL ) {
			m_uTextStyle &= ~(DT_LEFT | DT_RIGHT | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_CENTER;
		}
		if( _tcsstr(pstrValue, _T("right")) != NULL ) {
			m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_TOP | DT_BOTTOM);
			m_uTextStyle |= DT_RIGHT;
		}
		if( _tcsstr(pstrValue, _T("top")) != NULL ) {
			m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_TOP;
		}
		if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
			m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_BOTTOM;
		}
	}
	else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
	else if( _tcscmp(pstrName, _T("fgbkimage")) == 0 ) SetFgBkImage(pstrValue);
	else if( _tcscmp(pstrName, _T("endellipsis")) == 0 ) {
		if( _tcscmp(pstrValue, _T("true")) == 0 )
		{
			m_uTextStyle &= ~DT_PATH_ELLIPSIS;
			m_uTextStyle |= DT_END_ELLIPSIS;
		}
		else m_uTextStyle &= ~DT_END_ELLIPSIS;
	}    
	else if (_tcscmp(pstrName, _T("pathellipsis")) == 0)    //add by lighten 2013.03.29, 支持中间省略
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_uTextStyle &= ~DT_END_ELLIPSIS;
			m_uTextStyle |= DT_PATH_ELLIPSIS;
		}
		else m_uTextStyle &= ~DT_PATH_ELLIPSIS;
	}
	else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("showlines")) == 0 ) SetShowLines(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("linespace")) == 0 ) SetLineSpace(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("disabledtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetTextPadding(rcTextPadding);
	}
	else if( _tcscmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
	else if (_tcscmp(pstrName, _T("leavepos")) ==0 )
	{
		LPTSTR pstr = NULL;
		m_nLeavePos= _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
	}
	else if (_tcscmp(pstrName, _T("showdirect")) == 0 )  // 是否正常顺序显示
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_bShowLeft_to_right = true;
		}
		else
			m_bShowLeft_to_right = false;
		Invalidate();
		
	}
	else CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
}

void CListElementEx::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	rc = m_rcItem;
	m_nButtonSize = 0;
	// Adjust for inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;

	if( m_items.GetSize() == 0) {
		return;
	}


	if (m_bShowLeft_to_right)
	{
				// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cx == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;

		int cxExpand = 0;
		if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left + m_nLeavePos;   // 从左到右显示
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsAirLayout()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
				}
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

				cxFixedRemaining -= sz.cx;
			}
			sz.cy = pControl->GetFixedHeight();
			if (sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

			if (rcPadding.top == 0)
			{
				rcPadding.top = (m_rcItem.bottom - m_rcItem.top - sz.cy) / 2;
			}
			if (rcPadding.top < 0)
			{
				rcPadding.top = 0;
			}
			RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy };
			// 			RECT rcCtrl = { iPosX - rcPadding.left - sz.cx - rcPadding.right, rc.top + rcPadding.top, iPosX - rcPadding.right, rc.top + rcPadding.top + sz.cy };
			pControl->SetPos(rcCtrl);
			m_nButtonSize = rcCtrl.left;
			iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
		//非悬浮布局从右到左
		iPosX = rc.right - m_nLeavePos;
		// 	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
		// 		iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		// 	}
		iAdjustable = 0;
		cxFixedRemaining = cxFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (!pControl->IsAirLayout()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
				}
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

				cxFixedRemaining -= sz.cx;
			}
			sz.cy = pControl->GetFixedHeight();
			if (sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

			if (rcPadding.top == 0)
			{
				rcPadding.top = (m_rcItem.bottom - m_rcItem.top - sz.cy) / 2;
			}
			if (rcPadding.top < 0)
			{
				rcPadding.top = 0;
			}

			// 		RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy};
			RECT rcCtrl = { iPosX - rcPadding.left - sz.cx - rcPadding.right, rc.top + rcPadding.top, iPosX - rcPadding.right, rc.top + rcPadding.top + sz.cy };
			pControl->SetPos(rcCtrl);
			m_nButtonSize = rcCtrl.left;
			iPosX -= sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
	}
	else
	{
		// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cx == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;

		int cxExpand = 0;
		if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.right - m_nLeavePos;
		// 	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
		// 		iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		// 	}
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
				}
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

				cxFixedRemaining -= sz.cx;
			}
			sz.cy = pControl->GetFixedHeight();
			if (sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

			if (rcPadding.top == 0)
			{
				rcPadding.top = (m_rcItem.bottom - m_rcItem.top - sz.cy) / 2;
			}
			if (rcPadding.top < 0)
			{
				rcPadding.top = 0;
			}

			// 		RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy};
			RECT rcCtrl = { iPosX - rcPadding.left - sz.cx - rcPadding.right, rc.top + rcPadding.top, iPosX - rcPadding.right, rc.top + rcPadding.top + sz.cy };
			pControl->SetPos(rcCtrl);
			m_nButtonSize = rcCtrl.left;
			iPosX -= sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
		// Process the scrollbar
	}
}

void CListElementEx::SetChildItemVisible(int nIndex, bool bVisible /* = true */)
{
	if (nIndex < 0 || nIndex >= m_items.GetSize())
	{
		return;
	}

	CControlUI* pControl = static_cast<CControlUI*>(m_items[nIndex]);
	if (pControl != NULL)
	{
		pControl->SetVisible(bVisible);
	}
}

// 获取指定索引的控件的位置
CControlUI * CListElementEx::GetChildItemControl(int nIndex)
{
	if( m_items.GetSize() == 0) {
		return NULL;
	}
	if ((nIndex < 0) || (nIndex >= m_items.GetSize()))
	{
		return NULL;
	}
	CControlUI* pControl = static_cast<CControlUI*>(m_items[nIndex]);
	if (pControl != NULL)
	{
		return pControl;
	}
	return NULL;
}



//子项的显示和隐藏
void CListElementEx::ChildVisible(bool bVisible /* = true */)
{
	if( m_items.GetSize() == 0) {
		return;
	}
	for (int it = 0; it<m_items.GetSize(); it++)
	{
		bool bData = m_VisibleArray[it];

		CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
		if (pControl != NULL && !pControl->IsFloat() && pControl->IsAirLayout())
		{
			pControl->SetVisible(bVisible && bData);
		}
	}
	return;
}

void CListElementEx::SetChildItemText(int nIndex, CStdString strText)        //设置指定的子项的文本
{
	if( m_items.GetSize() == 0) {
		return;
	}
	if ((nIndex < 0) || (nIndex >= m_items.GetSize()))
	{
		return;
	}
	CControlUI* pControl = static_cast<CControlUI*>(m_items[nIndex]);
	if (pControl != NULL)
	{
		pControl->SetText(strText);
	}
	return;
}

void CListElementEx::SetChildAirItemVisible(int nIndex, bool bVisible /*= true*/)
{
	if (nIndex < 0 || nIndex >= m_items.GetSize())
	{
		return;
	}

	m_VisibleArray[nIndex] = bVisible;
}



}

