#include "stdafx.h"

namespace DuiLib {

CButtonAniUI::CButtonAniUI(void)
{
	m_bShowAni = true;
	m_bTimer = false;
	m_bLeaveTimer = false;
	m_nEliipsetime = 25;
	m_nStep = 2;
	m_nTotalStep = 10;
	m_nCountStep = 0;
}

CButtonAniUI::~CButtonAniUI(void)
{
}

void CButtonAniUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) 
	{
		if( m_pParent != NULL ) 
			m_pParent->DoEvent(event);
		else
			CButtonUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( IsEnabled() ) {
			if (m_bShowAni && !m_bTimer)   //需要动画效果而且没有开始动画
			{
				if ((m_bTimer == false) && (m_bLeaveTimer == false))
				{
					m_rcSrc = m_rcItem;
				}
				if (m_bLeaveTimer)   //正在播放离开动画
				{
					m_pManager->KillTimer(this, 0x100);
					m_bLeaveTimer = false;
				}

				m_pManager->AddPostPaint(this);
				m_pManager->SetTimer(this, 0x101, m_nEliipsetime);
				m_bTimer = true;
			}
			m_uButtonState |= UISTATE_HOT;
			if (!m_bShowAni)
			{
				Invalidate();
			}
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if (m_bShowAni && !m_bLeaveTimer)
		{
			m_pManager->KillTimer(this, 0x101);
			m_bTimer = false;
			m_pManager->SetTimer(this, 0x100, m_nEliipsetime);
			m_bLeaveTimer = true;
		}
		if( IsEnabled() ) {
			m_uButtonState &= ~UISTATE_HOT;
			if (!m_bShowAni)
			{
				Invalidate();
			}
		}
		return;
	}
	if (event.Type == UIEVENT_TIMER)
	{
		if (event.wParam == 0x100)
		{
			m_nCountStep --;
			m_pManager->Invalidate(m_rcItem);
			m_rcItem.left += m_nStep;
			m_rcItem.top += m_nStep;
			m_rcItem.right -= m_nStep;
			m_rcItem.bottom -= m_nStep;

			if (m_nCountStep <= 0)    //动画结束，临界点计算
			{
				m_nCountStep = 0;
				m_rcItem = m_rcSrc;
				m_pManager->KillTimer(this, 0x100);
				m_pManager->RemovePostPaint(this);
				m_bLeaveTimer = false;
			}
			return;
		}
		if (event.wParam == 0x101)
		{
			m_nCountStep ++;
			m_rcItem.left -= m_nStep;
			m_rcItem.top -= m_nStep;
			m_rcItem.right += m_nStep;
			m_rcItem.bottom += m_nStep;
			m_pManager->Invalidate(m_rcItem);
			if (m_nCountStep >= 5)    //动画结束，临界点计算
			{
				RECT m_rcMaxSize = m_rcSrc;
				m_rcMaxSize.left -= m_nTotalStep;
				m_rcMaxSize.top -= m_nTotalStep;
				m_rcMaxSize.right += m_nTotalStep;
				m_rcMaxSize.bottom += m_nTotalStep;
				m_rcItem = m_rcMaxSize;
				m_nCountStep = 5;
				m_pManager->KillTimer(this, 0x101);
				m_bTimer = false;
			}
		}

	}
	return CButtonUI::DoEvent(event);
}

void CButtonAniUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
	PaintStatusImage(hDC);
	PaintText(hDC);
}

}