#ifndef __FILELISTUI_H__
#define __FILELISTUI_H__
#pragma once;

#include <vector>
#include <math.h>

namespace DuiLib {

inline double CalculateDelay(double state)
{
	return pow(state, 2);
}

struct NodeData
{
	int _level;
	int _LeftPos;   //左边的位置
	bool _expand;
	bool _select;
	CStdString _expandText;
	CStdString _selectionBoxText;
	CStdString _text;
	CStdString _path;   //add by ligthen
	CListLabelElementUI* _pListElement;
};

class UILIB_API TreeNode
{
public:
	TreeNode();
	explicit TreeNode(NodeData t);
	TreeNode(NodeData t, TreeNode* parent);
	~TreeNode();

	NodeData& data() ;
	int num_children() const;
	TreeNode* child(int i);
	TreeNode* parent();
	bool has_children() const;

	void add_child(TreeNode* child) ;
	void remove_child(TreeNode* child);
	TreeNode* get_last_child();

private:
	void set_parent(TreeNode* parent);

public:
	typedef std::vector <TreeNode*>	Children;
	Children	_children;
	TreeNode*		_parent;
	NodeData    _data;
	bool  LeafFlag;
};	

class UILIB_API CFileListUI : public CListUI
{
public:
	CFileListUI();
	~CFileListUI();

	bool Add(CControlUI* pControl);
	bool AddAt(CControlUI* pControl, int iIndex);

	bool Remove(CControlUI* pControl);
	bool RemoveAt(int iIndex);
	void RemoveAll();

	void SetVisible(bool bVisible = true);
	void SetInternVisible(bool bVisible = true);

	TreeNode* GetRoot();
	bool RemoveNode(TreeNode* TreeNode);

	TreeNode* AddNode(LPCTSTR text, TreeNode* parent = NULL, bool leafFlag = false );	
	void ExpandNode( TreeNode* TreeNode );
	void SelectionBoxNodeCore( TreeNode* TreeNode, bool select );
	SIZE GetExpanderSizeX(TreeNode* TreeNode) const;
	SIZE GetSelectionBoxSizeX( TreeNode* TreeNode ) const;
	
	void SetExpandImageSize(SIZE sz);    //设置展开图片大小
	SIZE GetExpandImageSize() const;     //获取展开图片大小
	void SetSelctedImageSize(SIZE sz);    //设置选中图片大小
	SIZE GetSelctedImageSize() const;     //设置选中图片大小

	void SetExpandImage(LPCTSTR strImage); //设置展开状态时图片
	LPCTSTR GetExpandImage() const;
	void SetNoExpandImage(LPCTSTR strImage);//设置非展开状态时图片
	LPCTSTR GetNoExpandImage() const;
	void SetSelctedImage(LPCTSTR strImage);   //设置选中时的状态图
	LPCTSTR GetSelectedImage() const;
	void SetNoSelectedImage(LPCTSTR strImage);  //设置非选中时的状态图
	LPCTSTR GetNoSelectedImage() const;
	void SetSpecificImage(LPCTSTR strImage);  //设置非选中时的状态图
	LPCTSTR GetSpecificImage() const;

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	LPCTSTR GetSelPath();
	void SetNodeExpandAttr(TreeNode *pNode, bool leafFlag = false);

	TreeNode *GetSelNode();
	void UnSelAllNode();							//取法所有树节点选中的状态
	void SetExpandSelected(bool bExpandSel = true);	//设置展开收缩节点时是否需要选中状态
	bool GetExpandSelected();
	void ClearSelNode();							//清除所有选中的节点

	void SetToolTipFlag(bool bShowFlag = false);	//设置tooltip是否标记	www 2014.06.09
	void SetEndEllipsisLength(int nNumbers = -1);	//设置省略时最大中文字符长度
private:
	TreeNode* _root;
	bool m_bExpandSelected;			      // 点击展开符号时，默认选中
	bool m_bSelNeedSelBkFlag;
	bool m_bSetToolTipFlag;				  //开启TOOLTIP设置		www 2014.06.09
	int m_nEllipsisLength;				  //省略时最大中文字符长度 www 2014.06.09
	SIZE m_nExpandPos;                    //展开图片大小
	SIZE m_nSelectedPos;                  //选中项图片大小
	CStdString m_strExpandIamge;          //展开时图片
	CStdString m_strNoExpandImage;        //非展开时图片
	CStdString m_strSelectdImage;         //选中时图片
	CStdString m_strNoSelectedImage;      //非选择中图片
	CStdString m_strSpecificImage;
};

class UILIB_API CTreeElement: public CListLabelElementUI
{
public:
	CTreeElement();
	~CTreeElement();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	void DoPaint(HDC hDC, const RECT& rcPaint);
	void DrawItem(HDC hDC, const RECT& rcItem);
	void DoEvent(TEventUI& event);
	void SetShowText(LPCTSTR pstrText);
	
protected:
	CStdString m_ShowText;
	CRect m_rcInvalidate;
	
};

}
#endif
