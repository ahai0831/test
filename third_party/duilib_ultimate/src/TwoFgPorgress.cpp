#include "stdafx.h"
namespace DuiLib {
CTwoFgPorgressUI::CTwoFgPorgressUI(void)
{
	m_nTwoValue = 0;
}

CTwoFgPorgressUI::~CTwoFgPorgressUI(void)
{
}

LPCTSTR CTwoFgPorgressUI::GetClass() const
{
	return _T("TwoProgressUI");
}

LPVOID CTwoFgPorgressUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("TwoProgress")) == 0 ) return static_cast<CTwoFgPorgressUI*>(this);
	return CProgressUI::GetInterface(pstrName);
}

int CTwoFgPorgressUI::GetTwoValue() const
{
	return m_nTwoValue;
}

void CTwoFgPorgressUI::SetTwoValue(int nValue)
{
	m_nTwoValue = nValue;
	Invalidate();
}

LPCTSTR CTwoFgPorgressUI::GetTwoFgImage() const
{
	return m_sTwoFgImage;
}

void CTwoFgPorgressUI::SetTwoFgImage(LPCTSTR pStrImage, bool bInvalidate /* = false */)
{
	if( m_sTwoFgImage == pStrImage ) return;

	m_sTwoFgImage = pStrImage;
	if (bInvalidate)
	{
		Invalidate();
	}
}

void CTwoFgPorgressUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("twofgimage")) == 0 ) SetTwoFgImage(pstrValue);
	else if( _tcscmp(pstrName, _T("twovalue")) == 0 ) SetTwoValue(_ttoi(pstrValue));
	else CProgressUI::SetAttribute(pstrName, pstrValue);
}

void CTwoFgPorgressUI::PaintStatusImage(HDC hDC)
{
	if( m_nMax <= m_nMin ) 
		m_nMax = m_nMin + 1;
	if( m_nValue > m_nMax ) 
		m_nValue = m_nMax;
	if( m_nValue < m_nMin ) 
		m_nValue = m_nMin;

	if (m_nTwoValue > m_nMax) 
		m_nValue = m_nMax;

	if (m_nTwoValue < m_nMin) 
		m_nTwoValue = m_nMin;

	//Fixed ,如果二者相加大于最大值，以第一个为主
	if (m_nTwoValue + m_nValue > m_nMax) 
		m_nTwoValue = m_nMax - m_nValue;

	RECT rc = {0};
	RECT rcTwo = {0};
	if( m_bHorizontal ) {
		rc.right = (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
		rc.bottom = m_rcItem.bottom - m_rcItem.top;
		rcTwo.left = rc.right;
		rcTwo.right = rc.right + (m_nTwoValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
		rcTwo.bottom = rc.bottom;
	}
	else {
		rc.top = (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);
		rc.right = m_rcItem.right - m_rcItem.left;
		rc.bottom = m_rcItem.bottom - m_rcItem.top;

		rcTwo.bottom = rc.top;
		rcTwo.right = rc.right;
		rcTwo.top = rcTwo.bottom + (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nTwoValue) / (m_nMax - m_nMin);		
	}

	if( !m_sFgImage.IsEmpty() ) {
		m_sFgImageModify.Empty();
		m_sFgImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rc.left, rc.top, rc.right, rc.bottom);

		if( !DrawImage(hDC, (LPCTSTR)m_sFgImage, (LPCTSTR)m_sFgImageModify) ) 
			m_sFgImage.Empty();
	}
	if (!m_sTwoFgImage.IsEmpty())
	{
		m_sTwoFgImageModify.Empty();
		m_sTwoFgImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcTwo.left, rcTwo.top, rcTwo.right, rcTwo.bottom);
		if (!DrawImage(hDC, (LPCTSTR)m_sTwoFgImage, (LPCTSTR)m_sTwoFgImageModify))
		{
			m_sFgImage.Empty();
		}
	}
	return;
}

}