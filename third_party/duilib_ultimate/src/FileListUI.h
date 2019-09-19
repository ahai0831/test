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
	int _LeftPos;   //��ߵ�λ��
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
	
	void SetExpandImageSize(SIZE sz);    //����չ��ͼƬ��С
	SIZE GetExpandImageSize() const;     //��ȡչ��ͼƬ��С
	void SetSelctedImageSize(SIZE sz);    //����ѡ��ͼƬ��С
	SIZE GetSelctedImageSize() const;     //����ѡ��ͼƬ��С

	void SetExpandImage(LPCTSTR strImage); //����չ��״̬ʱͼƬ
	LPCTSTR GetExpandImage() const;
	void SetNoExpandImage(LPCTSTR strImage);//���÷�չ��״̬ʱͼƬ
	LPCTSTR GetNoExpandImage() const;
	void SetSelctedImage(LPCTSTR strImage);   //����ѡ��ʱ��״̬ͼ
	LPCTSTR GetSelectedImage() const;
	void SetNoSelectedImage(LPCTSTR strImage);  //���÷�ѡ��ʱ��״̬ͼ
	LPCTSTR GetNoSelectedImage() const;
	void SetSpecificImage(LPCTSTR strImage);  //���÷�ѡ��ʱ��״̬ͼ
	LPCTSTR GetSpecificImage() const;

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	LPCTSTR GetSelPath();
	void SetNodeExpandAttr(TreeNode *pNode, bool leafFlag = false);

	TreeNode *GetSelNode();
	void UnSelAllNode();							//ȡ���������ڵ�ѡ�е�״̬
	void SetExpandSelected(bool bExpandSel = true);	//����չ�������ڵ�ʱ�Ƿ���Ҫѡ��״̬
	bool GetExpandSelected();
	void ClearSelNode();							//�������ѡ�еĽڵ�

	void SetToolTipFlag(bool bShowFlag = false);	//����tooltip�Ƿ���	www 2014.06.09
	void SetEndEllipsisLength(int nNumbers = -1);	//����ʡ��ʱ��������ַ�����
private:
	TreeNode* _root;
	bool m_bExpandSelected;			      // ���չ������ʱ��Ĭ��ѡ��
	bool m_bSelNeedSelBkFlag;
	bool m_bSetToolTipFlag;				  //����TOOLTIP����		www 2014.06.09
	int m_nEllipsisLength;				  //ʡ��ʱ��������ַ����� www 2014.06.09
	SIZE m_nExpandPos;                    //չ��ͼƬ��С
	SIZE m_nSelectedPos;                  //ѡ����ͼƬ��С
	CStdString m_strExpandIamge;          //չ��ʱͼƬ
	CStdString m_strNoExpandImage;        //��չ��ʱͼƬ
	CStdString m_strSelectdImage;         //ѡ��ʱͼƬ
	CStdString m_strNoSelectedImage;      //��ѡ����ͼƬ
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
