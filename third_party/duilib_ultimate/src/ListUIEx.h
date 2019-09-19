#ifndef __UILISTEX_H__
#define __UILISTEX_H__
#pragma once

namespace DuiLib {

#define  NUM_LEFT       0          //ǰ��Ԥ������������������
class CListContainerElementUIEx;
//���������ݹ����б�
/////////////////////////////////////////////////////////////////////////////////////
//

class CListBodyUIEx;
class CListHeaderUI;

class UILIB_API CListUIEx : public CVerticalLayoutUI, public IListUI
{
	friend class CMoveItemWnd;
public:
	CListUIEx();
	~CListUIEx(void);

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	CControlUI* GetItemExAt(int iIndex) const; //��ȡָ����������
	int GetAllCount() const;                   //��ȡ������ 

	bool GetScrollSelect();
	void SetScrollSelect(bool bScrollSelect);
	int GetCurSel() const;

	CListHeaderUI* GetHeader() const;  
	CContainerUI* GetList() const;
	TListInfoUI* GetListInfo();

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

	void SetEmptyImage(LPCTSTR pStrImage);
	LPCTSTR GetEmptyImage();
	void PaintEmptyImage(bool bPaint = true);

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
	void PaintEmptyImage(HDC hDC);   // ����Ϊ��ʱͼƬ

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
	int  GetItemHeight() const;            //��ȡ��߶�
	void SetItemCount(int nNums);          //����������
	int  GetItemCount() const;            //��ȡ������
	int  GetTopIndex() const;             //��ȡ��������
	int  GetShowItems() const;            //��ȡ������ʾ��Item��
	int  GetListDataSize();               // ��ȡ����Դ��ǰ��С
	void AddSelItem(int nIndex);          // ���ѡ����Ŀ����ˢ��
	void EmptySelItem();                  // ���ѡ����Ŀ����ˢ�� 


	CListContainerElementUIEx * GetReuseableItem();  //��ȡ�ɸ��õ���

	void CalShowItemNum();       //���������ʾ������

	void SetDataSource(IListDataSource *ListDataSource);   // ��������Դ�������κν������
	void SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex = true);    //����������Դ��ˢ��

	bool IsHiddenItemInTop();             //��ѯ�Ƿ�������������ǰ��
	bool IsHiddenItemInBottom();          //�ڵײ��Ƿ������ص���

	bool ScrollDownLine();     //������һ��
	UINT ScrollUpLine();       //������һ��
	bool ScrollDown();
	bool RefreshListData(int nTopIndex, bool bInit = true);    //����ָ������ˢ������
	bool RefreshPos(int nTopIndex, bool bInit = true);         //����ָ������ˢ��pos������
	bool DeleteIndexData(int Index);        //ɾ��ָ��������
	bool InsetIndexData(int Index);         //��ָ������֮ǰ��������
	bool InsetDataAt(int nIndex);           //��ָ������֮ǰ��������(��ˢ��ȫ�����ݣ�
	bool DeleteDataAt(int nIndex);          //ɾ��ָ��������(��ˢ��ȫ�����ݣ�
	bool RefreshData(CStdPtrArray & deleteList, CStdPtrArray & InsertList);    // ˢ�����ݣ�����ѡ����

	//add by lighten
	bool SelectItem(int iIndex, bool bTakeFocus = false);                //ѡ��һ��
	bool SelectMultiItem(int iIndex, bool bTakeFocus = false, bool bSendMessage = true);           //��Ӷ�һ�ѡ��
	bool SetItemSelect(int nStart, int nEnd, bool bTakeFocus = false);   //ѡ��ָ����Χ����
	void SetSingleSelect(bool bSingleSel);
	bool GetSingleSelect() const;
	bool UnSelectItem(int iIndex, bool bSendMessage = true);
	void SelectAllItems();
	void UnSelectAllItems();
	int GetSelectItemCount() const;
	int GetNextSelItem(int nItem) const;
	int SetFocusItem(int nItem, bool bTakeFocus = false);   //���ý���������
	int SetEndItem(int nItem, bool bTakeFocus = false);     //���ö�ѡ�н�����
	int GetEndItem() const;
	bool SetLButtonState(bool bDown);   //��������Ƿ񱻰���
	bool GetLButtonState() const; 
	bool SetLButton_ClickData(bool bDown);   //��������Ƿ�������������
	bool GetLButtonClickData() const; 
	void SetStartPoint(POINT pt);   //���ÿ�ʼ���
	bool IsMovedItem() const;       //�Ƿ��Ѿ��ƶ���ѡ����
	bool IsListDataChanged();   // �жϵ�ǰ�����Ƿ��޸�

	void UnSelectAllItems(bool bSendMessage);

	CStdPtrArray GetSelArray() const;       //��ȡѡ�е�����

	bool SelectMoveTemp(int iIndex);            //�϶�ʱ��ѡ�����Ŀ
	bool UnSelectMoveTemp();            //�϶�ʱ��ȡ��ѡ�����Ŀ
	void CloseMoveWindow();                       //�ر��ƶ�����
	
	void SetMoveItemEffect(bool bMoveItem = true);      //�����Ƿ���Ҫ�ƶ�Ч��
	bool IsMoveItemEffect() const;                     //�����ƶ�Ч����ֵ
	
	bool NeedFreshData(bool bFRresh = true);    //�����Ƿ���Ҫˢ������

	int MoveToPoint(POINT &pt);     //�ƶ���ָ���ĵ�
	int FindIndexbyPoint(POINT &pt);   //ͨ��ָ���ĵ��ҵ���Ӧ������
	void SetAllNeedHeight(int nHeight);
	void SetVerticalScrollPos(int nPos);

	//www 2014.07.16
	bool IsSelectTopIndex();

	void SetToHeaderHeight(int nHeight);
	int  GetToHeaderHeight();
	void SetAddItemStyle(bool bAddItem = false);      //�����Ƿ��ж�������ѡ��Ŀ
	void SetWindowsStyle(bool bStyle = false);
	bool GetWindowsStyle() const;
	void SetLeftWidth(int nWidth);
	int  GetLeftWidth() const;
protected:
	void Init();               //��ʼ������
	int GetMinSelItemIndex();
	int GetMaxSelItemIndex();

protected:
	bool m_bScrollSelect;
/*	int m_iCurSel;*/
	int m_iExpandedItem;
	IListCallbackUI* m_pCallback;
	CListBodyUIEx* m_pList;
	CListHeaderUI* m_pHeader;
	TListInfoUI m_ListInfo;

	//add by lighten 	
	int m_nTopIndex;        //λ�ڶ�����������
	int m_nItemHeight;      //��߶�
	int m_nItems;           //������
	int m_nShowItems;   //������ʾ������

	CStdPtrArray m_dequeueReusableItem;   //�ɸ��õ���
	IListDataSource *m_pListDataSource;   //������Դ

	bool m_bSingleSel;                   //�Ƿ�ѡ��Ĭ�϶�ѡ   
	CStdPtrArray m_aSelItems;            //����ѡ�е���
	int m_nFocusItem;
	int m_nStartSelItem;                 //��ѡ��ʼ��
	int m_nEndSelItem;
	bool m_bLButtonDown;                  //����ڷ��������� 
	bool m_bLButtonDown_ClickInData;            //�������������
	POINT m_pStartPoint;   //��ʼ����
	POINT m_pEndPoint;     //��������
	POINT m_pTempPoint;    //�м�ڵ�
	SIZE m_StartPos;       //��ʼ���ڵ�pos

	CMoveItemWnd *m_pMoveItem;   //�ƶ�ʱ�Ĵ���
	bool m_bMoveItemEffect;            //�Ƿ���Ҫ�ƶ���קЧ��

	bool m_bNeedRefresh;
	UINT m_uMoveSelectIndex;           //�ƶ���ѡ�����
	bool m_bHasMoveItem;               //�ƶ���ѡ�е���Ŀ
	CStdString m_strMoveSelectImage;   //��ѡͼ

	CStdString m_strEmptyImage;        // ���б���ʾͼ
	bool m_bPaintEmptyImage;
	
	bool m_bFreshed;       //�Ѿ�ˢ��


	//���б�ʱ���ı�����
	DWORD m_dwTextColor;
	DWORD m_dwDisabledTextColor;
	int m_iFont;
	UINT m_uTextStyle;
	RECT m_rcTextPadding;
	bool m_bShowHtml;

	bool m_bSelTopIndex;		//�Ƿ�ѡ�ж��������0��
	bool m_bNeedCheckListData;   // �Ƿ���Ҫ��鵱ǰ����Դ�����Ƿ�ı�
	int  m_nEditIndex;
	int  m_ntoHeaderHeight;

	// ������ʽ
	bool m_bStyleAddItem;
	bool m_bWindowsStyle;		//Item��꾭����ѡ�У�����ѡ�з�Χ
	int  m_nLeftWidth;			//Item��꾭����ѡ�У�����ѡ�з�Χ������߱߿�ľ���

};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListBodyUIEx : public CVerticalLayoutUI
{
public:
	CListBodyUIEx(CListUIEx* pOwner);
	LPCTSTR GetClass();

	void SetScrollPos(SIZE szPos);
	void SetScrollLinePos(SIZE szPos, bool bScrollItem  = true );     //����һ�е�pos����
	void SetPos(RECT rc);
	void DoEvent(TEventUI& event);
	
	void SetVerticalScrollPos(int nPos);
	int  GetVerticalFixWidth();
	void SetVerticalScrollRange(int nRange);


	void LineDown();
	void LineUp();
	void PageUp();
	void PageDown();
	void HomeUp();
	void EndDown();

	bool IsHiddenItemInTop();          //��ͷ���Ƿ������ص���
	UINT GetHiddenItemInTop();          //��ȡͷͼ���ص�����
	bool IsHiddenItemInBottom();          //�ڵײ��Ƿ������ص���
	UINT GetHiddenItemInBootom();        //��ȡ�ײ����ص�����

protected:
	CListUIEx* m_pOwner;
	int m_nLeavePos;          //���һ����Ҫ���Ϲ���������
};

class UILIB_API CListContainerElementUIEx : public CContainerUI, public IListItemUI
{
public:
	CListContainerElementUIEx();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	int GetIndex() const;
	void SetIndex(int iIndex);
	void SetbVisibleEdit(bool bVisible);
	void SetbVisibleEdit(bool bVisible, bool bIsFolder);


	CListUIEx* GetOwner();
	void SetOwner(CControlUI* pOwner);
	void SetVisible(bool bVisible = true);
	void SetEnabled(bool bEnable = true);

	bool IsSelected() const;
	bool Select(bool bSelect = true, bool bInvalidate = true);
	bool IsExpanded() const;
	bool Expand(bool bExpand = true);
	void SetCanSel(bool bCanSel = true);
	bool GetCanSel() const;

	void Invalidate(); // ֱ��CControl::Invalidate�ᵼ�¹�����ˢ�£���д����ˢ������
	bool Activate();

	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);    
	void DrawItemBk(HDC hDC, const RECT& rcItem);

	//add by lighten
	SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);

	void SetShade(bool bShade = false);
	bool GetShade() const;
	void SetShadeColor(DWORD dwShadeColor);
	DWORD GetShadeColor() const;

	void SetShadeImage(CStdString strImage);
	LPCTSTR GetShadeImage() const;

protected:
	int m_iIndex;
	bool m_bSelected;
	UINT m_uButtonState;
	CListUIEx* m_pOwner;
	CRect m_rcNotData;               //������������
	int m_uScrollshowControl;        //��껬����ʾ�ӿؼ���������
	bool m_bNeedSelected;
	bool m_bShade;   //��Ӱ����
	DWORD m_dwShadeColor;
	CStdString m_strShadeImage;
	bool m_bvisibleEdit;
	bool m_bCanSel;
};

class UILIB_API CIgnoreListContainerElementUI : public CListContainerElementUIEx
{
public:
	CIgnoreListContainerElementUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	void DoEvent(TEventUI& event);
	bool Select(bool bSelect = true, bool bInvalidate = true);
};

}  // namespace DuiLib

#endif  // __UILISTEX_H__