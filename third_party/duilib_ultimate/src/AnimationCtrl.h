#pragma once

namespace DuiLib
{

class UILIB_API CAnimationCtrl :public CTabLayoutUI, public CUIAnimation
{
public:
	CAnimationCtrl(void);
	virtual ~CAnimationCtrl(void);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);

	void DoEvent(TEventUI& event);
	void OnTimer( int nTimerID );

	void ScrollPos();
	
	void HideAnimation(bool bHidden = true);  // 隐藏动画，bHidden 为true ,隐藏后不显示
	void ShowAnimation();
	void ResetShowAnimation();       // 隐藏当前显示内容，重新显示

	virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
	virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
	virtual void OnAnimationStop(INT nAnimationID);

	virtual bool SelectItem(int iIndex, bool bVisible = true);    // 改写选中项目，实现自动变化

	virtual void SetVisible(bool bVisible = true);   // 改写隐藏，需先动画完成，再隐藏

	void SetShowElapseTime(int nShowElapse);          // 设置显示时间间隔
	void SetHiddenElapseTime(int nHiddenElapse);      // 设置隐藏时间间隔 
	void SetAnimationStep(int nStep);                 // 设置动画步长

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	

protected:
	int m_nCurPos;          //当前滑动条所在位置
	int m_nTotalPos;        //总长度
	int m_nLastPos;         //上次在的位置
	int m_nStep;            //步长
	int m_nElapseTime;      //定时器间隔时间
	DWORD m_dwTimerId;      //定时器ID（显示时）

	WORD m_dwHiddenTimerId;       // 隐藏时的定时器ID
	int m_nHiddenElapseTime;      // 隐藏时定时器间隔时间

	bool m_bResetShow;            // 重新显示

	bool m_bSelectChanged;        // 是否要改变当前项目
	int m_nIndexSelect;           // 要改变的选项目

	bool m_bVisibleControl;        // 是否隐藏控件

	bool m_bHiddenControl;         // 控件隐藏后，是否继续显示 
};

}

