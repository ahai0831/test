#include "StdAfx.h"
#include "FileListUI.h"
namespace DuiLib {
TreeNode::TreeNode() : _parent (NULL), LeafFlag( false )
{

}
TreeNode::TreeNode(NodeData t) : _data (t), _parent (NULL)
{

}

TreeNode::TreeNode(NodeData t, TreeNode* parent)	: _data (t), _parent (parent) 
{

}

TreeNode::~TreeNode() 
{
	for (int i = 0; i < num_children(); i++)
	{
		if (_children[i])
		{
			delete _children[i]; 
		}
	}
}

NodeData& TreeNode::data() 
{ 
	return _data; 
}	

int TreeNode::num_children() const
{ 
	return _children.size();
}

TreeNode* TreeNode::child(int i)	
{ 
	return _children[i]; 
}

TreeNode* TreeNode::parent() 
{ 
	return ( _parent);	
}

bool TreeNode::has_children() const 
{	
	return num_children() > 0;
}

void TreeNode::add_child(TreeNode* child) 
{
	if (child == NULL)
	{
		return;
	}
	child->set_parent(this); 
	_children.push_back(child); 
}

void TreeNode::remove_child(TreeNode* child)
{
	if (child == NULL)
	{
		return;
	}
	Children::iterator iter = _children.begin();
	for( ; iter < _children.end(); ++iter )
	{
		if( *iter == child ) 
		{
			_children.erase(iter);
			return;
		}
	}
}

TreeNode* TreeNode::get_last_child()
{
	if( has_children() )
	{
		return child(num_children() - 1)->get_last_child();
	}
	else return this;
}

void TreeNode::set_parent(TreeNode* parent) 
{ 
	if (parent == NULL)
	{
		return;
	}
	_parent = parent; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CFileListUI::CFileListUI() : _root(NULL)
{
	SetItemShowHtml(true);

	_root = NULL;
	_root = new TreeNode;
	if (_root != NULL)
	{
		_root->data()._level = -1;
		_root->data()._expand = true;
		_root->data()._select = false;
		_root->data()._text = _T("");	
		_root->data()._pListElement = NULL;
	}

	m_bExpandSelected = true;
	m_nExpandPos.cy = 0;
	m_nExpandPos.cx = 11;
	m_nSelectedPos.cy = 0;
	m_nSelectedPos.cx = 15;
	m_bSetToolTipFlag = false;
	m_nEllipsisLength = -1;
//	SetSelectedItemBkColor(0xFFFFFFFF);
}

CFileListUI::~CFileListUI() 
{
	if(_root)
		delete _root; 
}

bool CFileListUI::Add(CControlUI* pControl)
{
	if( !pControl ) return false;
	if( _tcscmp(pControl->GetClass(), _T("TreeElementUI")) != 0 ) 
		return false;

	return CListUI::Add(pControl);
}

bool CFileListUI::AddAt(CControlUI* pControl, int iIndex)
{
	if( !pControl ) return false;
	if( _tcscmp(pControl->GetClass(), _T("TreeElementUI")) != 0 ) return false;
	return CListUI::AddAt(pControl, iIndex);
}

bool CFileListUI::Remove(CControlUI* pControl)
{
	if( !pControl ) return false;
	if( _tcscmp(pControl->GetClass(), _T("TreeElementUI")) != 0 ) return false;

	if (reinterpret_cast<TreeNode*>(static_cast<CTreeElement*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()) == NULL)
		return CListUI::Remove(pControl);
	else
		return RemoveNode(reinterpret_cast<TreeNode*>(static_cast<CTreeElement*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()));
}

bool CFileListUI::RemoveAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if( !pControl ) return false;
	if( _tcscmp(pControl->GetClass(), _T("TreeElementUI")) != 0 ) return false;

	if (reinterpret_cast<TreeNode*>(static_cast<CTreeElement*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()) == NULL)
		return CListUI::RemoveAt(iIndex);
	else
		return RemoveNode(reinterpret_cast<TreeNode*>(static_cast<CTreeElement*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()));
}

void CFileListUI::RemoveAll()
{
	CListUI::RemoveAll();
	for (int i = 0; i < _root->num_children(); ++i)
	{
		TreeNode* child = _root->child(i);
		RemoveNode(child);
	}
	if (_root != NULL)
	{
		delete _root;
		_root = new TreeNode;
	}
	if (_root == NULL)
	{
		return;
	}
	_root->data()._level = -1;
	_root->data()._expand = true;
	_root->data()._pListElement = NULL;
}

void CFileListUI::SetVisible(bool bVisible )
{
	if( m_bVisible == bVisible ) return;
	CControlUI::SetVisible(bVisible);
}

void CFileListUI::SetInternVisible(bool bVisible )
{
	CControlUI::SetInternVisible(bVisible);
}

TreeNode* CFileListUI::GetRoot() 
{ 
	return _root; 
}

bool CFileListUI::RemoveNode(TreeNode* node)
{
	if( !node || node == _root ) return false;
	for( int i = 0; i < node->num_children(); ++i ) {
		TreeNode* child = node->child(i);
		RemoveNode(child);
	}
	CListUI::Remove(node->data()._pListElement);
	node->parent()->remove_child(node);
	delete node;
	return true;
}

TreeNode* CFileListUI::AddNode(LPCTSTR text, TreeNode* parent, bool leafFlag )
{
	if( !parent ) parent = _root;

	CTreeElement* pListElement = new CTreeElement;
	if (pListElement == NULL)
	{
		return NULL;
	}
	TreeNode* node = new TreeNode;
	if (node == NULL)
	{
		return NULL;
	}
	node->data()._level = parent->data()._level + 1;
	node->data()._expand = false;
	node->data()._select = false;

	node->data()._text = text;
	if (parent == _root)
	{
		node->data()._path = text;
	}
	else
		node->data()._path = parent->data()._path + _T("\\") + text;

	node->data()._pListElement = pListElement;
	node->LeafFlag = leafFlag;

	if( parent != _root ) 
	{
		if( !(parent->data()._expand && parent->data()._pListElement->IsVisible()) )
			pListElement->SetInternVisible(false);
	}

	CStdString html_text;  
	html_text += _T("<x 8>"); 
	node->data()._LeftPos = 8;
	for( int i = 0; i < node->data()._level; ++i ) {
		html_text += _T("<x 24>"); 
		node->data()._LeftPos += 24;
	}

	if( !node->LeafFlag )
	{
		node->data()._selectionBoxText = _T("{x 8}{i ");
		if(parent->_parent == NULL && !m_strSpecificImage.IsEmpty())
		{
			node->data()._selectionBoxText += m_strSpecificImage;
		}
		else
		{
			node->data()._selectionBoxText += m_strNoSelectedImage;
		}
		
		node->data()._selectionBoxText += _T(" }{x 2}");
		node->data()._LeftPos += 8;
		html_text += _T("<i ");
		html_text += m_strExpandIamge;
		html_text += _T(">");
//		node->data()._LeftPos += m_nExpandPos.cx/* + m_nSelectedPos.cx*/;
	}
	else
	{
		node->data()._selectionBoxText = _T("{x 8}{i ");
		if(parent->_parent == NULL && !m_strSpecificImage.IsEmpty())
		{
			node->data()._selectionBoxText += m_strSpecificImage;
		}
		else
		{
			node->data()._selectionBoxText += m_strNoSelectedImage;
		}
		node->data()._selectionBoxText += _T("}{x 2}");
		html_text += _T("{x ");
		CStdString strSize;
		strSize.Format(_T("%d"), m_nExpandPos.cx);
		html_text += strSize;
		html_text += _T("}");
		node->data()._LeftPos += 8;
//		node->data()._LeftPos += m_nSelectedPos.cx;

	}
//	node->data()._LeftPos += 2;

	node->data()._LeftPos += m_nExpandPos.cx/* + m_nSelectedPos.cx*/;
	node->data()._expandText = html_text;
	// www 2014.06.09
	if (m_nEllipsisLength > 0 && m_nEllipsisLength <= MAX_PATH)
	{
		pListElement->SetEndEllipsisLength(m_nEllipsisLength);
	}
	pListElement->SetText( node->data()._expandText + node->data()._selectionBoxText + node->data()._text );
	pListElement->SetShowText(text);
	pListElement->SetTag((UINT_PTR)node);

	// www 2014.06.09
	if (m_bSetToolTipFlag)
	{
		pListElement->SetToolTip(text);
	}

	int index = 0;
	if( parent->has_children() ) 
	{
		TreeNode* prev = parent->get_last_child();
		index = prev->data()._pListElement->GetIndex() + 1;
	}
	else
	{
		if( parent == _root ) 
			index = 0;
		else 
			index = parent->data()._pListElement->GetIndex() + 1;
	}

	if( !CListUI::AddAt(pListElement, index) )
	{
		delete pListElement;
		delete node;
		node = NULL;
	}

	parent->add_child(node);
	return node;
}

void CFileListUI::ExpandNode(TreeNode* node)
{	
	if (node == NULL)
	{
		return;
	}
	if( !node->data()._pListElement->IsVisible() ) 
		return;
	if( !node->has_children() ) 
		return;
	bool expand = !node->data()._expand;
	if( !node || node == _root )
		return;

	if( node->data()._expand == expand )
		return;

	node->data()._expand = expand;

		
	CStdString html_text;
	html_text += _T("<x 8>");
	for( int i = 0; i < node->data()._level; ++i ) 
	{
		html_text += _T("<x 24>");
	}

	if( !node->LeafFlag )
	{
		html_text += _T("<a><i ");
		if( node->data()._expand ) 
			html_text += m_strNoExpandImage;
		else 
			html_text += m_strExpandIamge;
		html_text += _T("></a>");
//			html_text += _T("<a><i expand.png></a>");
	}

	node->data()._expandText = html_text;
	node->data()._pListElement->SetText( node->data()._expandText + node->data()._selectionBoxText + node->data()._text );


	TreeNode* begin = node->child(0);
	TreeNode* end = node->get_last_child();
	if ((begin == NULL) || (end == NULL))
	{
		return;
	}
	for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) 
	{
		CControlUI* control = GetItemAt(i);
		if( _tcscmp(control->GetClass(), _T("TreeElementUI")) == 0 )
		{
			TreeNode* local_parent = ((TreeNode*)control->GetTag())->parent();
			control->SetInternVisible(local_parent->data()._expand && local_parent->data()._pListElement->IsVisible());
// 			if(local_parent->_data._expandselect)
// 			{
// 				local_parent->_data._expandselect = false;
// 			}
			CTreeElement *pTreeElement = static_cast<CTreeElement *>(control);
			if (pTreeElement && GetExpandSelected())
			{
				pTreeElement->Select(false);
			}
		}
	}

// 	if( !GetSelNodeBkFlag() && node->has_children() )
// 	{
// 		node->data()._expandselect = true;
// 	}
	NeedUpdate();
}

void CFileListUI::SelectionBoxNodeCore( TreeNode* node, bool select )
{
	if( !node || node == _root )
		return;

	if( node->data()._select == select )
		return;

	node->data()._select = select;

	if( !node->LeafFlag )
	{	
		node->data()._selectionBoxText = _T("{x 8}{i ");
		if( node->data()._select ) 
		{
			if(node->_parent->parent() == NULL && !m_strSpecificImage.IsEmpty())
			{
				node->data()._selectionBoxText += m_strSpecificImage;
			}
			else
			{
				node->data()._selectionBoxText += m_strSelectdImage;
			}
			
//			node->data()._selectionBoxText = _T("{x 8}{i select.png}{x 2}");
		}
		else
		{
			if(node->_parent->parent() == NULL && !m_strSpecificImage.IsEmpty())
			{
				node->data()._selectionBoxText += m_strSpecificImage;
			}
			else
			{
				node->data()._selectionBoxText += m_strNoSelectedImage;
			}
			
//			node->data()._selectionBoxText += _T("{x 8}{i notselect.png}{x 2}");
		}
		node->data()._selectionBoxText += _T("}{x 2}");
	}
	else
	{
		node->data()._selectionBoxText = _T("{x 8}{i ");
		if( node->data()._select ) 
		{
			if(node->_parent->parent() == NULL && !m_strSpecificImage.IsEmpty())
			{
				node->data()._selectionBoxText += m_strSpecificImage;
			}
			else
			{
				node->data()._selectionBoxText += m_strSelectdImage;
			}
			
//			node->data()._selectionBoxText = _T("{x 24}{i select.png}{x 2}");
		}
		else
		{
			if(node->_parent->parent() == NULL && !m_strSpecificImage.IsEmpty())
			{
				node->data()._selectionBoxText += m_strSpecificImage;
			}
			else
			{
				node->data()._selectionBoxText += m_strNoSelectedImage;
			}
//			node->data()._selectionBoxText = _T("{x 24}{i notselect.png}{x 2}");
		}
		node->data()._selectionBoxText += _T("}{x 2}");
	}

	node->data()._pListElement->SetText( node->data()._expandText + node->data()._selectionBoxText + node->data()._text );

	if( !node->data()._pListElement->IsVisible() ) 
		return;
	if( !node->has_children() ) 
		return;

//	NeedUpdate();
}

SIZE CFileListUI::GetExpanderSizeX(TreeNode* node) const
{
	if( !node || node == _root ) 
		return CSize();

	if( node->LeafFlag )
		return CSize();

	SIZE szExpander = {0};
	szExpander.cx = 6 + 24 * node->data()._level;
	szExpander.cy = szExpander.cx + 11;
	return szExpander;
}

SIZE CFileListUI::GetSelectionBoxSizeX( TreeNode* node ) const
{
	if( !node || node == _root ) 
		return CSize();

	SIZE szSelectionBox = {0};
	if( !node->LeafFlag )	
	{
		szSelectionBox.cx = 6 + 24 + 24 * node->data()._level  + 11 + 8;
		szSelectionBox.cy = szSelectionBox.cx + 15;
	}
	else
	{
		szSelectionBox.cx = 6 +  24 + 24 * node->data()._level  + 11 + 8;
		szSelectionBox.cy = szSelectionBox.cx + 15;
	}

	return szSelectionBox;
}

void CFileListUI::SetExpandImageSize(SIZE sz)    //设置展开图片大小
{
	m_nExpandPos = sz;
}

SIZE CFileListUI::GetExpandImageSize() const     //获取展开图片大小
{
	return m_nExpandPos;
}

void CFileListUI::SetSelctedImageSize(SIZE sz)    //设置选中图片大小
{
	m_nSelectedPos = sz;
}

SIZE CFileListUI::GetSelctedImageSize() const     //设置选中图片大小
{
	return m_nSelectedPos;
}

void CFileListUI::SetExpandImage(LPCTSTR strImage) //设置展开状态时图片
{
	m_strExpandIamge = strImage;
}

LPCTSTR CFileListUI::GetExpandImage() const
{
	return m_strExpandIamge;
}

void CFileListUI::SetNoExpandImage(LPCTSTR strImage)//设置非展开状态时图片
{
	m_strNoExpandImage = strImage;
}

LPCTSTR CFileListUI::GetNoExpandImage() const
{
	return m_strNoExpandImage;
}

void CFileListUI::SetSelctedImage(LPCTSTR strImage)   //设置选中时的状态图
{
	m_strSelectdImage = strImage;
}

LPCTSTR CFileListUI::GetSelectedImage() const
{
	return m_strSelectdImage;
}

void CFileListUI::SetNoSelectedImage(LPCTSTR strImage)  //设置非选中时的状态图
{
	m_strNoSelectedImage = strImage;
}

LPCTSTR CFileListUI::GetNoSelectedImage() const
{
	return m_strNoExpandImage;
}

void CFileListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (pstrName == _T("expandsize"))
	{
		SIZE cxyExpand = { 0 };
		LPTSTR pstr = NULL;
		cxyExpand.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		cxyExpand.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);     
		SetExpandImageSize(cxyExpand);
	}
	else if (pstrName == _T("selectedsize"))
	{
		SIZE cxySelected = { 0 };
		LPTSTR pstr = NULL;
		cxySelected.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		cxySelected.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);     
		SetSelctedImageSize(cxySelected);
	}
	else if( _tcscmp(pstrName, _T("expandimage")) == 0 ) SetExpandImage(pstrValue);
	else if( _tcscmp(pstrName, _T("noexpandimage")) == 0 ) SetNoExpandImage(pstrValue);
	else if( _tcscmp(pstrName, _T("selectedstatusimage")) == 0 ) SetSelctedImage(pstrValue);
	else if( _tcscmp(pstrName, _T("noselectedstatusimage")) == 0 ) SetNoSelectedImage(pstrValue);
	else if( _tcscmp(pstrName, _T("expandselect")) == 0 )
	{
		bool bExpandSel = true;
		if( _tcscmp(pstrValue, _T("false")) == 0)
		{
			bExpandSel = false;
		}
		SetExpandSelected(bExpandSel);
	}
	else if(_tcscmp(pstrName,_T("specificimage")) == 0) SetSpecificImage(pstrValue);
	else CListUI::SetAttribute(pstrName, pstrValue);
}

void CFileListUI::SetNodeExpandAttr(TreeNode *pNode, bool leafFlag /* = false */)
{
	if (pNode == NULL)
	{
		return;
	}

	pNode->LeafFlag = leafFlag;
	CStdString html_text;  
	html_text += _T("<x 8>"); 
	for( int i = 0; i < pNode->data()._level; ++i ) {
		html_text += _T("<x 24>"); 
	}
	if( !pNode->LeafFlag )
	{
		html_text += _T("<i ");
		html_text += m_strExpandIamge;
		html_text += _T(">");
	}
	else
	{
		html_text += _T("{x ");
		CStdString strSize;
		strSize.Format(_T("%d"), m_nExpandPos.cx);
		html_text += strSize;
		html_text += _T("}");
	}
	pNode->data()._expandText = html_text;
	pNode->data()._pListElement->SetText( pNode->data()._expandText + pNode->data()._selectionBoxText + pNode->data()._text );
	return;
}

TreeNode *CFileListUI::GetSelNode()
{

	TreeNode* begin;
	TreeNode* end;
	begin = _root->child(0);
	end = _root->get_last_child();
	if ((begin == NULL) || (end == NULL))
	{
		return NULL;
	}
	int level = _root->data()._level + 1; // root的直接孩子
	for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) 
	{
		CListLabelElementUI* control = static_cast<CListLabelElementUI *>(GetItemAt(i));
		if (control == NULL)
		{
			continue;
		}
		bool bSelect = control->IsSelected();
		TreeNode* childNode = (TreeNode*)control->GetTag();
		if (childNode == NULL)
		{
			continue;
		}
		if (bSelect)
		{
			return childNode;
		}
	}
	return NULL;
}

//bool CFileListUI::IsExpandSelect()
//{
//	TreeNode* begin;
//	TreeNode* end;
//	begin = _root->child(0);
//	end = _root->get_last_child();
//	if ((begin == NULL) || (end == NULL))
//	{
//		return false;
//	}
//	int level = _root->data()._level + 1; // root的直接孩子
//	for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) 
//	{
//		CListLabelElementUI* control = static_cast<CListLabelElementUI *>(GetItemAt(i));
//		if (control == NULL)
//		{
//			continue;
//		}
//		TreeNode* childNode = (TreeNode*)control->GetTag();
//		if(childNode->_data._expandselect)
//		{
//			return true;
//		}
//	}
//	return false;
//}

void CFileListUI::SetExpandSelected(bool bExpandSel /* = true */)
{
	m_bExpandSelected = bExpandSel;
}

bool CFileListUI::GetExpandSelected()
{
	return m_bExpandSelected;
}

void CFileListUI::ClearSelNode()
{
	TreeNode* begin;
	TreeNode* end;
	begin = _root->child(0);
	end = _root->get_last_child();
	if ((begin == NULL) || (end == NULL))
	{
		return;
	}
	int level = _root->data()._level + 1; // root的直接孩子
	for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) 
	{
		CListLabelElementUI* control = static_cast<CListLabelElementUI *>(GetItemAt(i)->GetInterface(_T("ListLabelElement")));
		if (control == NULL)
		{
			continue;
		}
		control->Select(false,false);
	}
}
void CFileListUI::SetToolTipFlag(bool bShowFlag/* = false*/)
{
	m_bSetToolTipFlag = bShowFlag;
}

void CFileListUI::SetEndEllipsisLength(int nNumbers)
{
	m_nEllipsisLength = (nNumbers < 0 || nNumbers > MAX_PATH) ? -1 : nNumbers;
}

LPCTSTR CFileListUI::GetSelPath()
{
	TreeNode* begin;
	TreeNode* end;
	begin = _root->child(0);
	end = _root->get_last_child();
	if ((begin == NULL) || (end == NULL))
	{
		return NULL;
	}
	int level = _root->data()._level + 1; // root的直接孩子
	for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) 
	{
		CListLabelElementUI* control = static_cast<CListLabelElementUI *>(GetItemAt(i)->GetInterface(_T("ListLabelElement")));
		if (control == NULL)
		{
			continue;
		}
		bool bSelect = control->IsSelected();
		TreeNode* childNode = (TreeNode*)control->GetTag();
		if (childNode == NULL)
		{
			continue;
		}
		if (bSelect)
		{
			return childNode->data()._path;
		}
	}
	return NULL;
}

void CFileListUI::SetSpecificImage( LPCTSTR strImage )
{
	m_strSpecificImage = strImage;
}

LPCTSTR CFileListUI::GetSpecificImage() const
{
	return m_strSpecificImage;
}

void CFileListUI::UnSelAllNode()
{
	TreeNode* begin;
	TreeNode* end;
	begin = _root->child(0);
	end = _root->get_last_child();
	if ((begin == NULL) || (end == NULL))
	{
		return ;
	}
	int level = _root->data()._level + 1; // root的直接孩子
	for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) 
	{
		CListLabelElementUI* control = static_cast<CListLabelElementUI *>(GetItemAt(i));
		if (control == NULL)
		{
			continue;
		}
		bool bSelect = control->IsSelected();
		if(bSelect)
		{
			control->Select(false);
		}
		
	}
}

CTreeElement::CTreeElement()
{
}

CTreeElement::~CTreeElement()
{

}

LPCTSTR CTreeElement::GetClass() const
{
	return _T("TreeElementUI");
}

UINT CTreeElement::GetControlFlags() const
{
	return UIFLAG_WANTRETURN;
}

LPVOID CTreeElement::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("ListItem")) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, _T("TreeElement")) == 0 ) return static_cast<CTreeElement*>(this);
	return CControlUI::GetInterface(pstrName);
}

void CTreeElement::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
	DrawItem(hDC, m_rcItem);
	DrawItemText(hDC, m_rcItem);
}

void CTreeElement::DrawItem(HDC hDC, const RECT& rcItem)
{
	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();
	if (pInfo == NULL)
	{
		return;
	}
	TreeNode *node = (TreeNode*)GetTag();
	if (node == NULL)
	{
		return;
	}
	
	DWORD iBackColor = 0;
	if( m_iIndex % 2 == 0 ) iBackColor = pInfo->dwBkColor;
	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		iBackColor = pInfo->dwHotBkColor;
	}
	CFileListUI *pControl = static_cast<CFileListUI *>(m_pOwner);
	if(pControl == NULL)
	{
		return;
	}
	if( IsSelected()) {
		
		iBackColor = pInfo->dwSelectedBkColor;
	}
	if( !IsEnabled() ) {
		iBackColor = pInfo->dwDisabledBkColor;
	}
	
	SIZE szExpand = {0};
	if (pControl != NULL)
	{
		szExpand = pControl->GetSelctedImageSize();
		pControl->SelectionBoxNodeCore(node, IsSelected());
	}

	if ( iBackColor != 0 ) {
		//计算文本所占的大小
		TFontInfo* pFontInfo = m_pManager->GetFontInfo(pInfo->nFont);
		if (pFontInfo == NULL)
		{
			pFontInfo = m_pManager->GetDefaultFontInfo();
		}
		HFONT holdFont = (HFONT)::SelectObject(hDC, pFontInfo->hFont);
		SIZE szSpace = { 0 };
		::GetTextExtentPoint32(hDC, (LPCTSTR)m_ShowText, m_ShowText.GetLength(), &szSpace);
		
		RECT rc = m_rcItem;
		rc.left += node->data()._LeftPos - 2;
		rc.right = rc.left + szSpace.cx + 10 + szExpand.cx;
		if (rc.right > m_rcItem.right)
		{
			rc.right = m_rcItem.right;
		}
		m_rcInvalidate = rc;
		CRenderEngine::DrawRect(hDC, rc, 1, 0xFF0000FF);
		CRenderEngine::DrawColor(hDC, m_pManager, rc, GetAdjustColor(iBackColor));
		::SelectObject(hDC, holdFont);
	}

	if( !IsEnabled() ) {
		if( !pInfo->sDisabledImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sDisabledImage) ) pInfo->sDisabledImage.Empty();
			else return;
		}
	}
	if( IsSelected() ) {
		if( !pInfo->sSelectedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage) ) pInfo->sSelectedImage.Empty();
			else return;
		}
	}
	if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !pInfo->sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sHotImage) ) pInfo->sHotImage.Empty();
			else return;
		}
	}

	if( !m_sBkImage.IsEmpty() ) {
		if( m_iIndex % 2 == 0 ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
		}
	}

	if( m_sBkImage.IsEmpty() ) {
		if( !pInfo->sBkImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
			else return;
		}
	}

	if ( pInfo->dwLineColor != 0 ) {
		RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
	}
}

void CTreeElement::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
		else CListElementUI::DoEvent(event);
		return;
	}
	CFileListUI *pControl = static_cast<CFileListUI *>(m_pOwner);
	if(pControl == NULL)
	{
		return;
	}
	TreeNode* childNode = (TreeNode*)pControl->GetItemAt(m_iIndex)->GetTag();
	if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_RBUTTONDOWN )
	{

		if( IsEnabled() ) {
			pControl->UnSelAllNode();
			POINT pt = { 0 };
			::GetCursorPos(&pt);

			::ScreenToClient(pControl->GetManager()->GetPaintWindow(), &pt);
			pt.x -= GetX();
			SIZE sz4Expander = pControl->GetExpanderSizeX(childNode);
			bool bSelected = false;
			if( pt.x >= sz4Expander.cx && pt.x < sz4Expander.cy )
			{
				bool bExpand = childNode->data()._expand;
				pControl->ExpandNode(childNode);
				if (pControl->GetExpandSelected())
				{
					bSelected = true;
					Select();
				}
			}
			else
			{
				bSelected = true;
				Select();
			}
			m_pManager->SendNotify(this, _T("itemclick"), DUILIB_LIST_ITEMCLICK, bSelected);


// 			if(!childNode->_data._expandselect)
// 			{
// 				pControl->ClearSelNode();
// 				Select();
// 			}
// 			else
// 			{
// 				childNode->_data._expandselect = false;
// 			}
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE ) 
	{
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP )
	{
		return;
	}
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( IsEnabled() ) {
			m_uButtonState |= UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if (event.Type == UIEVENT_DBLCLICK)
	{
		if( IsEnabled() ) {
			pControl->UnSelAllNode();
			if (event.ptMouse.x <= m_rcInvalidate.right)
 			{
				POINT pt = { 0 };
				::GetCursorPos(&pt);

				::ScreenToClient(pControl->GetManager()->GetPaintWindow(), &pt);
				pt.x -= GetX();
				SIZE sz4Expander = pControl->GetExpanderSizeX(childNode);
				bool bSelected = false;
				if( pt.x >= sz4Expander.cx && pt.x < sz4Expander.cy )
				{
					bool bExpand = childNode->data()._expand;
					pControl->ExpandNode(childNode);
					if (pControl->GetExpandSelected())
					{
						bSelected = true;
						Select();
					}
				}
				else
				{
					pControl->ExpandNode(childNode);
					bSelected = true;
					Select();
				}
 				m_pManager->SendNotify(this, _T("itemdbclick"), DUILIB_LIST_ITEM_DOUBLE_CLICK, bSelected);
// 				if(!childNode->_data._expandselect)
// 				{
// 					pControl->ClearSelNode();
// 					Select();
// 				}
// 				else
// 				{
// 					//m_bSelected = false;
// 					childNode->_data._expandselect = false;
// 				}
				Invalidate();
			}
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	CListElementUI::DoEvent(event);
}

void CTreeElement::SetShowText(LPCTSTR pstrText)
{
	if( m_ShowText == pstrText )
		return;
	m_ShowText = pstrText;
}
}