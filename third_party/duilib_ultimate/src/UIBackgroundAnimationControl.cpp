#include "stdafx.h"
#include "UIBackgroundAnimationControl.h"

namespace DuiLib {
	


	CBackgroundAnimationControlUI::CBackgroundAnimationControlUI(void)
		: CUIAnimation(this)
	{
		m_nElapse = 100;
	}

	CBackgroundAnimationControlUI::~CBackgroundAnimationControlUI(void)
	{
		m_listBkImages.clear();
	}

	void CBackgroundAnimationControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("Elapse")) == 0)
			SetElapseTime(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("listbkimages")) == 0)
		{
			SetListBkImages(pstrValue);
		}
		else
			return CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CBackgroundAnimationControlUI::PaintBkImage(HDC hDC)
	{
		int ncurFrame = GetCurrentFrame();
		if (ncurFrame < m_listBkImages.size())
		{
			m_sBkImage = m_listBkImages.at(ncurFrame).c_str();
		}
		if (m_sBkImage.IsEmpty())
			return;
		if (!DrawImage(hDC,(LPCTSTR)m_sBkImage))
			m_sBkImage.Empty();
	}

	void CBackgroundAnimationControlUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER)
		{
			OnTimer(event.wParam);
			return;
		}
		__super::DoEvent(event);
	}

	void CBackgroundAnimationControlUI::OnTimer(int nTimerID)
	{
		OnAnimationElapse(nTimerID);
	}

	void CBackgroundAnimationControlUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
	{
		Invalidate();
	}

	void CBackgroundAnimationControlUI::OnAnimationStop(INT nAnimationID)
	{

	}

	void CBackgroundAnimationControlUI::SetElapseTime(UINT uElapse /*= 50*/)
	{
		m_nElapse = uElapse;
	}

	void CBackgroundAnimationControlUI::SetListBkImages(LPCTSTR pStrImage)
	{
		CStdString sListImageName = pStrImage;
		CStdString sValue;
		int nPos = sListImageName.Find(_T(";"));
		while(-1 != nPos)
		{
			sValue = sListImageName.Left(nPos);
			m_listBkImages.push_back(sValue.GetData());
			sListImageName = sListImageName.Right(sListImageName.GetLength() - nPos - 1);
			nPos = sListImageName.Find(_T(";"));
		}
		if (!sListImageName.IsEmpty())
		{
			m_listBkImages.push_back(sListImageName.GetData());
		}
		StartAnimation(m_nElapse, m_listBkImages.size() - 1,0,TRUE);
	}

	LPCTSTR CBackgroundAnimationControlUI::GetClass() const
	{
		return _T("BackgroundAnimationControlUI");
	}

	LPVOID CBackgroundAnimationControlUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("BackgroundAnimationControl")) == 0) return static_cast<CBackgroundAnimationControlUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

}

