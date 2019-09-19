#include "stdafx.h"
namespace DuiLib
{

CAnimationBar::CAnimationBar(void) :
CUIAnimation(this)
{
	m_nCurPos = 0;
	m_nTotalPos = 0;
	m_nLastPos = 0;
	m_bLoop = false;
	m_nElapseTime = 120;
	m_dwTimerId = 0x100;
	m_nStep = 5;
	m_bTurnToStart = false;
}

CAnimationBar::~CAnimationBar(void)
{
	StopAnimation(m_dwTimerId);
}

LPCTSTR CAnimationBar::GetClass() const
{
	return _T("AnimationBar");
}

LPVOID CAnimationBar::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("AnimationBar")) == 0 ) 
		return static_cast<CAnimationBar*>(this);
	return CHorizontalLayoutUI::GetInterface(pstrName);
}

void CAnimationBar::SetPos(RECT rc)
{
	CHorizontalLayoutUI::SetPos(rc);
	int cxNeeded = 0;
	int cyNeeded = 0;
	cxNeeded = MAX(0, EstimateSize(CSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
	if( cxNeeded > rc.right - rc.left )
	{
		m_nTotalPos = cxNeeded - rc.right + rc.left;
		if (IsAnimationRunning(m_dwTimerId))
		{
			StopAnimation(m_dwTimerId);
		}
		StartAnimation(m_nElapseTime, m_nTotalPos/m_nStep, m_dwTimerId, m_bLoop);
	}
}

//»¬¶¯pos
void CAnimationBar::ScrollPos()
{
	if (m_nCurPos > m_nTotalPos && !m_bLoop)
	{
		return;
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

SIZE CAnimationBar::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0, m_cxyFixed.cy};
	if( cXY.cy == 0 && m_pManager != NULL ) {
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			cXY.cy = MAX(cXY.cy,static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
		}
		int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6;
		cXY.cy = MAX(cXY.cy,nMin);
	}

	for( int it = 0; it < m_items.GetSize(); it++ ) {
		cXY.cx +=  static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
	}

	return cXY;
}

void CAnimationBar::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_TIMER ) 
	{
		OnTimer(  event.wParam );
	}
	__super::DoEvent( event );
}

void CAnimationBar::OnTimer( int nTimerID )
{
	OnAnimationElapse( nTimerID );
}

void CAnimationBar::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
{
	m_nCurPos = m_nLastPos;
	m_nCurPos += m_nStep;
	if (m_nCurPos > m_nTotalPos)
	{
		if (m_bLoop)
		{
			m_nCurPos = 0;
		}
		else
		{
			StopAnimation(m_dwTimerId);
			return;
		}
	}
	ScrollPos();
}

void CAnimationBar::OnAnimationStop(INT nAnimationID) 
{
	if (m_bLoop && m_bTurnToStart)
	{
		NeedParentUpdate();
	}
}

}
