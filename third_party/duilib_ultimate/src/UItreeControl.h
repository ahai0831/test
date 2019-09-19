#ifndef __UITREECONTROL_H__
#define __UITREECONTROL_H__
#pragma once

namespace DuiLib {

#define  NUM_LEFT       0          //前后预留的项数，用来滚动
	class CTreeContainerElementUI;
	//大批量数据管理列表
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

		CControlUI* GetItemExAt(int iIndex) const; //获取指定索引的项
		int GetAllCount() const;                   //获取总项数 

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

		bool SelectMoveTemp(int iIndex);            //拖动时，选择的项目
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
		void EnsureVisible(int iIndex);      //显示指定的项
		void SetItemHeight(int nItemHeigh);    //设置项高度
		int  GetItemHeight() const;            //获取项高度
		void SetItemCount(int nNums);          //设置总项数
		int  GetItemCount() const;            //获取总项数
		int  GetTopIndex() const;             //获取顶部索引
		int  GetShowItems() const;            //获取可以显示的Item数

		void CalShowItemNum();       //计算可以显示的项数

		void SetListDataSource(IListDataSource *ListDataSource, bool bChangeTopIndex = true);    //设置数据来源

		bool IsHiddenItemInTop();             //查询是否有数据隐藏在前面
		bool IsHiddenItemInBottom();          //在底部是否有隐藏的项

		bool ScrollDownLine();     //往下走一行
		UINT ScrollUpLine();       //往上走一行
		bool RefreshListData(int nTopIndex, bool bInit = true);    //根据指定索引刷新数据
		bool RefreshPos(int nTopIndex, bool bInit = true);         //根据指定索引刷新pos和数据
		bool DeleteIndexData(int Index);        //删除指定的数据
		bool InsetIndexData(int Index);         //在制定索引之前插入数据
		bool InsetDataAt(int nIndex);           //在制定索引之前插入数据(不刷新全部数据）
		bool DeleteDataAt(int nIndex);          //删除指定的数据(不刷新全部数据）

		//add by lighten
		int CurSelItemIndex();
		bool SelectItem(int iIndex, bool bTakeFocus = false);                //选中一项
		bool SelectMultiItem(int iIndex, bool bTakeFocus = false, bool bSendMessge = true);           //添加多一项被选中
		bool SetItemSelect(int nStart, int nEnd, bool bTakeFocus = false);   //选中指定范围的项
		void SetSingleSelect(bool bSingleSel);
		bool GetSingleSelect() const;
		bool UnSelectItem(int iIndex, bool bSendMessge = true);
		void SelectAllItems();
		void UnSelectAllItems();
		int GetSelectItemCount() const;
		int GetNextSelItem(int nItem) const;
		int SetFocusItem(int nItem, bool bTakeFocus = false);   //设置焦点所在项
		int SetEndItem(int nItem, bool bTakeFocus = false);     //设置多选中结束项
		int GetEndItem() const;
		bool SetLButtonState(bool bDown);   //设置鼠标是否被按下
		bool GetLButtonState() const;
		bool SetLButton_ClickData(bool bDown);   //设置鼠标是否点击在数据区域
		bool GetLButtonClickData() const;
		void SetStartPoint(POINT pt);   //设置开始起点

		CStdPtrArray GetSelArray() const;       //获取选中的数组

		bool NeedFreshData(bool bFRresh = true);    //设置是否需要刷新数据

		int FindIndexbyPoint(POINT &pt);   //通过指定的点找到对应的索引
		void SetAllNeedHeight(int nHeight);
		void SetVerticalScrollPos(int nPos);

		bool IsSelectTopIndex();
	protected:
		void Init();               //初始化数据
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
		int m_nTopIndex;        //位于顶部的索引号
		int m_nItemHeight;      //项高度
		int m_nItems;           //项总数
		int m_nShowItems;   //可以显示的项数

		IListDataSource *m_pListDataSource;   //数据来源

		bool m_bSingleSel;                   //是否单选，默认多选   
		CStdPtrArray m_aSelItems;            //管理选中的项
		int m_nFocusItem;
		int m_nStartSelItem;                 //多选开始项
		int m_nEndSelItem;
		bool m_bLButtonDown;                  //点击在非数据区域 
		bool m_bLButtonDown_ClickInData;            //点击在数据区域
		POINT m_pStartPoint;   //开始鼠标点
		POINT m_pEndPoint;     //结束鼠标点
		POINT m_pTempPoint;    //中间节点
		SIZE m_StartPos;       //开始所在的pos


		bool m_bNeedRefresh;
		UINT m_uMoveSelectIndex;           //移动中选择的项
		CStdString m_strMoveSelectImage;   //框选图


		bool m_bFreshed;       //已经刷新


		//空列表时，文本属性
		DWORD m_dwTextColor;
		DWORD m_dwDisabledTextColor;
		int m_iFont;
		UINT m_uTextStyle;
		RECT m_rcTextPadding;
		bool m_bShowHtml;

		bool m_bSelTopIndex;		//是否选中顶项，即索引0项
	};
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CTreeBodyUI : public CVerticalLayoutUI
	{
	public:
		CTreeBodyUI(CTreeControlUI* pOwner);
		LPCTSTR GetClass();

		void SetScrollPos(SIZE szPos);
		void SetScrollLinePos(SIZE szPos, bool bScrollItem = true);     //滚动一行的pos设置
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

		bool IsHiddenItemInTop();          //在头部是否有隐藏的项
		UINT GetHiddenItemInTop();          //获取头图隐藏的像素
		bool IsHiddenItemInBottom();          //在底部是否有隐藏的项
		UINT GetHiddenItemInBootom();        //获取底部隐藏的像素

	protected:
		CTreeControlUI* m_pOwner;
		int m_nLeavePos;          //最后一项需要向上滚动的像素
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

		void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
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
		CRect m_rcNotData;               //数据所在区域
		int m_uScrollshowControl;        //鼠标滑动显示子控件的子项标号
		bool m_bNeedSelected;
		bool m_bShade;   //阴影覆盖
		DWORD m_dwShadeColor;
		CStdString m_strShadeImage;

	};

}

#endif  // __UITREECONTROL_H__