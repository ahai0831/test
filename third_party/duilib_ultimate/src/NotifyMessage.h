#ifndef __NOTIFYMESSAGE_H__
#define __NOTIFYMESSAGE_H__

#pragma once

namespace DuiLib {

	//窗口消息
#define  DUILIB_WND_INIT        0x01       //窗口初始化
#define  DUILIB_WND_SETFOCUS    0x02       //获取焦点
#define  DUILIB_WND_KILLFOCUS   0x04       //失去焦点

	//Active 控件消息
#define  DUILIB_ACTIVE_SHOW     0x10        //显示

	//Combo  控件消息
#define  DUILIB_COMBO_ITMESELECT   0x20     //下拉框选中一项，
#define  DUILIB_COMBO_DROPDOWN     0x21     //下拉框显示

	//Menu   控件消息
#define  DUILIB_MENU_CLICK         0x30      //菜单项被选中

	//Button 控件消息
#define  DUILIB_BN_CLICKED       0x40      //按钮被点击

	//Option 控件消息(即Radio控件)
#define  DUILIB_OP_SELECTCHANGED   0x50       //选中状态改变

	//Text   控件消息
#define  DUILIB_TEXT_LINK          0x60       //对超文本连接时，点击发送选中的项

	//Slider 控件消息
#define  DUILIB_SLIDER_VALUECHANGED   0x70    //滑动块当前值被改变 


	//Edit   控件消息
#define  DUILIB_EDIT_VK_RETURN        0x80		//按回车键
#define  DUILIB_EDIT_TEXTCHANGED      0x81		//文本内容被改变

	//ScrollBar 控件消息
#define  DUILIB_SCROLLBAR_SCROLL      0x90		//滚动条滚动

	//Tab    控件消息
#define  DUILIB_TAB_SELECT            0xA0		//Tab控件选中

	//Timer  定时器消息
#define  DUILIB_TIMER                 0xB0		//定时器消息，和WM_TIMER一致

	//list   控件消息
#define  DUILIB_LIST_ITEMSELECT       0xC0		//Item选中时发送，多选时，会重复发送
#define  DUILIB_LIST_ITEMACTIVE       0xC1		//双击Item
#define  DUILIB_LIST_ITEMCLICK        0xC2		//单击item
#define  DUILIB_LIST_HEADERCLICK	  0xC3		//头控件项点击
#define  DUILIB_LIST_LINK             0xC4		//项是文本时，有超文本连接，点击发送
#define  DUILIB_LIST_ITEM_MOVE        0xC5		//Item选中时发送，多选时，会重复发送
#define  DUILIB_LIST_MOVE_ITEM        0xC6		//Item选中时发送，多选时，会重复发送
#define  DUILIB_LIST_ITEM_RBUTTON_CLICK    0xC7		//右键单击item
#define  DUILIB_LIST_ITEM_DOUBLE_CLICK  0xC8     //双击消息
#define  DUILIB_LIST_ITEM_MOUSEENTER    0xCA		//鼠标进入消息
#define  DUILIB_LIST_ITEM_MOUSELEAVE    0xCB		//鼠标离开消息
#define  DUILIB_LIST_ITEMUNSELECT		0xCC      //Item非选中状态

#define  DUILIB_VERTICAL_LAYOUT_CLICK 0xC9     //纵向容器点击消息

//SlideSwitch 控件消息
#define  DUILIB_SLIDE_SWITCH		  0x71	  //滑动开关开启或关闭

	//GIF 控件消息
#define  DUILIB_GIF_SWITCH		  0xD0	  //GIF播放与停止			//www 2016.04.27
}

#endif // __NOTIFYMESSAGE_H__