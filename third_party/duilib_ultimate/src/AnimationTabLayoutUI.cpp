#include "stdafx.h"
namespace DuiLib {

	CAnimationTabLayoutUI::CAnimationTabLayoutUI() : 
CUIAnimation( this ), 
m_bIsVerticalDirection( false ), 
m_nPositiveDirection( 1 ),
m_pCurrentControl( NULL ),
m_pLastControl(NULL),
m_iOldSel(0),
m_bControlVisibleFlag( false )
{
}

LPCTSTR CAnimationTabLayoutUI::GetClass() const
{
	return _T("AnimationTabLayoutUI");
}

LPVOID CAnimationTabLayoutUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("AnimationTabLayoutUI")) == 0 ) 
		return static_cast<CAnimationTabLayoutUI*>(this);
	return CTabLayoutUI::GetInterface(pstrName);
}

bool CAnimationTabLayoutUI::Add(CControlUI* pControl)
{
	bool ret = CContainerUI::Add(pControl);
	if( !ret ) return ret;

	if(m_iCurSel == -1 && pControl->IsVisible())
	{
		m_iCurSel = GetItemIndex(pControl);
	}
	else
	{
		//pControl->SetVisible(false);
		
	}

	return ret;
}

bool CAnimationTabLayoutUI::AddAt(CControlUI* pControl, int iIndex)
{
	bool ret = CContainerUI::AddAt(pControl, iIndex);
	if( !ret ) return ret;

	if(m_iCurSel == -1 && pControl->IsVisible())
	{
		m_iCurSel = GetItemIndex(pControl);
	}
	else if( m_iCurSel != -1 && iIndex <= m_iCurSel )
	{
		m_iCurSel += 1;
	}
	else
	{
		//pControl->SetVisible(false);
	}

	return ret;
}

bool CAnimationTabLayoutUI::SelectItem( int iIndex )
{
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return false;
	if( iIndex == m_iCurSel ) return true;
	if( iIndex > m_iCurSel ) m_nPositiveDirection = -1;
	if( iIndex < m_iCurSel ) m_nPositiveDirection = 1;

	if(m_pCurrentControl != NULL)
	{
		m_pLastControl = m_pCurrentControl;
	}
	else
	{
		m_pLastControl = static_cast<CControlUI*>(m_items[m_iCurSel]);   // 如果第一个不保证，会导致初始化和下一个选择的页面混合
	}
	int m_iOldSel = m_iCurSel;
	m_iCurSel = iIndex;
	for( int it = 0; it < m_items.GetSize(); it++ )
	{
		if( it == iIndex ) {
			//GetItemAt(it)->SetVisible(true);
			GetItemAt(it)->SetFocus();

			m_bControlVisibleFlag = false;
			m_pCurrentControl = static_cast<CControlUI*>(m_items[it]);
			/*if( NULL != m_pCurrentControl )
				m_pCurrentControl->SetVisible( false );*/
			AnimationSwitch();
		}
		else if(m_pLastControl == static_cast<CControlUI*>(m_items[it]))
		{
			m_pLastControl->SetPos(m_rcItem); // 把上一张图移动到完全显示的位置
		}
		else 
		{
			//把不需要显示的隐藏起来
			RECT rc = {0,0,0,0};
			GetItemAt(it)->SetPos(rc);
		}
	}
	//	NeedParentUpdate();
	if( m_pManager != NULL ) {
		m_pManager->SetNextTabControl();
		m_pManager->SendNotify(this, _T("tabselect"), DUILIB_TAB_SELECT, m_iCurSel, m_iOldSel);
	}

	return true;
}

void CAnimationTabLayoutUI::AnimationSwitch()
{
	m_rcLastItemPos = m_rcItemOld = m_rcItem;
	if( !m_bIsVerticalDirection )
	{
		m_rcCurPos.top = m_rcItem.top;
		m_rcCurPos.bottom = m_rcItem.bottom;
		m_rcCurPos.left = m_rcItem.left - ( m_rcItem.right - m_rcItem.left ) * m_nPositiveDirection /*+ 52 * m_nPositiveDirection*/;
		m_rcCurPos.right = m_rcItem.right - ( m_rcItem.right - m_rcItem.left ) * m_nPositiveDirection/*+ 52 * m_nPositiveDirection*/;
	}
	else
	{
		m_rcCurPos.left = m_rcItem.left;
		m_rcCurPos.right = m_rcItem.right;
		m_rcCurPos.top = m_rcItem.top - ( m_rcItem.bottom - m_rcItem.top ) * m_nPositiveDirection;
		m_rcCurPos.bottom = m_rcItem.bottom - ( m_rcItem.bottom - m_rcItem.top ) * m_nPositiveDirection;		
	}

	StopAnimation( TAB_ANIMATION_ID );
	StartAnimation( TAB_ANIMATION_ELLAPSE, TAB_ANIMATION_FRAME_COUNT, TAB_ANIMATION_ID );
}

void CAnimationTabLayoutUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_TIMER ) 
	{
		OnTimer(  event.wParam );
	}
	__super::DoEvent( event );
}

void CAnimationTabLayoutUI::OnTimer( int nTimerID )
{
	OnAnimationElapse( nTimerID );
}

void CAnimationTabLayoutUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
{
	if(m_pLastControl == NULL)
	{
		m_pCurrentControl->SetPos(m_rcItemOld);
		return;
	}
	int iStepLen = 0;
	if( !m_bIsVerticalDirection )
	{
		iStepLen = ( m_rcItemOld.right - m_rcItemOld.left ) * m_nPositiveDirection / nTotalFrame;
		if( nCurFrame != nTotalFrame )
		{
			m_rcCurPos.left = m_rcCurPos.left + iStepLen;
			m_rcCurPos.right = m_rcCurPos.right +iStepLen ;
			m_rcLastItemPos.left = m_rcLastItemPos.left + iStepLen;
			m_rcLastItemPos.right = m_rcLastItemPos.right + iStepLen;
		}
		else
		{
			m_rcLastItemPos = m_rcItem = m_rcCurPos = m_rcItemOld;
			m_rcLastItemPos.left += (m_rcItemOld.right - m_rcItemOld.left) * m_nPositiveDirection;
			m_rcLastItemPos.right += (m_rcItemOld.right - m_rcItemOld.left) * m_nPositiveDirection;
			//这里不能把上一个Tab区域设置（0，0，0，0）
			//memset(&m_rcLastItemPos,0,sizeof(m_rcLastItemPos));
			m_pCurrentControl->SetPos(m_rcItem);
			m_pLastControl->SetPos(m_rcLastItemPos);
			return;
		}
	}
	else
	{
		iStepLen = ( m_rcItemOld.bottom - m_rcItemOld.top ) * m_nPositiveDirection / nTotalFrame;
		if( nCurFrame != nTotalFrame )
		{
			m_rcCurPos.top = m_rcCurPos.top + iStepLen;
			m_rcCurPos.bottom = m_rcCurPos.bottom +iStepLen;
			m_rcLastItemPos.top = m_rcLastItemPos.top + iStepLen;
			m_rcLastItemPos.bottom = m_rcLastItemPos.bottom + iStepLen;
		}
		else
		{
			m_rcLastItemPos = m_rcItem = m_rcCurPos = m_rcItemOld;
			m_rcLastItemPos.top += (m_rcItemOld.bottom - m_rcItemOld.top) * m_nPositiveDirection;
			m_rcLastItemPos.bottom += (m_rcItemOld.bottom - m_rcItemOld.top) * m_nPositiveDirection;
			//这里不能把上一个Tab区域设置（0，0，0，0）
			//memset(&m_rcLastItemPos,0,sizeof(m_rcLastItemPos));
			m_pCurrentControl->SetPos(m_rcItem);
			m_pLastControl->SetPos(m_rcLastItemPos);
			
			return;
		}	
	}
	m_pCurrentControl->SetPos(m_rcCurPos);
	m_pLastControl->SetPos(m_rcLastItemPos);

	/*if( !m_bControlVisibleFlag )
	{
		m_bControlVisibleFlag = true;
	}*/

}

void CAnimationTabLayoutUI::OnAnimationStop(INT nAnimationID) 
{
	NeedParentUpdate();
}

void CAnimationTabLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("animation_direction")) == 0 && _tcscmp( pstrValue, _T("vertical")) == 0 )
		m_bIsVerticalDirection = true; // pstrValue = "vertical" or "horizontal"

	return CTabLayoutUI::SetAttribute(pstrName, pstrValue);
}

}