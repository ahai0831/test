#pragma once

namespace DuiLib
{

class UILIB_API CAnimationBar :public CHorizontalLayoutUI, public CUIAnimation
{
public:
	CAnimationBar(void);
	~CAnimationBar(void);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);

	void DoEvent(TEventUI& event);
	void OnTimer( int nTimerID );

	void ScrollPos();

	virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
	virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
	virtual void OnAnimationStop(INT nAnimationID);

protected:
	int m_nCurPos;          //当前滑动条所在位置
	int m_nTotalPos;        //总长度
	int m_nLastPos;         //上次在的位置
	bool m_bLoop;           //是否循环
	int m_nStep;            //步长
	int m_nElapseTime;      //定时器间隔时间
	DWORD m_dwTimerId;      //定时器ID
	bool m_bTurnToStart;    //不循环时，是否需要回到开头
};

}

