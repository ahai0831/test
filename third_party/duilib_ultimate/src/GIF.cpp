#include "stdafx.h"

namespace DuiLib {
CGIFUI::CGIFUI(void) :
CAnimationGIF(this)
{
}

CGIFUI::~CGIFUI(void)
{
}

void CGIFUI::PaintBkImage(HDC hDC)
{
	if( m_sBkImage.IsEmpty() ) 
		return;
	if( !DrawImageEx(hDC, m_pManager, m_rcItem, m_rcPaint, (LPCTSTR)m_sBkImage) ) 
		m_sBkImage.Empty();
}


void CGIFUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_TIMER ) 
	{
		OnTimer(  event.wParam );
		return;
	}
	if (event.Type == UIEVENT_MOUSELEAVE)
	{
//		EmptyGIFHash();
//		StopAnimation();
	}
	__super::DoEvent( event );
}

void CGIFUI::OnTimer( int nTimerID )
{
	OnAnimationElapse( nTimerID );
}

void CGIFUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("Elapse")) == 0 ) 
		SetElapseTime(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("loop")) == 0)
	{
		if (_tcscmp(pstrValue, _T("true")) == 0){SetLoopValue(true);}
		else SetLoopValue(false);
	}
// 	else if (_tcscmp(pstrName, _T("lastframe")) == 0)
// 	{
// 		if (_tcscmp(pstrValue, _T("true")) == 0){SetLastFrame(true);}
// 		else SetLastFrame(false);
// 	}
	else	
		return CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CGIFUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
{
	if (m_pManager != NULL) m_pManager->SendNotify(this, _T("gifstep"), DUILIB_GIF_SWITCH, (WPARAM)nTotalFrame, (LPARAM)nCurFrame);
	m_pManager->Invalidate(m_rcItem);	
}

void CGIFUI::OnAnimationStop(INT nAnimationID) 
{
//	m_pManager->Invalidate(m_rcItem);
//	NeedParentUpdate();
	if (m_pManager != NULL) m_pManager->SendNotify(this, _T("gifstop"), DUILIB_GIF_SWITCH, 0, 0);
}

// void CGIFUI::PaintStatusImage(HDC hDC)
// {
// /*	Stop();*/
// 
// 	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
// 	else m_uButtonState &= ~ UISTATE_FOCUSED;
// 	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
// 	else m_uButtonState &= ~ UISTATE_DISABLED;
// 	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
// 		if( !m_sDisabledImage.IsEmpty() ) {
// 			if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
// 			else return;
// 		}
// 	}
// 	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
// 		if( !m_sPushedImage.IsEmpty() ) {
// 			if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ) m_sPushedImage.Empty();
// 			else return;
// 		}
// 	}
// 	else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
// 		if( !m_sHotImage.IsEmpty() ) {
// 			if( !DrawImageEx(hDC, m_pManager, m_rcItem, m_rcPaint,(LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
// 			else return;
// 		}
// 	}
// 	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
// 		if( !m_sFocusedImage.IsEmpty() ) {
// 			if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
// 			else return;
// 		}
// 	}
// 
// 	if( !m_sNormalImage.IsEmpty() ) {
// 		if( !DrawImage(hDC, /*m_pManager, m_rcItem, m_rcPaint, */(LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
// 		else return;
// 	}
// }




CGIFUIEX::CGIFUIEX(void):
CAnimationGIF(this), m_nMin(0), m_nMax(100), m_nValue(0)
{
	m_rcPaintPos = { 0 };
	SetFixedHeight(12);
}

CGIFUIEX::~CGIFUIEX(void)
{
}

int CGIFUIEX::GetMinValue() const
{
	return m_nMin;
}

void CGIFUIEX::SetMinValue(int nMin)
{
	m_nMin = nMin;
	Invalidate();
}

int CGIFUIEX::GetMaxValue() const
{
	return m_nMax;
}

void CGIFUIEX::SetMaxValue(int nMax)
{
	m_nMax = nMax;
	Invalidate();
}

int CGIFUIEX::GetValue() const
{
	return m_nValue;
}

void CGIFUIEX::SetValue(int nValue)
{
	m_nValue = nValue;
	Invalidate();
}

void CGIFUIEX::PaintBkImage(HDC hDC)
{
	if (m_sBkImage.IsEmpty())
		return;

	//只处理水平 注意
	if (m_nMax <= m_nMin) m_nMax = m_nMin + 1;
	if (m_nValue > m_nMax) m_nValue = m_nMax;
	if (m_nValue < m_nMin) m_nValue = m_nMin;

	RECT rc = { 0 };
	rc.right = (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
	rc.bottom = m_rcItem.bottom - m_rcItem.top;	

	m_rcPaintPos.left = m_rcItem.left;
	m_rcPaintPos.right = m_rcItem.left + rc.right;
	m_rcPaintPos.top = m_rcItem.top;
	m_rcPaintPos.bottom = m_rcItem.top + rc.bottom;

	if (!DrawImageEx(hDC, m_pManager, m_rcPaintPos, m_rcPaintPos, (LPCTSTR)m_sBkImage))
		m_sBkImage.Empty();
}


void CGIFUIEX::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_TIMER)
	{
		OnTimer(event.wParam);
		return;
	}
	if (event.Type == UIEVENT_MOUSELEAVE)
	{
	}
	__super::DoEvent(event);
}

void CGIFUIEX::OnTimer(int nTimerID)
{
	OnAnimationElapse(nTimerID);
}

void CGIFUIEX::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("Elapse")) == 0)SetElapseTime(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("min")) == 0) SetMinValue(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("max")) == 0) SetMaxValue(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("value")) == 0) SetValue(_ttoi(pstrValue));
	else
		return CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CGIFUIEX::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
{
	m_pManager->Invalidate(m_rcItem);
}

void CGIFUIEX::OnAnimationStop(INT nAnimationID)
{

}

}