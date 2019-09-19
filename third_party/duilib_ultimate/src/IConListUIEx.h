#ifndef __ICON_UILISTEX_H__
#define __ICON_UILISTEX_H__
#pragma once

namespace DuiLib {

#define  NUM_LEFT       0          //ǰ��Ԥ������������������
class CIConListContainerElementUIEx;
class CIConPhotoBackupListContainerElementUI;
//����Դ�ӿ�
class UILIB_API IListDataSource
{
public:
	virtual ~IListDataSource() { NULL; }

	// ��ȡ�б�����
	virtual int numberOfRows(/*CListUI *pList*/) = 0;
	// ����������ȡ�б��Ӧitem
	virtual CControlUI *listItemForIndex(/*CListUI *pList,*/ int index) = 0;
	// ��ȡ�б�ͷ
	virtual CListHeaderUI *listHeader(/*CListUI *pList*/) = 0;
	 
	virtual void SelectItem(int index){}
	virtual void Expand(int index){}

	virtual int CurSelectItem(){ return -1; }

	virtual bool isSelected(int index){ return FALSE; }
	virtual bool isExpand(int index){ return FALSE; }

	virtual void UnSelectAllItems(){};
};


//���������ݹ����б�
/////////////////////////////////////////////////////////////////////////////////////
//
class CMoveVerticalLayoutUI : public CVerticalLayoutUI
{

public:
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetOwner(CControlUI *pOwner);
	POINT m_point;
protected:
	CControlUI *m_pOwner;

};

class UILIB_API CMoveItemWnd : public CWindowWnd
{
public:
	void Init(CControlUI* pOwner);
	LPCTSTR GetWindowClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void MoveWindowFollowPoint(POINT pt);
	
public:
	CPaintManagerUI m_pm;
	CControlUI* m_pOwner;
	CMoveVerticalLayoutUI* m_pLayout;
	POINT m_point;
};

class CIConListBodyUIEx;

class UILIB_API CIConListUIEx : public CVerticalLayoutUI, public IListUI
{
	friend class CMoveItemWnd;
public:
	CIConListUIEx();
	~CIConListUIEx(void);

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	CControlUI* GetItemExAt(int iIndex) const; //��ȡָ����������
	int GetAllCount() const;                   //��ȡ������ 

	bool GetScrollSelect();
	void SetScrollSelect(bool bScrollSelect);
	int GetCurSel() const;

	CContainerUI* GetList() const;
	TListInfoUI* GetListInfo();

	CListHeaderUI* GetHeader() const;

	CControlUI* GetItemAt(int iIndex) const;
	int GetItemIndex(CControlUI* pControl) const;
	bool SetItemIndex(CControlUI* pControl, int iIndex);
	int GetCount() const;
	bool Add(CControlUI* pControl);
	bool AddAt(CControlUI* pControl, int iIndex);
	bool Remove(CControlUI* pControl);
	bool RemoveAt(int iIndex);
	void RemoveAll();

	void Scroll(int dx, int dy);

	int GetChildPadding() const;
	void SetChildPadding(int iPadding);

	void SetItemFont(int index);
	void SetItemTextStyle(UINT uStyle);
	void SetItemTextPadding(RECT rc);
	void SetItemTextColor(DWORD dwTextColor);
	void SetItemBkColor(DWORD dwBkColor);
	void SetItemBkImage(LPCTSTR pStrImage);
	void SetAlternateBk(bool bAlternateBk);
	void SetSelectedItemTextColor(DWORD dwTextColor);
	void SetSelectedItemBkColor(DWORD dwBkColor);
	void SetSelectedItemImage(LPCTSTR pStrImage); 
	void SetHotItemTextColor(DWORD dwTextColor);
	void SetHotItemBkColor(DWORD dwBkColor);
	void SetHotItemImage(LPCTSTR pStrImage);
	void SetDisabledItemTextColor(DWORD dwTextColor);
	void SetDisabledItemBkColor(DWORD dwBkColor);
	void SetDisabledItemImage(LPCTSTR pStrImage);
	void SetItemLineColor(DWORD dwLineColor);
	bool IsItemShowHtml();
	void SetItemShowHtml(bool bShowHtml = true);
	RECT GetItemTextPadding() const;
	DWORD GetItemTextColor() const;
	DWORD GetItemBkColor() const;
	LPCTSTR GetItemBkImage() const;
	bool IsAlternateBk() const;
	DWORD GetSelectedItemTextColor() const;
	DWORD GetSelectedItemBkColor() const;
	LPCTSTR GetSelectedItemImage() const;
	DWORD GetHotItemTextColor() const;
	DWORD GetHotItemBkColor() const;
	LPCTSTR GetHotItemImage() const;
	DWORD GetDisabledItemTextColor() const;
	DWORD GetDisabledItemBkColor() const;
	LPCTSTR GetDisabledItemImage() const;
	DWORD GetItemLineColor() const;

	void SetMoveSelectImage(LPCTSTR pStrImage);
	LPCTSTR GetMoveSelectImage();

	void SetMultiExpanding(bool bMultiExpandable); 
	int GetExpandedItem() const;
	bool ExpandItem(int iIndex, bool bExpand = true);
	void SetEditIndex(int iIndex);
	int GetEditiIndex();

	void SetPos(RECT rc);
	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void PaintSelectItem(HDC hDC, const RECT &rcPaint);
	void PaintText(HDC hDC);

	IListCallbackUI* GetTextCallback() const;
	void SetTextCallback(IListCallbackUI* pCallback);

	SIZE GetScrollPos() const;
	SIZE GetScrollRange() const;
	void SetScrollPos(SIZE szPos);
	void LineUp();
	void LineDown();
	void PageUp();
	void PageDown();
	void HomeUp();
	void EndDown();
	void LineLeft();
	void LineRight();
	void PageLeft();
	void PageRight();
	void HomeLeft();
	void EndRight();
	void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
	virtual CScrollBarUI* GetVerticalScrollBar() const;
	virtual CScrollBarUI* GetHorizontalScrollBar() const;

	//add by lighten
	void EnsureVisible(int iIndex);      //��ʾָ������
	void SetItemHeight(int nItemHeigh);    //������߶�
	void SetItemWidth(int nItemWidth);    //��������

	int  GetItemHeight() const;            //��ȡ��߶�
	void SetItemCount(int nNums);          //����������
	int  GetItemCount() const;            //��ȡ������
	int  GetTopIndex() const;             //��ȡ��������
	int  GetShowItems() const;            //��ȡ������ʾ��Item��
	void AddSelItem(int nIndex);          // ���ѡ����Ŀ����ˢ��
	void EmptySelItem();                  // ���ѡ����Ŀ����ˢ�� 

	CIConListContainerElementUIEx * GetReuseableItem();  //��ȡ�ɸ��õ���

	void CalShowItemNum();       //���������ʾ������

	void SetDataSource(IListDataSource *ListDataSource);   // ��������Դ�������κν������
	void SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex = true);    //����������Դ��ˢ��

	bool IsHiddenItemInTop();             //��ѯ�Ƿ�������������ǰ��
	bool IsHiddenItemInBottom();          //�ڵײ��Ƿ������ص���

	bool ScrollDownLine();     //������һ��
	bool ScrollUpLine();       //������һ��
	bool ScrollDown();
	bool RefreshListData(int nTopIndex, bool bInit = true);    //����ָ������ˢ������
	bool RefreshListDatabyLine(int nLines); //����ָ����ˢ������
	bool RefreshPos(int nTopIndex, bool bInit = true);         //����ָ������ˢ��pos������
	bool DeleteIndexData(int Index);        //ɾ��ָ��������
	bool InsetIndexData(int Index);         //���ƶ�����֮ǰ��������
	bool PushBackData(int nIndex);          // �������

	//add by lighten
	bool SelectItem(int iIndex, bool bTakeFocus = false);                //ѡ��һ��
	bool SelectMultiItem(int iIndex, bool bTakeFocus = false, bool bSendMessage = true);           //��Ӷ�һ�ѡ��
	bool SelectMultiItembyDrag(int iIndex, bool bTakeFocus = false);           //��Ӷ�һ�ѡ��
	bool SetItemSelect(int nStart, int nEnd, bool bTakeFocus = false);   //ѡ��ָ����Χ����
	void SetSingleSelect(bool bSingleSel);
	bool GetSingleSelect() const;
	bool UnSelectItem(int iIndex,bool bSendMessage = true);
	void SelectAllItems();
	void UnSelectAllItems();
	int GetSelectItemCount() const;
	int GetNextSelItem(int nItem) const;
	int SetFocusItem(int nItem, bool bTakeFocus = false);   //���ý���������
	int SetEndItem(int nItem, bool bTakeFocus = false);     //���ö�ѡ�н�����
	int GetEndItem() const;
	bool SetLButtonState(bool bDown);   //��������Ƿ񱻰���
	bool GetLButtonState() const; 
	void SetStartPoint(POINT pt);
	bool IsListDataChanged();   // �жϵ�ǰ�����Ƿ��޸�

	void UnSelectAllItems(bool bSendMessage);
	CStdPtrArray GetSelArray() const;       //��ȡѡ�е�����

	void SelectbyDrag(bool bTakeFocus = false);       //�϶�ѡ��
	bool SelectMoveTemp(int iIndex);            //�϶�ʱ��ѡ�����Ŀ
	bool UnSelectMoveTemp();            //�϶�ʱ��ȡ��ѡ�����Ŀ
	void CloseMoveWindow();                       //�ر��ƶ�����
	
	void SetMoveItemEffect(bool bMoveItem = true);      //�����Ƿ���Ҫ�ƶ�Ч��
	bool IsMoveItemEffect() const;                     //�����ƶ�Ч����ֵ
	bool NeedFreshData(bool bFRresh = true);    //�����Ƿ���Ҫˢ������

	int MoveToPoint(POINT &pt);     //�ƶ���ָ���ĵ�
	int FindIndexbyPoint(POINT &pt);   //ͨ��ָ���ĵ��ҵ���Ӧ������

	bool IsMovedItem() const;       //�Ƿ��Ѿ��ƶ���ѡ����

	void SetAllNeedHeight(int nHeight);

	void SetAddItemStyle(bool bAddItem = false);      //�����Ƿ��ж�������ѡ��Ŀ
	void SetVerticalScrollPos(int nPos);

	//www 2014.07.16
	bool IsSelectTopIndex();
protected:
	void Init(bool bRefreshSize = true);               //��ʼ������
	int GetMinSelItemIndex();
	int GetMaxSelItemIndex();

	void CalStartPoint();      //���㿪ʼ�϶�ѡ��ʱ��״̬
	void CalEndPoint();      //��������϶�ѡ��ʱ��״̬

protected:
	bool m_bScrollSelect;
	/*	int m_iCurSel;*/
	int m_iExpandedItem;
	IListCallbackUI* m_pCallback;
	CIConListBodyUIEx* m_pList;
	TListInfoUI m_ListInfo;

	//add by lighten 	
	int m_nTopIndex;        //λ�ڶ�����������
	int m_nHorIndex;       //��������
	int m_nItemHeight;      //��߶�
	int m_nItemWidth;
	int m_nItems;           //������
	int m_nShowItems;   //������ʾ������
	int m_nShowItemsInLine;    //�п�����ʾ������
	int m_nShowItemsInColumn;  //�п�����ʾ������
	int m_nShowNeedLines;      //��ʾ��������Ҫ������ 
	int m_nShowNeedHeights;    //��ʾ��Ŀ��Ҫ��������������


	CStdPtrArray m_dequeueReusableItem;   //�ɸ��õ���
	IListDataSource *m_pListDataSource;   //������Դ

	bool m_bSingleSel;                   //�Ƿ�ѡ��Ĭ�϶�ѡ   
	CStdPtrArray m_aSelItems;            //����ѡ�е���
	int m_nFocusItem;
	int m_nStartSelItem;                 //��ѡ��ʼ��
	int m_nEndSelItem;
	bool m_bLButtonDown;
	bool m_bItemLButtonDown;
	SIZE m_StartPos;       //��ʼ���ڵ�pos
	POINT m_pStartPoint;   //��ʼ����
	POINT m_pEndPoint;     //��������
	POINT m_pTempPoint;    //�м�ڵ�
	UINT  m_nStartLine;    //��ʼѡ��ʱ������
	UINT  m_nStartColumn;  //��ʼѡ��ʱ������
	UINT  m_nEndLine;    //����ѡ��ʱ������
	UINT  m_nEndColumn;  //����ѡ��ʱ������
	bool m_bMoveSelect;

	CMoveItemWnd *m_pMoveItem;   //�ƶ�ʱ�Ĵ���
	bool m_bMoveItemEffect;            //�Ƿ���Ҫ�ƶ���קЧ��
	bool m_bNeedRefresh;               //�Ƿ���Ҫˢ������
	UINT m_uMoveSelectIndex;           //�ƶ���ѡ�����
	bool m_bHasMoveItem;               //�ƶ���ѡ�е���Ŀ
	CStdString m_strMoveSelectImage;   //��ѡͼ

	bool m_bFreshed;       //�Ѿ�ˢ��

	//���б�ʱ���ı�����
	DWORD m_dwTextColor;
	DWORD m_dwDisabledTextColor;
	int m_iFont;
	UINT m_uTextStyle;
	RECT m_rcTextPadding;
	bool m_bShowHtml;

	// ������ʽ�����һ��Ϊ��Ӱ�ťѡ��
	bool m_bStyleAddItem; 

	bool m_bSelTopIndex;		//�Ƿ�ѡ�ж��������0��

	bool m_bNeedCheckListData;   // �Ƿ���Ҫ��鵱ǰ����Դ�����Ƿ�ı�
	int  m_nEditIndex;
};

class UILIB_API CIConPhotoBackupListUI : public CIConListUIEx
{
public:
	CIConPhotoBackupListUI();
	~CIConPhotoBackupListUI(void);

	void AddSelItemIndex(int Index);
	void ReMoveSelItemIndex(int Index);

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	void DoEvent(TEventUI& event);
	//add by lighten
	//bool SetItemSelect(int nStart, int nEnd, bool bTakeFocus = false);   //ѡ��ָ����Χ����
};
/////////////////////////////////////////////////////////////////////////////////////
//
class UILIB_API CGridLayoutUI :public CTileLayoutUI
{
public:
	CGridLayoutUI();
	~CGridLayoutUI();

	void SetPos(RECT rc);
	
};

//////////////////////////////////////////////////////////////////////////
//
class UILIB_API CIConListBodyUIEx : public CGridLayoutUI
{
public:
	CIConListBodyUIEx(CIConListUIEx* pOwner);
	LPCTSTR GetClass();

	void SetScrollPos(SIZE szPos);
	void SetScrollLinePos(SIZE szPos, bool bScrollItem = true);     //����һ�е�pos����
	void FreshDataBySize(SIZE szPos);     //λ�ñ仯ʱ��ˢ�½�������
	void SetPos(RECT rc);
	void DoEvent(TEventUI& event);

	void SetVerticalScrollPos(int nPos);
	int  GetVerticalFixWidth();
	void setUp(bool up);
	
	void LineDown();
	void LineUp();
	void PageUp();
	void PageDown();
	void HomeUp();
	void EndDown();

	bool IsHiddenItemInTop();          //��ͷ���Ƿ������ص���
	bool IsHiddenItemInBottom();          //�ڵײ��Ƿ������ص���
	void SetLeavePos(int nLeavePos);      //����Ϊ��ʾȫ�������ص����ش�С

protected:
	CIConListUIEx* m_pOwner;
	int   m_nLeavePos;            // ������ʾʱ����ȱ������
	bool  m_bUp;                  // ���Ϲ���
	bool  m_bPosChange;           // ��С�ı� 
	int   m_nPosOffset;           // ��С�ı���Ҫ�ƶ��Ĵ�С
};

class UILIB_API CIConListContainerElementUIEx : public CContainerUI, public IListItemUI
{
public:
	CIConListContainerElementUIEx();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	int GetIndex() const;
	void SetIndex(int iIndex);
	void SetbVisibleEdit(bool bVisible);
	void SetbVisibleEdit(bool bVisible, bool bIsFolder);


	CIConListUIEx* GetOwner();
	void SetOwner(CControlUI* pOwner);
	void SetVisible(bool bVisible = true);
	void SetEnabled(bool bEnable = true);

	bool IsSelected() const;
	bool Select(bool bSelect = true, bool bInvalidate = true);
	bool IsExpanded() const;
	bool Expand(bool bExpand = true);

	void Invalidate(); // ֱ��CControl::Invalidate�ᵼ�¹�����ˢ�£���д����ˢ������
	bool Activate();

	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);    
	void DrawItemBk(HDC hDC, const RECT& rcItem);
	void PaintSelect(HDC hDC, const RECT&rcItem);

	void SetShade(bool bShade = false);
	bool GetShade() const;
	void SetShadeColor(DWORD dwShadeColor);
	DWORD GetShadeColor() const;
	void SetCanSel(bool bCanSel = true);
	bool GetCanSel() const;

	void SetShadeImage(CStdString strImage);
	LPCTSTR GetShadeImage() const;

	void SetNormalImage(CStdString strImage);
	LPCTSTR GetNormalImage() const;
	void SetHotImage(CStdString strImage);
	LPCTSTR GetHotImage() const;
	void SetSelectImage(CStdString strImage);
	LPCTSTR GetSelectImage() const;

protected:
	int m_iIndex;
	bool m_bSelected;
	UINT m_uButtonState;
	CIConListUIEx* m_pOwner;
	bool m_bNeedSelect;
	bool m_bvisibleEdit;

	bool m_bShade;   //��Ӱ����
	DWORD m_dwShadeColor;
	CStdString m_strShadeImage;
	bool m_bCanSel;
	CStdString m_strNormalImage;
	CStdString m_strHotImage;
	CStdString m_strSelectImage;

};


class UILIB_API CIConPhotoBackupListContainerElementUI : public CIConListContainerElementUIEx
{
public:
	CIConPhotoBackupListContainerElementUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	void DoEvent(TEventUI& event);


protected:
};

}  // namespace DuiLib

#endif  // __ICON_UILISTEX_H__