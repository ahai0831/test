#ifndef __TREELIST_H__
#define __TREELIST_H__
#pragma once

namespace DuiLib {

struct INodeData
{
	int _level;               
	bool _child_visible;
	CControlUI* _pListElement;
};

class INode
{
	typedef std::vector <INode*>	Children;
	Children	_children;
	INode*		_parent;
	INodeData    _data;

private:
	void set_parent(INode* parent) { _parent = parent; }

public:
	INode() : _parent (NULL) {}
	explicit INode(INodeData t) : _data (t), _parent (NULL) {}
	INode(INodeData t, INode* parent)	: _data (t), _parent (parent) {}
	~INode() 
	{
		for (int i = 0; i < num_children(); i++)
			delete _children[i]; 
	}
	INodeData& data() { return _data; }	
	int num_children() const { return _children.size(); }
	INode* child(int i)	{ return _children[i]; }
	INode* parent() { return ( _parent);	}
	bool has_children() const {	return num_children() > 0; }
	void add_child(INode* child) 
	{
		child->set_parent(this); 
		_children.push_back(child); 
	}
	void remove_child(INode* child)
	{
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
	INode* get_last_child()
	{
		if( has_children() )
		{
			return child(num_children() - 1)->get_last_child();
		}
		else return this;
	}
};	

class UILIB_API CTreeListUI : public CListUIEx
{
public:
	CTreeListUI(void);
	~CTreeListUI(void);
};

}
#endif  //__TREELIST_H__
