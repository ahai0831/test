#ifndef UITreeView_h__
#define UITreeView_h__


#pragma once

namespace DuiLib
{
	class CTreeViewUI;
	class CCustomizationOptionUI;
	class CLabelUI;

	class UILIB_API CTreeNodeUI : public CListContainerElementUI
	{
	public:
		CTreeNodeUI(CTreeNodeUI* _ParentNode = NULL);
		~CTreeNodeUI(void);

	public:
		LPCTSTR GetClass() const;
		LPVOID	GetInterface(LPCTSTR pstrName);
		void	DoEvent(TEventUI& event);
		void	Invalidate();
		bool	Select(bool bSelect = true);
		bool IsExpanded() const;
		bool Expand(bool bExpand = true);


		bool	Add(CControlUI* _pTreeNodeUI);
		bool	AddAt(CControlUI* pControl, int iIndex);
		bool	Remove(CControlUI* pControl);

		void	SetVisible(bool bVisible = true);
		void	SetVisibleTag(bool _IsVisible);
		bool	GetVisibleTag();
		void	SetItemText(LPCTSTR pstrValue);
		CStdString	GetItemText();
		void	CheckBoxSelected(bool _Selected);
		bool	IsCheckBoxSelected() const;
		bool	IsHasChild() const;
		long	GetTreeLevel() const;
		bool	AddChildNode(CTreeNodeUI* _pTreeNodeUI);
		bool	RemoveAt(CTreeNodeUI* _pTreeNodeUI);
		void	SetParentNode(CTreeNodeUI* _pParentTreeNode);
		CTreeNodeUI* GetParentNode();
		long	GetCountChild();
		void	SetTreeView(CTreeViewUI* _CTreeViewUI);
		CTreeViewUI* GetTreeView();
		CTreeNodeUI* GetChildNode(int _nIndex);
		void	SetVisibleFolderBtn(bool _IsVisibled);
		bool	GetVisibleFolderBtn();
		void	SetVisibleCheckBtn(bool _IsVisibled);
		bool	GetVisibleCheckBtn();
		void    SetVisibleLoadingLabel(bool _IsVisibled);
		bool    GetVisibleLoadingLabel();
		void	SetItemTextColor(DWORD _dwItemTextColor);
		DWORD	GetItemTextColor() const;
		void	SetItemHotTextColor(DWORD _dwItemHotTextColor);
		DWORD	GetItemHotTextColor() const;
		void	SetSelItemTextColor(DWORD _dwSelItemTextColor);
		DWORD	GetSelItemTextColor() const;
		void	SetSelItemHotTextColor(DWORD _dwSelHotItemTextColor);
		DWORD	GetSelItemHotTextColor() const;
		void    SetExpendwidth(int nWidth);
		int		GetExpendwidth();
		// 鼠标提示
		virtual CStdString GetToolTip() const;

		SIZE EstimateSize(SIZE szAvailable);

		void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		
		void	IsAllChildChecked();
		CStdPtrArray GetTreeNodes();

		int			 GetTreeIndex();
		int			 GetNodeIndex();
		void DoPaint(HDC hDC, const RECT& rcPaint);
		void DrawItemBk(HDC hDC, const RECT& rcItem);
		void DrawItemText(HDC hDC, const RECT& rcItem);
	private:
		CTreeNodeUI* GetLastNode();
		CTreeNodeUI* CalLocation(CTreeNodeUI* _pTreeNodeUI);
	public:
		CHorizontalLayoutUI*	GetTreeNodeHoriznotal() const {return m_pHoriz;};
		CCustomizationOptionUI*			GetFolderButton() const { return m_pFolderButton; };
		CLabelUI*				GetFolderOccupy()const { return m_pFolderOccupy; }
		CLabelUI*				GetDottedLine() const {return m_pDottedLine;};
		CCustomizationOptionUI*			GetCheckBox() const { return m_pCheckBox; }
		CCustomizationOptionUI*				GetItemButton() const { return m_pItemButton; }
		CLabelUI*				GetLoadingLabel() const { return m_ploadingLabel; }
	private:
		long	m_iTreeLavel;
		bool	m_bIsVisable;
		bool	m_bIsCheckBox;
		DWORD	m_dwItemTextColor;
		DWORD	m_dwItemHotTextColor;
		DWORD	m_dwSelItemTextColor;
		DWORD	m_dwSelItemHotTextColor;

		CTreeViewUI*			m_pTreeView;
		CHorizontalLayoutUI*	m_pHoriz;
		CCustomizationOptionUI*			m_pFolderButton;
		CLabelUI*				m_pDottedLine;
		CLabelUI*				m_pFolderOccupy;
		CCustomizationOptionUI*			m_pCheckBox;
		CCustomizationOptionUI*				m_pItemButton;
		CLabelUI*				m_ploadingLabel;
		CTreeNodeUI*			m_pParentTreeNode;

		CStdPtrArray			m_TreeNodes;
		int		m_nexpendwidth;
	};

	class UILIB_API CTreeViewUI : public CListUI,public INotifyUI
	{
	public:
		CTreeViewUI(void);
		~CTreeViewUI(void);

	public:
		virtual LPCTSTR GetClass() const;
		virtual LPVOID	GetInterface(LPCTSTR pstrName);
		virtual bool Add(CTreeNodeUI* pControl );
		virtual long AddAt(CTreeNodeUI* pControl, int iIndex );
		virtual bool AddAt(CTreeNodeUI* pControl,CTreeNodeUI* _IndexNode);
		virtual bool Remove(CTreeNodeUI* pControl);
		virtual bool RemoveAt(int iIndex);
		virtual void RemoveAll();
		virtual bool OnCheckBoxChanged(void* param);
		virtual bool OnFolderChanged(void* param);
		virtual bool OnDBClickItem(void* param);
		virtual bool SetItemCheckBox(bool _Selected,CTreeNodeUI* _TreeNode = NULL);
		virtual void SetItemExpand(bool _Expanded,CTreeNodeUI* _TreeNode = NULL);
		virtual void Notify(TNotifyUI& msg);
		virtual void SetVisibleFolderBtn(bool _IsVisibled);
		virtual bool GetVisibleFolderBtn();
		virtual void SetVisibleCheckBtn(bool _IsVisibled);
		virtual bool GetVisibleCheckBtn();
		virtual void SetItemMinWidth(UINT _ItemMinWidth);
		virtual UINT GetItemMinWidth();
		virtual void SetItemTextColor(DWORD _dwItemTextColor);
		virtual void SetItemHotTextColor(DWORD _dwItemHotTextColor);
		virtual void SetSelItemTextColor(DWORD _dwSelItemTextColor);
		virtual void SetSelItemHotTextColor(DWORD _dwSelHotItemTextColor);
		
		//add by lighten
		void UnSelectAllItems();
		bool SetSelectItem(int iIndex, bool bTakeFocus = false);
		
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		CTreeNodeUI* GetCurSelNode();
		CStdPtrArray GetAllSelCheckNode();
		void SetToolTipFlag(bool bShowFlag = false);	//设置tooltip是否标记
		bool GetToolTipFlag();
		void SetEndEllipsisLength(int nNumbers = -1);	//设置省略时最大中文字符长度
		int GetEndEllipsisLength();
		CTreeNodeUI* FindControlByPoint(POINT &pt, int &IndexOut);
	private:
		UINT m_uItemMinWidth;
		bool m_bVisibleFolderBtn;
		bool m_bVisibleCheckBtn;
		int	 m_nEllipsisLength;
		bool m_bSetToolTipFlag;				  //开启TOOLTIP设置	
	};
}


#endif // UITreeView_h__
