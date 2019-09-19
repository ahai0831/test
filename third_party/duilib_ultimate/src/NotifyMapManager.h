#ifndef __NOTIFYMAPMANAGER_H__
#define __NOTIFYMAPMANAGER_H__
#pragma once

namespace DuiLib {

//֪ͨ��Ϣ�ʹ�����ƥ��ṹ
typedef void (*pBtnFunc)(TNotifyUI &pMsg);

typedef struct tagTNofifyMap
{
	LPCTSTR wsWndClassName;    //֪ͨ��Ϣ�Ĵ�������
	DWORD   dwMessageID;       //֪ͨ��Ϣ
	LPCTSTR wsControlName;     //�ؼ�����
	void (*PFunc)(TNotifyUI &pMsg);          //����������
}TNotifyMap;

void UILIB_API SetMap(LPCTSTR wsWndClassName, DWORD dwMessageID, LPCTSTR wsControlName, void(*pFunc)(TNotifyUI &pMsg));

#define DUILIB_ON_NOTIFY(wsWndClassName, dwMessageID, wsControlName, pBtnFunc) \
	SetMap(wsWndClassName, dwMessageID, wsControlName, pBtnFunc);

class UILIB_API CNotifyMapManager
{
public:
	CNotifyMapManager();
	~CNotifyMapManager();
	static void AddNotifyMap(LPCTSTR wsWndClassName, DWORD dwMessageID, LPCTSTR wsControlName,void(*pFunc)(TNotifyUI &pMsg)); //���map������
	static const CStdPtrArray& GetNotifyMap();   //��ȡ����map����
	static void RemoveNotifyMap();   //�������
	static pBtnFunc FindFunc(DWORD dwMessageID, LPCTSTR wsControlName, LPCTSTR wsWndClassName = NULL);   //������Ϣid �Ϳؼ����������Ƿ���ָ���˴����������򷵻�

private:
	static CStdPtrArray g_TNotifyMap;
};

}

#endif // _NOTIFYMAPMANAGER_H_