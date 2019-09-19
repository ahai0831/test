#include "StdAfx.h"


namespace DuiLib {
	CSlideSwitchUI::CSlideSwitchUI(): m_uThumbState(0)
		,m_nThumbPosX(0)
		,m_nThumbWidth(15)
		,m_bSwitchOpen(FALSE)
		,m_nLastPos(0)
		,m_nTotalPos(0)
		,m_bLoop(false)
		,m_nStep(5)
		,m_nElapseTime(10)
		,m_dwTimerId(0x110)
		,CUIAnimation(this)
	{
		m_ptLastMouse.x = m_ptLastMouse.y = 0;
	}

	LPCTSTR CSlideSwitchUI::GetClass() const
	{
		 return _T("SlideSwitchUI");
	}

	LPVOID CSlideSwitchUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, _T("SlideSwitch")) == 0 ) return static_cast<CSlideSwitchUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CSlideSwitchUI::SetEnabled( bool bEnable /*= true*/ )
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uThumbState = 0;
		}
	}

	LPCTSTR CSlideSwitchUI::GetThumbNormalImage() const
	{
		return m_sThumbNormalImage;
	}

	void CSlideSwitchUI::SetThumbNormalImage( LPCTSTR pStrImage )
	{
		m_sThumbNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSlideSwitchUI::GetThumbHotImage() const
	{
		return m_sThumbHotImage;
	}

	void CSlideSwitchUI::SetThumbHotImage( LPCTSTR pStrImage )
	{
		m_sThumbHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSlideSwitchUI::GetThumbPushedImage() const
	{
		return m_sThumbPushedImage;
	}

	void CSlideSwitchUI::SetThumbPushedImage( LPCTSTR pStrImage )
	{
		m_sThumbPushedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSlideSwitchUI::GetBkCloseImage()
	{
		return m_sBkCloseImage;
	}

	void CSlideSwitchUI::SetBkCloseImage( LPCTSTR pStrImage )
	{
		m_sBkCloseImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSlideSwitchUI::GetBkOpenImage()
	{
		return m_sBkOpenImage;
	}

	void CSlideSwitchUI::SetBkOpenImage( LPCTSTR pStrImage )
	{
		m_sBkOpenImage = pStrImage;
		Invalidate();
	}

	void CSlideSwitchUI::SetThumbWidth(INT nThumbWidth)
	{
		m_nThumbWidth = nThumbWidth;
	}

	void CSlideSwitchUI::SetThumbPos(INT nThumbPos)
	{
		m_nThumbPosX = nThumbPos;
	}

	RECT CSlideSwitchUI::GetThumbRect() const
	{
		ASSERT(m_nThumbPosX <= m_rcItem.right - m_rcItem.left - m_nThumbWidth);
		return CRect(m_rcItem.left + m_nThumbPosX, m_rcItem.top, m_rcItem.left + m_nThumbPosX + m_nThumbWidth, m_rcItem.bottom);
	}

	BOOL CSlideSwitchUI::IsSwitchOpen()
	{
		return m_bSwitchOpen;
	}

	void CSlideSwitchUI::SetSwitchState(BOOL bOpen)
	{
		m_bSwitchOpen = bOpen;
		if(m_bSwitchOpen)
		{
			m_nThumbPosX = m_rcItem.right - m_rcItem.left - m_nThumbWidth;
		}
		else
		{
			m_nThumbPosX = 0;
		}
		Invalidate();
	}

	void CSlideSwitchUI::DoEvent( TEventUI& event )
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( IsEnabled() ) {
				RECT rcThumb = GetThumbRect();
				if( ::PtInRect(&rcThumb, event.ptMouse) ) {
					m_uThumbState |= UISTATE_CAPTURED;
					::SetCapture( m_pManager->GetPaintWindow() );
					m_ptLastMouse = event.ptMouse;
					m_nLastPos = m_nThumbPosX;
#ifdef DEBG_CODE
					TCHAR pThumbPos[128] = {0};
					swprintf(pThumbPos,128,_T("m_nThumbPosX:%d\n"),m_nThumbPosX);
					OutputDebugString(pThumbPos);
#endif
					
				}
			}
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if(IsEnabled())
			{
				if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
					m_uThumbState &= ~UISTATE_CAPTURED;
					::ReleaseCapture();
					m_nTotalPos = 0;
					m_ptLastMouse = event.ptMouse;
					if (IsAnimationRunning(m_dwTimerId))
					{
						StopAnimation(m_dwTimerId);
					}
					if(m_nThumbPosX > (m_rcItem.right - m_rcItem.left - m_nThumbWidth) / 2)
					{
						m_nTotalPos = (m_rcItem.right - m_rcItem.left - m_nThumbWidth) - m_nThumbPosX;
						m_bSwitchOpen = TRUE;
					}
					else if(m_nThumbPosX == m_rcItem.right - m_rcItem.left - m_nThumbWidth)
					{
						;
					}
					else
					{
						m_nTotalPos = m_nThumbPosX;
						m_bSwitchOpen = FALSE;
					}
#ifdef DEBG_CODE
					TCHAR pThumbPos[128] = {0};
					swprintf(pThumbPos,128,_T("m_nThumbPosX:%d\n"),m_nThumbPosX);
					OutputDebugString(pThumbPos);
#endif
					
					StartAnimation(m_nElapseTime, (m_nTotalPos/m_nStep) + 1, m_dwTimerId, m_bLoop);

					m_pManager->SendNotify(this,_T("slideswitch"),DUILIB_SLIDE_SWITCH,(WPARAM)m_bSwitchOpen);
				}
			}		
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_SCROLLWHEEL ) 
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if(IsEnabled())
			{
				if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) 
				{
					INT nOffset = event.ptMouse.x - m_ptLastMouse.x;
					m_nThumbPosX =m_nLastPos + nOffset;
					if(m_nThumbPosX >= 0)
					{
						if(m_nThumbPosX > m_rcItem.right - m_rcItem.left - m_nThumbWidth)
						{
							m_nThumbPosX = m_rcItem.right - m_rcItem.left - m_nThumbWidth;
						}
					}
					else
					{
						m_nThumbPosX = 0;
					}
#ifdef DEBG_CODE
					TCHAR pThumbPos[256] = {0};
					swprintf(pThumbPos,256,_T("UIEVENT_MOUSEMOVE m_nThumbPosX:%d nOffset %d\n"),m_nThumbPosX,nOffset);
					OutputDebugString(pThumbPos);
#endif
				}
				else
				{
					RECT rcThumb = GetThumbRect();
					if( ::PtInRect(&rcThumb, event.ptMouse) )
					{
						m_uThumbState |= UISTATE_HOT;
					}
					else
					{
						m_uThumbState &= ~UISTATE_HOT;
					}
				}
				
			}
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			/*if( IsEnabled() ) 
			{
				RECT rcThumb = GetThumbRect();
				if( ::PtInRect(&rcThumb, event.ptMouse) )
				{
					m_uThumbState |= UISTATE_HOT;
				}
				Invalidate();
			}*/
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) 
			{
				m_uThumbState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_TIMER ) 
		{
			OnTimer(  event.wParam );
		}
		CControlUI::DoEvent(event);
	}

	void CSlideSwitchUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
	{
		if( _tcscmp(pstrName, _T("thumbnormalimage")) == 0 )
		{
			SetThumbNormalImage(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("thumbhotimage")) == 0 )
		{
			SetThumbHotImage(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("thumbpushedimage")) == 0 )
		{
			SetThumbPushedImage(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("bkcloseimage")) == 0 )
		{
			SetBkCloseImage(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("bkopenimage")) == 0 )
		{
			SetBkOpenImage(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("thumbwidth")) == 0 )
		{
			SetThumbWidth(_ttoi(pstrValue));
		}
		else 
		{
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CSlideSwitchUI::DoPaint( HDC hDC, const RECT& rcPaint )
	{
		if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
		PaintBk(hDC);
		PaintThumb(hDC);
	}

	void CSlideSwitchUI::PaintBk( HDC hDC )
	{
		if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~ UISTATE_DISABLED;


		m_sImageModify.Empty();
		//在滑块开和关的状态不需要“source”这个字段，又因为这个是绘图时内部处理，个人以为这里代码去掉就好咯，如果另外从外面的.xml读取，这个样会破坏控件的独立性，不可取
		if(m_nThumbPosX == 0)
		{
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), 0,
				0, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
			if( !m_sBkCloseImage.IsEmpty() )
			{
				int nPos =  m_sBkCloseImage.Find(_T("source"));
				CStdString SzImage;
				if(nPos != -1)
				{
					SzImage.Empty();
					TCHAR c = m_sBkCloseImage[nPos +6];
					if(_tcscmp(&c, _T("=")))
					{
						int nPos2 = m_sBkCloseImage.Find(_T("\'"),nPos + 8);
						if(nPos2 != -1)
						{
							SzImage = m_sBkCloseImage.Left(nPos);
							SzImage.Append(m_sBkCloseImage.Mid(nPos2 + 1));
						}
					}
				}
				if(!SzImage.IsEmpty())
				{
					if( !DrawImage(hDC, (LPCTSTR)SzImage, (LPCTSTR)m_sImageModify))
						SzImage.Empty();
					else return;
				}
			}
		}
		else if(m_nThumbPosX == (m_rcItem.right - m_rcItem.left - m_nThumbWidth))
		{
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), 0,
				0, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
			if( !m_sBkOpenImage.IsEmpty() ) 
			{
				int nPos =  m_sBkOpenImage.Find(_T("source"));
				CStdString SzImage;
				if(nPos != -1)
				{
					SzImage.Empty();
					TCHAR c = m_sBkOpenImage[nPos +6];
					if(_tcscmp(&c, _T("=")))
					{
						int nPos2 = m_sBkOpenImage.Find(_T("\'"),nPos + 8);
						if(nPos2 != -1)
						{
							SzImage = m_sBkOpenImage.Left(nPos);
							SzImage.Append(m_sBkOpenImage.Mid(nPos2 + 1));
						}
					}
				}
				if(!SzImage.IsEmpty())
				{
					if( !DrawImage(hDC, (LPCTSTR)SzImage, (LPCTSTR)m_sImageModify))
						SzImage.Empty();
					else return;
				}
			}
		}
		else
		{
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), 0,
				0, m_nThumbPosX + (m_nThumbWidth / 2), m_rcItem.bottom - m_rcItem.top);

			if( !m_sBkOpenImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sBkOpenImage, (LPCTSTR)m_sImageModify))
					m_sBkOpenImage.Empty();
			}
			m_sImageModify.Empty();
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_nThumbPosX +( m_nThumbWidth / 2),0, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
			if( !m_sBkCloseImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sBkCloseImage, (LPCTSTR)m_sImageModify))
					m_sBkCloseImage.Empty();
			}
		}
	
	}
	void CSlideSwitchUI::PaintThumb( HDC hDC )
	{
		if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~ UISTATE_DISABLED;
		RECT rcThumb = GetThumbRect();
		rcThumb.left -= m_rcItem.left;
		rcThumb.right -= m_rcItem.left;
		rcThumb.top -= m_rcItem.top;
		rcThumb.bottom -= m_rcItem.top;
		m_sImageModify.Empty();
		m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left,rcThumb.top,
			rcThumb.right,rcThumb.bottom);
#ifdef DEBG_CODE
		OutputDebugString(m_sImageModify);
#endif
		if( (m_uThumbState & UISTATE_CAPTURED) != 0 )
		{
			if( !m_sThumbPushedImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify) ) m_sThumbPushedImage.Empty();
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_HOT) != 0 ) 
		{
			if( !m_sThumbHotImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify) ) m_sThumbHotImage.Empty();
				else return;
			}
		}

		if( !m_sThumbNormalImage.IsEmpty() ) 
		{
			if( !DrawImage(hDC, (LPCTSTR)m_sThumbNormalImage, (LPCTSTR)m_sImageModify) ) m_sThumbNormalImage.Empty();
			else return;
		}
	}

	void CSlideSwitchUI::OnAnimationStep( INT nTotalFrame, INT nCurFrame, INT nAnimationID )
	{
		INT nMoveSpace = m_rcItem.right - m_rcItem.left - m_nThumbWidth;
		if(m_nThumbPosX != nMoveSpace && m_nThumbPosX > nMoveSpace / 2)
		{
			m_nThumbPosX += m_nStep;
			if(m_nThumbPosX > (nMoveSpace))
			{
				m_nThumbPosX = nMoveSpace;
			}
		}
		else if(m_nThumbPosX != 0 && m_nThumbPosX <= nMoveSpace /2)
		{
			m_nThumbPosX -= m_nStep;
			if(m_nThumbPosX < 0)
			{
				m_nThumbPosX = 0;
			}
		}
		if(m_nThumbPosX == 0 || m_nThumbPosX == nMoveSpace)
		{
			RECT rcThumb = GetThumbRect();
			if( ::PtInRect(&rcThumb, m_ptLastMouse) )
			{
				m_uThumbState |= UISTATE_HOT;
			}
			else
			{
				m_uThumbState &= ~UISTATE_HOT;
			}
		}
		Invalidate();
	}

	void CSlideSwitchUI::OnTimer( int nTimerID )
	{
		OnAnimationElapse( nTimerID );
	}

}
