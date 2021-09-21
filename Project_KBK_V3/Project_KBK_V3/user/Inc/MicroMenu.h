#ifndef _MICRO_MENU_H_
#define _MICRO_MENU_H_
#include "stm32f0xx.h"
#define MENU_MEM_ITEM	5 //������ ������� ����, ��� �� ��� �������� ���������� ������ �� ��������� �� ������ �����

#define  BUTTON_BACK  		  	0x1 
#define  BUTTON_DOWN    	  	0x2
#define  BUTTON_ENTER    			0x3
#define  BUTTON_UP    		  	0x4
#define  BUTTON_SOUND    		  0x5

#define DOUBLE_CALL					0
#define	ONE_ZERO					1
#include <stddef.h>

#define MENU_ITEM_STORAGE	const
#define _RM(x)	(x)
typedef uint32_t _uchar_menu;
typedef uint32_t _uint_menu;

typedef struct {
	const uint8_t *font; //������� ������������� �������
	const uint16_t *fontInfo;//0�_FF(width)_FF(ofset) ������ ������� � ��� ��������(���. ���.) � ������� const uint8_t *font
	const uint8_t WidthNum;//���� ������ ������� �����
	const uint8_t Height;
	uint16_t Color;
	uint16_t BgColor;
} TSettingfontDot;



typedef const struct Menu_Item 
{
	const struct Menu_Item *Next; 
	const struct Menu_Item *Previous; 
	const struct Menu_Item *Parent;
	const struct Menu_Item *Child; 
	void (*SelectCallback)(void);//������� ���������� ��� Menu_Navigate(NewMenu);
	void (*EnterCallback)(void); 
	void (*InitCallback)(void);
	void (*printItemCallback)(void); 

	const char *Text;

} Menu_Item_t;

typedef struct {
	_uchar_menu ySelMenu;		//y ���������� ����������� ������ ����
	_uchar_menu posSelMenu;		//������� ����������� ����
	_uchar_menu posPaintMenu;	//������� ��������� ����
	_uchar_menu BrenchCountMenu;//���-�� ������� ���� � �������� ����� ����
	
	_uchar_menu Repaint;		//��� ������ ����������� ������
	
	//��� �� ��� �������� ���������� ������ �� ��������� �� ������ �����
	Menu_Item_t *ptrGeneric_Menu;//���������� � menu_make_screen, ������������ ����� ������ �������� ����
	Menu_Item_t *ptrFirstItem[MENU_MEM_ITEM];//������ ������ ������� �� �������� �������� ����
	Menu_Item_t *ptrFirstItemPrint[MENU_MEM_ITEM];//������ ������ ������� �� �������� �������� ����
	
	_uchar_menu Level;//������� ����������� ptrFirstItem[mData.Level]
	
	_uchar_menu BegPosMenu; 	//c ������ ����(�������) �������� ����� �� ����� "����"
	_uchar_menu EndPosMenu; 	//����� "����" �� MaxStr
	_uchar_menu OldPosMenu;		//���������� ���������� �����
	_uchar_menu LastPosMenu[MENU_MEM_ITEM];		//���������� ���������� �����
	
	uint8_t OldTotalList[MENU_MEM_ITEM];
	uint8_t OldStrNum[MENU_MEM_ITEM];
	uint8_t OldListNum[MENU_MEM_ITEM];
	
	_uchar_menu MenuState;	//��������� ���� ���������/�����/��������������
	_uchar_menu SelectParametr;//����� �������������� ���������, �� ��� ����� �������� ��������� ���������
	_uint_menu DunamicTime;//����� �������� ������ ������������ �. DunamicTime*MS_SYSTICK_PERIOD
	
} FlgMnu;

// Creates a new menu item entry with the specified links and callbacks.

// #define MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, PrintItemFunc, Text) 
#define MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, InitFunc, PrintItemFunc, Text) \
	extern Menu_Item_t MENU_ITEM_STORAGE Next;     \
	extern Menu_Item_t MENU_ITEM_STORAGE Previous; \
	extern Menu_Item_t MENU_ITEM_STORAGE Parent;   \
	extern Menu_Item_t MENU_ITEM_STORAGE Child;  \
	Menu_Item_t MENU_ITEM_STORAGE Name = {&Next, &Previous, &Parent, &Child, SelectFunc, EnterFunc, InitFunc, PrintItemFunc, Text}

	#define MENU_PARENT         _RM(Menu_GetCurrentMenu()->Parent)
	#define MENU_CHILD          _RM(Menu_GetCurrentMenu()->Child)
	#define MENU_NEXT           _RM(Menu_GetCurrentMenu()->Next)
	#define MENU_PREVIOUS       _RM(Menu_GetCurrentMenu()->Previous)

	// ����������
	// Null menu entry ��������� � MicroMenu.� = Menu_Item_t NULL_MENU = {0};
	extern Menu_Item_t MENU_ITEM_STORAGE NULL_MENU;
	
	// ��������� �� ������� ������������ � ����
	// ��������� �� �������, ������� ������������� ����� ����
	extern void 	(*menu_item_print)(uint8_t pos, const char *s);
	extern uint8_t	(*menu_item_edit)(void);//��������� �� ������� ��� �������������� ����������	
	extern void 	(*dunamic_data_print)(void);//��������� �� ������� ��� ������ ������������ ������
	extern void 	(*exit_callback_f)(void);//��������� �� ������� ������������� ��� ������ �� �������������� ������ ����

	// �������
	Menu_Item_t* Menu_GetCurrentMenu(void);//������� ��������� �� ������� ����
	void Menu_Navigate(Menu_Item_t* const NewMenu);//��������� CurrentMenuItem �� ����� ����
	void Menu_SetGenericWriteCallback(void (*WriteFunc)(void));//��������� �������, ������� ����� �������� ����, ��� ������ Menu_Navigate
	
	_uchar_menu Menu_InitCurrentItem(void);//��� ������� ���������� InitCallback ���������� 1 ����� 0
	_uchar_menu Menu_InitChildItem(void);//���� ���� ������� ����� 1 (+ ���������� InitCallback �������, ���� �� �����) ����� 0
	_uchar_menu Menu_EnterCallback(void);//��������� �� ������� Enter(������ Set), ���� EnterCallback ����� ����� 1 ����� 0
	void select_callback_exec(void);//�������������� ���������� SelectCallback �������
	_uchar_menu menu_process(_uchar_menu Key);//�������� �� ���� (����� ������ � ������ ������� ������)
	
	//--------------------------------------------------------------
	//������� ������ � ����������� ����, ������ �� ���������� FlgMnu
	void menu_set_repaint(void);//������� ��������� � ����������� ����
	void menu_set_repaint_newlevel(void);//������� ��������� � ����������� ���� � ���������� ����� ����� ����
	_uchar_menu menu_get_repaint(void);
	
	_uchar_menu menu_get_y(void);//y ���������� ����������� ������ ����
	_uchar_menu menu_get_pos(void);//������� ������� ����������� ����
	_uchar_menu menu_get_pos_paint(void);//�������� ����� ��������� ���������
	_uchar_menu menu_get_BrenchCount(void);//���-�� ������� ���� � �������� ����� ����
	Menu_Item_t *menu_get_ptrGeneric(void);//������� ��������� �� ������� ����(�����) � �����
	void menu_set_ptrGeneric(Menu_Item_t *FirstItem);//�������� ��������� �� ������� ����(�����) � �����
	void menu_store_level(void);//��������� "���������" ���. ������
	void menu_back_level(void);//������������ "���������" ����������� ������
	
	void m_clsSelP(void);//�������� ����� �������������� ���������
	_uchar_menu m_getSelP(void);//�������� ����� �������������� ���������
	void m_incSelP(void);//��������� ����� �������������� ���������
	void menu_set_dunamic_time(_uint_menu t);//���������� ����� ����� ������� ����� ��������� dunamic_data_print
	_uint_menu menu_get_dunamic_time(void);//��������� ����� ����� ������� ����� ��������� dunamic_data_print
	_uchar_menu menu_get_state(void);//������ ��������� ����
	
	void menu_start(void);//������������� ����
	void menu_exit(void);//����� �� ����
	//--------------------------------------------------------------
	
	//���������� ��������� �� �. menu_item_print ������ ������ �������
	void menu_set_print_item_callback(void (*f)(uint8_t pos, const char *s));
	
	void menu_make_screen(Menu_Item_t* const Menu);//������� �������� ����
	void Vise_Versa_Menu(Menu_Item_t* const NewMenu);
	void Back_Through_Pass (uint8_t access);
	void NextParam (uint8_t numbers_of_param, void (*func)(void), Menu_Item_t* const Menu, uint8_t edit);




	enum TMenuSTATE{//��������� ����
		ST_MENU_IDLE=	0,
		ST_MENU_NAVIGATE,
		ST_MENU_EDIT
	};
	
	void Set_Hided_Menu (uint8_t data);
	
	void show_pop_up_windiw(void (*func1)(void), void (*func2)(void), void (*func3)(void));
	
//������ � ����� ������� InitFunc->MenuWriteFunc->PrintFunc->SelectFunc
#endif


