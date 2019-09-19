#include "stdafx.h"
namespace DuiLib
{

CAnimationCtrl::CAnimationCtrl(void) :
CUIAnimation(this)
{
	m_nCurPos = 0;
	m_nTotalPos = 0;
	m_nLastPos = 0;
	m_nElapseTime = 30;
	m_dwTimerId = 0x10001;
	m_nStep = 5;
	m_dwHiddenTimerId = 0x10002;
	m_nHiddenElapseTime = 20;
	m_bResetShow = false;
	m_bSelectChanged = false;
	m_nIndexSelect = -1;
	m_bVisibleControl = false;
	m_bHiddenControl = false;
}

CAnimationCtrl::~CAnimationCtrl(void)
{
	StopAnimation(0);
//	if (m_pManager)
//	{
//		m_pManager->KillTimer(this);
//	}
}

LPCTSTR CAnimationCtrl::GetClass() const
{
	return _T("AnimationCtrl");
}

LPVOID CAnimationCtrl::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("AnimationCtrl")) == 0 ) 
		return static_cast<CAnimationCtrl*>(this);
	return CTabLayoutUI::GetInterface(pstrName);
}

void CAnimationCtrl::SetPos(RECT rc)
{
//	CTabLayoutUI::SetPos(rc);

	if (m_bSelectChanged || m_bResetShow || m_bVisibleControl)   // 当前正在动画时，不重新计算位置
	{
		return;
	}
	CControlUI::SetPos(rc);
	rc = m_rcItem;
	int cx = rc.right - rc.left  - m_nCurPos;

	// Adjust for inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;

	for( int it = 0; it < m_items.GetSize(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) {
			SetFloatPos(it);
			continue;
		}

		if( it != m_iCurSel ) continue;

		RECT rcPadding = pControl->GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

		SIZE sz = pControl->EstimateSize(szAvailable);
		if( sz.cx == 0 ) {
			sz.cx = MAX(0, szAvailable.cx);
		}
		if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
		if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

		if(sz.cy == 0) {
			sz.cy = MAX(0, szAvailable.cy);
		}
		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

		RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy};
		rcCtrl.left += cx;
		rcCtrl.right += cx;
		pControl->SetPos(rcCtrl);
	}

	if (!m_bHiddenControl)
	{
		int cxNeeded = 0;
		int cyNeeded = 0;
		cxNeeded = MAX(0, EstimateSize(CSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
		if( cxNeeded > 0 )
		{
			// pos计算，状态初始化
			if (IsAnimationRunning(m_dwTimerId))
			{
				StopAnimation(m_dwTimerId);
			}
			if (IsAnimationRunning(m_dwHiddenTimerId))
			{
				StopAnimation(m_dwHiddenTimerId);
			}
			m_nTotalPos = cxNeeded;
			m_nCurPos = 0;
//			m_nLastPos = 0;
			int nPos = m_nTotalPos;
			int nFrame = nPos/m_nStep;
			if ((nPos%m_nStep) != 0 )
			{
				nFrame ++;
			}
			StartAnimation(m_nElapseTime, nFrame, m_dwTimerId, false);
		}
	}

}

void CAnimationCtrl::HideAnimation(bool bHidden /* = true */)
{
	m_bHiddenControl = bHidden;
	if (IsAnimationRunning(m_dwTimerId))
	{
		StopAnimation(m_dwTimerId);
	}
	if ((m_nCurPos > 0) && (!IsAnimationRunning(m_dwHiddenTimerId)))
	{
		int nFrame = m_nCurPos/m_nStep;
		if (m_nCurPos%m_nStep != 0)
		{
			nFrame++;
		}
		StartAnimation(m_nHiddenElapseTime, nFrame, m_dwHiddenTimerId, false);
	}
}

void CAnimationCtrl::ShowAnimation()
{
	m_bHiddenControl = false;
	if (IsAnimationRunning(m_dwHiddenTimerId))
	{
		StopAnimation(m_dwHiddenTimerId);
	}

	if ((m_nCurPos < m_nTotalPos) && (!IsAnimationRunning(m_dwTimerId)))
	{
		int nPos = m_nTotalPos - m_nCurPos;
		int nFrame = nPos/m_nStep;
		if (nPos %m_nStep)
		{
			nFrame ++;
		}
		StartAnimation(m_nElapseTime, nFrame, m_dwTimerId, false);
	}

}

// 隐藏当前显示内容，重新显示
void CAnimationCtrl::ResetShowAnimation()
{
	m_bHiddenControl = false;
	m_bResetShow = true;
	m_bSelectChanged = false;
	HideAnimation(false);
	if (m_nCurPos <= 0)
	{
		int nPos = m_nTotalPos - m_nCurPos;
		int nFrame = nPos/m_nStep;
		if (nPos %m_nStep)
		{
			nFrame ++;
		}
		// 启动重新显示
		StartAnimation(m_nElapseTime, nFrame, m_dwTimerId, false);
		m_bResetShow = false;
	}

}

//滑动pos
void CAnimationCtrl::ScrollPos()
{
	if (m_nCurPos > m_nTotalPos)   // 校正数据范围
	{
		m_nCurPos = m_nTotalPos;
//		return;
	}
	if (m_nCurPos < 0)
	{
		m_nCurPos = 0;
	}
	int cx = (m_nCurPos - m_nLastPos);
	int cy = 0;
	if( cx == 0) return;

	RECT rcPos;
	for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
		if( !pControl->IsVisible() ) continue;
		if( pControl->IsFloat() ) continue;

		rcPos = pControl->GetPos();
		rcPos.left -= cx;
		rcPos.right -= cx;
		rcPos.top -= cy;
		rcPos.bottom -= cy;
		pControl->SetPos(rcPos);
	}
	m_nLastPos = m_nCurPos;
	//	Invalidate();
}

SIZE CAnimationCtrl::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0, m_cxyFixed.cy};
	if( cXY.cy == 0 && m_pManager != NULL ) {
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			cXY.cy = MAX(cXY.cy,static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
		}
		int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6;
		cXY.cy = MAX(cXY.cy,nMin);
	}

	// 	for( int it = 0; it < m_items.GetSize(); it++ ) {
	if (m_iCurSel >=0 && m_iCurSel <= m_items.GetSize() - 1)
	{
		cXY.cx +=  static_cast<CControlUI*>(m_items[m_iCurSel])->EstimateSize(szAvailable).cx;
	}
	//	}

	return cXY;
}

void CAnimationCtrl::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_TIMER ) 
	{
		OnTimer(  event.wParam );
	}
	__super::DoEvent( event );
}

void CAnimationCtrl::OnTimer( int nTimerID )
{
	OnAnimationElapse( nTimerID );
}

void CAnimationCtrl::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
{
	if (nAnimationID == m_dwTimerId)   // 显示
	{
		m_nCurPos = m_nLastPos;
		if (m_nCurPos >= m_nTotalPos)   // 判断条件
		{
			m_nCurPos = m_nTotalPos;
			return;
		}
		m_nCurPos += m_nStep;
		if (m_nCurPos > m_nTotalPos)    // 限定大小
		{
			m_nCurPos = m_nTotalPos;
		}
		ScrollPos();
	}
	else if(nAnimationID == m_dwHiddenTimerId)   // 隐藏
	{
		m_nCurPos = m_nLastPos;
		if (m_nCurPos <= 0)        // 判断条件
		{
			m_nCurPos = 0;
			return;
		}
		m_nCurPos -= m_nStep;
		if (m_nCurPos < 0)          // 限定大小
		{
			m_nCurPos = 0;
		}
		ScrollPos();
	}
}

void CAnimationCtrl::OnAnimationStop(INT nAnimationID) 
{
	if (nAnimationID == m_dwHiddenTimerId)    // 隐藏结束
	{
		if (m_bSelectChanged)   // 需要重新选择
		{
			m_bSelectChanged = false;
			CTabLayoutUI::SelectItem(m_nIndexSelect, false);
		}
		if (m_bResetShow)    // 需要重新启动
		{
			m_bResetShow = false;
			int nPos = m_nTotalPos - m_nCurPos;
			int nFrame = nPos/m_nStep;
			if (nPos %m_nStep)
			{
				nFrame ++;
			}
			StartAnimation(m_nElapseTime, nFrame, m_dwTimerId, false);
		}
		if (m_bVisibleControl)
		{
			m_bVisibleControl = false;
			CTabLayoutUI::SetVisible(false);
		}
	}
}

// 改写选中项目，实现自动变化
bool CAnimationCtrl::SelectItem(int iIndex, bool bVisible /* = true */)
{
	if ((iIndex < 0) || (iIndex >= m_items.GetSize()))
	{
		return false;
	}
	m_bHiddenControl = false;
	if (m_iCurSel == iIndex)   // 选中当前项目
	{
		if (!IsVisible())    // 当前项目没有显示
		{
			CTabLayoutUI::SelectItem(iIndex, false);
		}
		else 
			ResetShowAnimation();
	}
	else    // 选中其他项目
	{
		m_nIndexSelect = iIndex;
		m_bSelectChanged = true;
		m_bResetShow = false;
		HideAnimation(false);
		if (m_nCurPos <= 0)
		{
			// 选中新项目
			m_bSelectChanged = false;
			CTabLayoutUI::SelectItem(m_nIndexSelect, false);
		}
	}
	if (bVisible && !IsVisible())
	{
		SetVisible();		
	}
	return true;
}

// 改写隐藏，需先动画完成，再隐藏
void CAnimationCtrl::SetVisible(bool bVisible /* = true */)
{
	if (bVisible == false)
	{
		m_bVisibleControl = true;
		HideAnimation(true);
		if (m_nCurPos <= 0)
		{
			m_bVisibleControl = false;
			CTabLayoutUI::SetVisible(bVisible);
		}
	}
	else
	{
		m_bHiddenControl = false;
		CTabLayoutUI::SetVisible(bVisible);
	}
}

// 设置显示时间间隔
void CAnimationCtrl::SetShowElapseTime(int nShowElpase)          
{
	if (nShowElpase > 0)
	{
		m_nElapseTime = nShowElpase;
	}
}

// 设置隐藏时间间隔 
void CAnimationCtrl::SetHiddenElapseTime(int nHiddenElpase)
{
	if (nHiddenElpase > 0)
	{
		m_nHiddenElapseTime = nHiddenElpase;
	}
}

// 设置动画步长
void CAnimationCtrl::SetAnimationStep(int nStep)
{
	if (nStep > 0)
	{
		m_nStep = nStep;
	}
}


void CAnimationCtrl::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("showelapse")) == 0 ) SetShowElapseTime(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("hiddenelapse")) == 0 ) SetHiddenElapseTime(_ttoi(pstrValue));
	else if( _tcscmp(pstrName, _T("animation_step")) == 0 ) SetAnimationStep(_ttoi(pstrValue));

	 return CTabLayoutUI::SetAttribute(pstrName, pstrValue);
}




}
