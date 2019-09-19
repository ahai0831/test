#ifndef __NOTIFYMESSAGE_H__
#define __NOTIFYMESSAGE_H__

#pragma once

namespace DuiLib {

	//������Ϣ
#define  DUILIB_WND_INIT        0x01       //���ڳ�ʼ��
#define  DUILIB_WND_SETFOCUS    0x02       //��ȡ����
#define  DUILIB_WND_KILLFOCUS   0x04       //ʧȥ����

	//Active �ؼ���Ϣ
#define  DUILIB_ACTIVE_SHOW     0x10        //��ʾ

	//Combo  �ؼ���Ϣ
#define  DUILIB_COMBO_ITMESELECT   0x20     //������ѡ��һ�
#define  DUILIB_COMBO_DROPDOWN     0x21     //��������ʾ

	//Menu   �ؼ���Ϣ
#define  DUILIB_MENU_CLICK         0x30      //�˵��ѡ��

	//Button �ؼ���Ϣ
#define  DUILIB_BN_CLICKED       0x40      //��ť�����

	//Option �ؼ���Ϣ(��Radio�ؼ�)
#define  DUILIB_OP_SELECTCHANGED   0x50       //ѡ��״̬�ı�

	//Text   �ؼ���Ϣ
#define  DUILIB_TEXT_LINK          0x60       //�Գ��ı�����ʱ���������ѡ�е���

	//Slider �ؼ���Ϣ
#define  DUILIB_SLIDER_VALUECHANGED   0x70    //�����鵱ǰֵ���ı� 


	//Edit   �ؼ���Ϣ
#define  DUILIB_EDIT_VK_RETURN        0x80		//���س���
#define  DUILIB_EDIT_TEXTCHANGED      0x81		//�ı����ݱ��ı�

	//ScrollBar �ؼ���Ϣ
#define  DUILIB_SCROLLBAR_SCROLL      0x90		//����������

	//Tab    �ؼ���Ϣ
#define  DUILIB_TAB_SELECT            0xA0		//Tab�ؼ�ѡ��

	//Timer  ��ʱ����Ϣ
#define  DUILIB_TIMER                 0xB0		//��ʱ����Ϣ����WM_TIMERһ��

	//list   �ؼ���Ϣ
#define  DUILIB_LIST_ITEMSELECT       0xC0		//Itemѡ��ʱ���ͣ���ѡʱ�����ظ�����
#define  DUILIB_LIST_ITEMACTIVE       0xC1		//˫��Item
#define  DUILIB_LIST_ITEMCLICK        0xC2		//����item
#define  DUILIB_LIST_HEADERCLICK	  0xC3		//ͷ�ؼ�����
#define  DUILIB_LIST_LINK             0xC4		//�����ı�ʱ���г��ı����ӣ��������
#define  DUILIB_LIST_ITEM_MOVE        0xC5		//Itemѡ��ʱ���ͣ���ѡʱ�����ظ�����
#define  DUILIB_LIST_MOVE_ITEM        0xC6		//Itemѡ��ʱ���ͣ���ѡʱ�����ظ�����
#define  DUILIB_LIST_ITEM_RBUTTON_CLICK    0xC7		//�Ҽ�����item
#define  DUILIB_LIST_ITEM_DOUBLE_CLICK  0xC8     //˫����Ϣ
#define  DUILIB_LIST_ITEM_MOUSEENTER    0xCA		//��������Ϣ
#define  DUILIB_LIST_ITEM_MOUSELEAVE    0xCB		//����뿪��Ϣ
#define  DUILIB_LIST_ITEMUNSELECT		0xCC      //Item��ѡ��״̬

#define  DUILIB_VERTICAL_LAYOUT_CLICK 0xC9     //�������������Ϣ

//SlideSwitch �ؼ���Ϣ
#define  DUILIB_SLIDE_SWITCH		  0x71	  //�������ؿ�����ر�

	//GIF �ؼ���Ϣ
#define  DUILIB_GIF_SWITCH		  0xD0	  //GIF������ֹͣ			//www 2016.04.27
}

#endif // __NOTIFYMESSAGE_H__