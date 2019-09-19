#include "stdafx.h"
#include "ButtonEx.h"
namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//
//

CButtonExUI::CButtonExUI() : CButtonUI()
{
	m_uControlNeedLen = 0;
}

LPCTSTR CButtonExUI::GetClass() const
{
	return _T("ButtonExUI");
}

LPVOID CButtonExUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ButtonEx")) == 0 ) return static_cast<CButtonExUI*>(this);
	return CButtonUI::GetInterface(pstrName);
}

UINT CButtonExUI::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

SIZE CButtonExUI::EstimateSize(SIZE szAvailable)
{
	m_uControlNeedLen = m_rcItem.right - m_rcItem.left;
	SIZE cXY = {0, 0};
	CRect rcTextPadding = m_rcTextPadding;
	if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();

	//计算字符需要的长度
	RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
	CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
	cXY.cx = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right;
	cXY.cy = rcText.bottom - rcText.left + rcTextPadding.top + rcTextPadding.bottom;
	m_uControlNeedLen = MAX(cXY.cx, m_uControlNeedLen);
	cXY.cx = m_uControlNeedLen;

	if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
	return cXY;
}

}