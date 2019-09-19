#ifndef __WNDFRAM_H__
#define __WNDFRAM_H__

#pragma once

enum Accelerator_Type
{
	Accelerator_Copy = 1000,			// ����
	Accelerator_Cut = 1001,				// ����
	Accelerator_Paste = 1002,		    // ճ��
	Accelerator_Delete = 1003,			// ɾ��
	Accelerator_SelectAll = 1004,		// ȫѡ
	Accelerator_Enter = 1005,           // �س���
	Accelerator_F5 = 1006,              // F5��
	Accelerator_Backspace = 1007,       // ���˼�
	Accelerator_ESC = 1008,             // ESC
	Accelerator_VK_Left = 1009,         // ->��
	Accelerator_VK_Right = 1010,         // <-��
	Accelerator_VK_Up = 1011,         // ���Ϸ����
	Accelerator_VK_Down = 1012,         // ���·����
};

namespace DuiLib {
class IAcceleratorsManager
{
public:
};

class UILIB_API CWndFram :public CWindowWnd, public IAcceleratorsManager
{
public:
	CWndFram(void);
	~CWndFram(void);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);  //ϵͳ��Ϣ������

	virtual void HandleKeyMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);   //���������Ϣ��תΪ��ݼ���Ϣ

protected:
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	CPaintManagerUI m_pm;
};

}
#endif  //__WNDFRAM_H__
