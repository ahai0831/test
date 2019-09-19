#pragma once
#include "stdafx.h"
namespace DuiLib {

CTextLabel::CTextLabel(void)
{
}

CTextLabel::~CTextLabel(void)
{
}

LPCTSTR CTextLabel::GetClass() const
{
	return _T("TextLabel");
}

LPVOID CTextLabel::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("TextLabel")) == 0 ) return static_cast<CTextLabel*>(this);
	return CLabelUI::GetInterface(pstrName);
}

SIZE CTextLabel::EstimateSize(SIZE szAvailable)
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