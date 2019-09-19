#include "stdafx.h"
#pragma once
namespace DuiLib {

CTextButton::CTextButton(void)
{
}

CTextButton::~CTextButton(void)
{
}

LPCTSTR CTextButton::GetClass() const
{
	return _T("TextButton");
}

LPVOID CTextButton::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("TextButton")) == 0 ) return static_cast<CTextButton*>(this);
	return CButtonUI::GetInterface(pstrName);
}

UINT CTextButton::GetControlFlags() const
{
	if (!IsEnabled()) return CControlUI::GetControlFlags();

	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

SIZE CTextButton::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0, 0};

	//计算字符需要的长度
	RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
//	CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
// 	cXY.cx = rcText.right - rcText.left + m_rcTextPadding.left + m_rcTextPadding.right;
// 	cXY.cy = rcText.bottom - rcText.left + m_rcTextPadding.top + m_rcTextPadding.bottom;
	HWND hDestopWnd = ::GetDesktopWindow();
	if (hDestopWnd != NULL)
	{
		HDC hdesktopDc = ::GetDC(hDestopWnd);   // 使用桌面窗口上下文句柄，计算文本大小
		CRenderEngine::DrawText(hdesktopDc, m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
		::ReleaseDC(hDestopWnd, hdesktopDc);
		cXY.cx = rcText.right - rcText.left + m_rcTextPadding.left + m_rcTextPadding.right;
		cXY.cy = rcText.bottom - rcText.left + m_rcTextPadding.top + m_rcTextPadding.bottom;
	}
	cXY.cx = MAX(cXY.cx, m_cxyFixed.cx);
	if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
	return cXY;
}

}