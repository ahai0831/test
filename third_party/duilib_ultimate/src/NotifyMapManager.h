#ifndef __NOTIFYMAPMANAGER_H__
#define __NOTIFYMAPMANAGER_H__
#pragma once

namespace DuiLib {

//通知消息和处理函数匹配结构
typedef void (*pBtnFunc)(TNotifyUI &pMsg);

typedef struct tagTNofifyMap
{
	LPCTSTR wsWndClassName;    //通知消息的窗口类名
	DWORD   dwMessageID;       //通知消息
	LPCTSTR wsControlName;     //控件名称
	void (*PFunc)(TNotifyUI &pMsg);          //处理函数名称
}TNotifyMap;

void UILIB_API SetMap(LPCTSTR wsWndClassName, DWORD dwMessageID, LPCTSTR wsControlName, void(*pFunc)(TNotifyUI &pMsg));

#define DUILIB_ON_NOTIFY(wsWndClassName, dwMessageID, wsControlName, pBtnFunc) \
	SetMap(wsWndClassName, dwMessageID, wsControlName, pBtnFunc);

class UILIB_API CNotifyMapManager
{
public:
	CNotifyMapManager();
	~CNotifyMapManager();
	static void AddNotifyMap(LPCTSTR wsWndClassName, DWORD dwMessageID, LPCTSTR wsControlName,void(*pFunc)(TNotifyUI &pMsg)); //添加map到类中
	static const CStdPtrArray& GetNotifyMap();   //获取整个map数据
	static void RemoveNotifyMap();   //清空数据
	static pBtnFunc FindFunc(DWORD dwMessageID, LPCTSTR wsControlName, LPCTSTR wsWndClassName = NULL);   //根据消息id 和控件名，查找是否有指定了处理函数，有则返回

private:
	static CStdPtrArray g_TNotifyMap;
};

}

#endif // _NOTIFYMAPMANAGER_H_