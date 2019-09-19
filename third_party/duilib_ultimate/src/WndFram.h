#ifndef __WNDFRAM_H__
#define __WNDFRAM_H__

#pragma once

enum Accelerator_Type
{
	Accelerator_Copy = 1000,			// 拷贝
	Accelerator_Cut = 1001,				// 剪切
	Accelerator_Paste = 1002,		    // 粘帖
	Accelerator_Delete = 1003,			// 删除
	Accelerator_SelectAll = 1004,		// 全选
	Accelerator_Enter = 1005,           // 回车键
	Accelerator_F5 = 1006,              // F5键
	Accelerator_Backspace = 1007,       // 回退键
	Accelerator_ESC = 1008,             // ESC
	Accelerator_VK_Left = 1009,         // ->键
	Accelerator_VK_Right = 1010,         // <-键
	Accelerator_VK_Up = 1011,         // 向上方向键
	Accelerator_VK_Down = 1012,         // 向下方向键
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

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);  //系统消息处理函数

	virtual void HandleKeyMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);   //处理键盘消息，转为快捷键消息

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
