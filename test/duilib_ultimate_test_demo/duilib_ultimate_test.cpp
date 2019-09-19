//#include "StdAfx.h"
#include "UIlib.h"

using namespace DuiLib;

class CDuiFrameWnd : public CWindowWnd, public INotifyUI {
 public:
  virtual LPCTSTR GetWindowClassName() const { return _T("DUIMainFrame"); }
  virtual void Notify(TNotifyUI& msg) {}

  virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT lRes = 0;

    if (uMsg == WM_CREATE) {
      CControlUI* pWnd = new CButtonUI;
      pWnd->SetText(_T("Hello World"));
      // pWnd->SetBkImage(_T("E:/Desktop/Snipaste_2019-09-19_14-50-15.png"));
      pWnd->SetBkColor(0xFF00FF00);

      m_PaintManager.Init(m_hWnd);
      m_PaintManager.AttachDialog(pWnd);
      return lRes;
    }

    if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) {
      return lRes;
    }

    return __super::HandleMessage(uMsg, wParam, lParam);
  }

 protected:
  CPaintManagerUI m_PaintManager;
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  CPaintManagerUI::SetInstance(hInstance);

  CDuiFrameWnd duiFrame;
  duiFrame.Create(NULL, _T("duilib_ultimate_test"), UI_WNDSTYLE_FRAME,
                  WS_EX_WINDOWEDGE);
  duiFrame.ShowModal();
  return 0;
}
