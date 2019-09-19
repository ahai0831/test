#ifndef __UITREECONTROL_H__
#define __UITREECONTROL_H__
#pragma once

namespace DuiLib {

#define  NUM_LEFT       0          //ǰ��Ԥ������������������
	class CTreeContainerElementUI;
	//���������ݹ����б�
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CTreeBodyUI;

	class UILIB_API CTreeControlUI : public CVerticalLayoutUI, public IListUI
	{
	public:
		CTreeControlUI();
		~CTreeControlUI(void);

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		CControlUI* GetItemExAt(int iIndex) const; //��ȡָ����������
		int GetAllCount() const;                   //��ȡ������ 

		bool GetScrollSelect();
		void SetScrollSelect(bool bScrollSelect);
		int GetCurSel() const;

		CListHeaderUI* GetHeader() const{ return NULL; }
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


		bool UnSelectMoveTemp();

		bool SelectMoveTemp(int iIndex);            //�϶�ʱ��ѡ�����Ŀ
		void SetMultiExpanding(bool bMultiExpandable);
		int GetExpandedItem() const;
		bool ExpandItem(int iIndex, bool bExpand = true);

		bool ExpandNode();
	

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
		int  GetItemHeight() const;            //��ȡ��߶�
		void SetItemCount(int nNums);          //����������
		int  GetItemCount() const;            //��ȡ������
		int  GetTopIndex() const;             //��ȡ��������
		int  GetShowItems() const;            //��ȡ������ʾ��Item��

		void CalShowItemNum();       //���������ʾ������

		void SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex = true);    //����������Դ

		bool IsHiddenItemInTop();             //��ѯ�Ƿ�������������ǰ��
		bool IsHiddenItemInBottom();          //�ڵײ��Ƿ������ص���

		bool ScrollDownLine();     //������һ��
		UINT ScrollUpLine();       //������һ��
		bool RefreshListData(int nTopIndex, bool bInit = true);    //����ָ������ˢ������
		bool RefreshPos(int nTopIndex, bool bInit = true);         //����ָ������ˢ��pos������
		bool DeleteIndexData(int Index);        //ɾ��ָ��������
		bool InsetIndexData(int Index);         //���ƶ�����֮ǰ��������
		bool InsetDataAt(int nIndex);           //���ƶ�����֮ǰ��������(��ˢ��ȫ�����ݣ�
		bool DeleteDataAt(int nIndex);          //ɾ��ָ��������(��ˢ��ȫ�����ݣ�

		//add by lighten
		int CurSelItemIndex();
		bool SelectItem(int iIndex, bool bTakeFocus = false);                //ѡ��һ��
		bool SelectMultiItem(int iIndex, bool bTakeFocus = false, bool bSendMessge = true);           //��Ӷ�һ�ѡ��
		bool SetItemSelect(int nStart, int nEnd, bool bTakeFocus = false);   //ѡ��ָ����Χ����
		void SetSingleSelect(bool bSingleSel);
		bool GetSingleSelect() const;
		bool UnSelectItem(int iIndex, bool bSendMessge = true);
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

		CStdPtrArray GetSelArray() const;       //��ȡѡ�е�����

		bool NeedFreshData(bool bFRresh = true);    //�����Ƿ���Ҫˢ������

		int FindIndexbyPoint(POINT &pt);   //ͨ��ָ���ĵ��ҵ���Ӧ������
		void SetAllNeedHeight(int nHeight);
		void SetVerticalScrollPos(int nPos);

		bool IsSelectTopIndex();
	protected:
		void Init();               //��ʼ������
		int GetMinSelItemIndex();
		int GetMaxSelItemIndex();

	protected:
		bool m_bScrollSelect;
		/*	int m_iCurSel;*/
		int m_iExpandedItem;
		IListCallbackUI* m_pCallback;
		CTreeBodyUI* m_pTreeUI;
		TListInfoUI m_ListInfo;

		//add by lighten 	
		int m_nTopIndex;        //λ�ڶ�����������
		int m_nItemHeight;      //��߶�
		int m_nItems;           //������
		int m_nShowItems;   //������ʾ������

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


		bool m_bNeedRefresh;
		UINT m_uMoveSelectIndex;           //�ƶ���ѡ�����
		CStdString m_strMoveSelectImage;   //��ѡͼ


		bool m_bFreshed;       //�Ѿ�ˢ��


		//���б�ʱ���ı�����
		DWORD m_dwTextColor;
		DWORD m_dwDisabledTextColor;
		int m_iFont;
		UINT m_uTextStyle;
		RECT m_rcTextPadding;
		bool m_bShowHtml;

		bool m_bSelTopIndex;		//�Ƿ�ѡ�ж��������0��
	};
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CTreeBodyUI : public CVerticalLayoutUI
	{
	public:
		CTreeBodyUI(CTreeControlUI* pOwner);
		LPCTSTR GetClass();

		void SetScrollPos(SIZE szPos);
		void SetScrollLinePos(SIZE szPos, bool bScrollItem = true);     //����һ�е�pos����
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
		CTreeControlUI* m_pOwner;
		int m_nLeavePos;          //���һ����Ҫ���Ϲ���������
	};

	class UILIB_API CTreeContainerElementUI : public CContainerUI, public IListItemUI
	{
	public:
		CTreeContainerElementUI();
		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		int GetIndex() const;
		void SetIndex(int iIndex);

		CTreeControlUI* GetOwner();
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
		CTreeControlUI* m_pOwner;
		CRect m_rcNotData;               //������������
		int m_uScrollshowControl;        //��껬����ʾ�ӿؼ���������
		bool m_bNeedSelected;
		bool m_bShade;   //��Ӱ����
		DWORD m_dwShadeColor;
		CStdString m_strShadeImage;

	};

}

#endif  // __UITREECONTROL_H__