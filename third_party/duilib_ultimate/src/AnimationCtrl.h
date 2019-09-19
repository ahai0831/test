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
	
	void HideAnimation(bool bHidden = true);  // ���ض�����bHidden Ϊtrue ,���غ���ʾ
	void ShowAnimation();
	void ResetShowAnimation();       // ���ص�ǰ��ʾ���ݣ�������ʾ

	virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
	virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
	virtual void OnAnimationStop(INT nAnimationID);

	virtual bool SelectItem(int iIndex, bool bVisible = true);    // ��дѡ����Ŀ��ʵ���Զ��仯

	virtual void SetVisible(bool bVisible = true);   // ��д���أ����ȶ�����ɣ�������

	void SetShowElapseTime(int nShowElapse);          // ������ʾʱ����
	void SetHiddenElapseTime(int nHiddenElapse);      // ��������ʱ���� 
	void SetAnimationStep(int nStep);                 // ���ö�������

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	

protected:
	int m_nCurPos;          //��ǰ����������λ��
	int m_nTotalPos;        //�ܳ���
	int m_nLastPos;         //�ϴ��ڵ�λ��
	int m_nStep;            //����
	int m_nElapseTime;      //��ʱ�����ʱ��
	DWORD m_dwTimerId;      //��ʱ��ID����ʾʱ��

	WORD m_dwHiddenTimerId;       // ����ʱ�Ķ�ʱ��ID
	int m_nHiddenElapseTime;      // ����ʱ��ʱ�����ʱ��

	bool m_bResetShow;            // ������ʾ

	bool m_bSelectChanged;        // �Ƿ�Ҫ�ı䵱ǰ��Ŀ
	int m_nIndexSelect;           // Ҫ�ı��ѡ��Ŀ

	bool m_bVisibleControl;        // �Ƿ����ؿؼ�

	bool m_bHiddenControl;         // �ؼ����غ��Ƿ������ʾ 
};

}

