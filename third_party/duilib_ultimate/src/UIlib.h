
#ifdef UILIB_STATIC
#define UILIB_API
#pragma comment(lib,"oledlg.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib") 
#pragma comment(lib,"Riched20.lib")
#else
#ifdef UILIB_EXPORTS
#define UILIB_API __declspec(dllexport)
#else
#define UILIB_API __declspec(dllimport)
#endif
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stddef.h>
#include <richedit.h>
#include <tchar.h>
#include <assert.h>
#include <crtdbg.h>
#include <malloc.h>

#include "UIBase.h"
#include "UIManager.h"
#include "UIDelegate.h"
#include "UIControl.h"
#include "UIContainer.h"
#include "UICommonControls.h"
#include "UIList.h"
#include "UICombo.h"
#include "UIActiveX.h"
#include "UIRichEdit.h"

#include "UIMarkup.h"
#include "UIDlgBuilder.h"
#include "UIRender.h"
#include "NotifyMapManager.h"
#include "NotifyMessage.h"
#include "WndShadow.h"
#include "GIF.h"
#include "UIAnimation.h"
#include "AnimationTabLayoutUI.h"
#include "AnimationGIF.h"
#include "AnimationBar.h"
#include "ButtonAniUI.h"
#include "ButtonElement.h"
#include "OptionDelayUI.h"
#include "WndFram.h"
#include "FileListUI.h"
#include "IConListUIEx.h"
#include "ListUIEx.h"
#include "ComboUIEx.h"
#include "TwoFgPorgress.h"
#include "ListElementEx.h"
#include "URLBar.h"
#include "TextButton.h"
#include "ButtonNonUI.h"
#include "UIWebBrowser.h"
#include "VerticalLayoutExUI.h"
#include "SlideSwitchUI.h"
#include "OptionElementUI.h"
#include "AnimationCtrl.h"
#include "ChildHorizontalLayout.h"
#include "TextLabel.h"
#include "UIEditEx.h"

//新增动画功能实现
#include "Internal.h"
#include "UIAnim.h"
#undef  GetNextSibling    // cancel the marco for cef
#include "UITreeView.h"
#include "UICustomizationOption.h"
#include "UISegmetation.h"
#include "LabelElement.h"
#include "UIButtonElementEx.h"
#include "UIBackgroundAnimationControl.h"
#include "UIStepperControl.h"
#include "UItreeControl.h"