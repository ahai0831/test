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
	int m_nCurPos;          //��ǰ����������λ��
	int m_nTotalPos;        //�ܳ���
	int m_nLastPos;         //�ϴ��ڵ�λ��
	bool m_bLoop;           //�Ƿ�ѭ��
	int m_nStep;            //����
	int m_nElapseTime;      //��ʱ�����ʱ��
	DWORD m_dwTimerId;      //��ʱ��ID
	bool m_bTurnToStart;    //��ѭ��ʱ���Ƿ���Ҫ�ص���ͷ
};

}

