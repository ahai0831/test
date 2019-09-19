#ifndef __UIANIMATIONTABLAYOUTUI_H__
#define __UIANIMATIONTABLAYOUTUI_H__

#pragma once

namespace DuiLib {

class UILIB_API CAnimationTabLayoutUI : public CTabLayoutUI, public CUIAnimation
{
public:
	CAnimationTabLayoutUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	bool Add(CControlUI* pControl);
	bool AddAt(CControlUI* pControl, int iIndex);
	bool SelectItem( int iIndex );
	void AnimationSwitch();
	void DoEvent(TEventUI& event);
	void OnTimer( int nTimerID );

	virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
	virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
	virtual void OnAnimationStop(INT nAnimationID);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
	bool m_bIsVerticalDirection;
	int m_nPositiveDirection;
	RECT m_rcCurPos;
	RECT m_rcItemOld;
	RECT m_rcLastItemPos;
	int m_iOldSel;
	CControlUI* m_pCurrentControl;
	CControlUI* m_pLastControl;
	bool m_bControlVisibleFlag;
	enum
	{
		TAB_ANIMATION_ID = 1,

		TAB_ANIMATION_ELLAPSE = 10,
		TAB_ANIMATION_FRAME_COUNT = 15,
	};
};

}// namespace DuiLib
#endif   //__UIANIMATIONTABLAYOUTUI_H__
