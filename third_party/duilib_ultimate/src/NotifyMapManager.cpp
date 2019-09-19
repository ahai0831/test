#include "stdafx.h"
#include "NotifyMapManager.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//
CStdPtrArray CNotifyMapManager::g_TNotifyMap;

CNotifyMapManager::CNotifyMapManager()
{

}

CNotifyMapManager::~CNotifyMapManager()
{
	RemoveNotifyMap();
}

void CNotifyMapManager::AddNotifyMap(LPCTSTR wsWndClassName, DWORD dwMessageID, LPCTSTR wsControlName, void(*pFunc)(TNotifyUI &pMsg))  //������Ϣ����map
{
	TNotifyMap *NotifyMap = NULL;
	NotifyMap = (TNotifyMap *)calloc(1, sizeof(TNotifyMap));
	if (NotifyMap == NULL)
	{
		return;
	}
	memset(NotifyMap, 0x00, sizeof(TNotifyMap));
	NotifyMap->wsWndClassName = wsWndClassName;
	NotifyMap->dwMessageID = dwMessageID;
	NotifyMap->wsControlName = wsControlName;
	NotifyMap->PFunc = (static_cast<pBtnFunc>(pFunc));
	g_TNotifyMap.Add(NotifyMap);	
}

const CStdPtrArray& CNotifyMapManager::GetNotifyMap()
{
	return g_TNotifyMap;
}

//�����Ϣ����map
void CNotifyMapManager::RemoveNotifyMap()   
{
	if (g_TNotifyMap.GetSize() == 0)
	{
		return;
	}
	for (int i = 0; i< g_TNotifyMap.GetSize(); i++)
	{
		TNotifyMap *PNotifyMap = static_cast<TNotifyMap*>(g_TNotifyMap[i]);
		if (PNotifyMap != NULL)
		{
			free(PNotifyMap);
			PNotifyMap = NULL;
		}
	}
	g_TNotifyMap.Empty();
}

//������Ϣ�Ϳؼ����Ʋ�����Ϣ�Ĵ�������ַ
pBtnFunc CNotifyMapManager::FindFunc(DWORD dwMessageID, LPCTSTR wsControlName, LPCTSTR wsWndClassName /* = NULL */)
{
	if (g_TNotifyMap.GetSize() == 0)
	{
		return NULL;
	}
	for (int i = 0; i< g_TNotifyMap.GetSize(); i++)
	{
		TNotifyMap *PNotifyMap = static_cast<TNotifyMap*>(g_TNotifyMap[i]);
		if ((PNotifyMap->dwMessageID == dwMessageID) && (_tcscmp(PNotifyMap->wsControlName, wsControlName) == 0))
		{
			if (wsWndClassName == NULL)
			{
				return PNotifyMap->PFunc;
			}

			//�ǿ�
			if (_tcscmp(PNotifyMap->wsWndClassName, wsWndClassName) == 0)
			{
				return PNotifyMap->PFunc;
			}
		}
	}
	return NULL;
}

void SetMap(LPCTSTR wsWndClassName, DWORD dwMessageID, LPCTSTR wsControlName, void(*pFunc)(TNotifyUI &pMsg))
{
	CNotifyMapManager::AddNotifyMap(wsWndClassName, dwMessageID, wsControlName, pFunc);
}
}