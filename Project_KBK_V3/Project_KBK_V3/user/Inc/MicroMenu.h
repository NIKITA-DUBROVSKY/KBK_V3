#ifndef _MICRO_MENU_H_
#define _MICRO_MENU_H_
#include "stm32f0xx.h"
#define MENU_MEM_ITEM	5 //память уровней меню, что бы при возврате выделенные пункты не смещались на первое место

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
	const uint8_t *font; //битовое представление символа
	const uint16_t *fontInfo;//0х_FF(width)_FF(ofset) ширина символа и его смещение(нач. поз.) в массиве const uint8_t *font
	const uint8_t WidthNum;//макс ширина символа цифры
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
	void (*SelectCallback)(void);//функция выполнится при Menu_Navigate(NewMenu);
	void (*EnterCallback)(void); 
	void (*InitCallback)(void);
	void (*printItemCallback)(void); 

	const char *Text;

} Menu_Item_t;

typedef struct {
	_uchar_menu ySelMenu;		//y координата выделенного пункта меню
	_uchar_menu posSelMenu;		//позиция выделенного меню
	_uchar_menu posPaintMenu;	//позиция рисуемого меню
	_uchar_menu BrenchCountMenu;//кол-во пунктов меню в активной ветви меню
	
	_uchar_menu Repaint;		//для полной перерисовки экрана
	
	//что бы при возврате выделенные пункты не смещались на первое место
	Menu_Item_t *ptrGeneric_Menu;//передается в menu_make_screen, относительно этого пункта строится меню
	Menu_Item_t *ptrFirstItem[MENU_MEM_ITEM];//память первых пунктов от которого строится меню
	Menu_Item_t *ptrFirstItemPrint[MENU_MEM_ITEM];//память первых пунктов от которого строится меню
	
	_uchar_menu Level;//уровень запоминания ptrFirstItem[mData.Level]
	
	_uchar_menu BegPosMenu; 	//c какого меню(позиции) начинать вывод на экран "Окна"
	_uchar_menu EndPosMenu; 	//конец "Окна" из MaxStr
	_uchar_menu OldPosMenu;		//предыдущий выделенный пункт
	_uchar_menu LastPosMenu[MENU_MEM_ITEM];		//предыдущий выделенный пункт
	
	uint8_t OldTotalList[MENU_MEM_ITEM];
	uint8_t OldStrNum[MENU_MEM_ITEM];
	uint8_t OldListNum[MENU_MEM_ITEM];
	
	_uchar_menu MenuState;	//Состояния меню выключено/выбор/редактирование
	_uchar_menu SelectParametr;//номер редактируемого параметра, по нем также цветовое выделение параметра
	_uint_menu DunamicTime;//время задержки вывода динамической ф. DunamicTime*MS_SYSTICK_PERIOD
	
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

	// ПЕРЕМЕННЫЕ
	// Null menu entry определен в MicroMenu.с = Menu_Item_t NULL_MENU = {0};
	extern Menu_Item_t MENU_ITEM_STORAGE NULL_MENU;
	
	// указатели на функции используемые в меню
	// указатель на функцию, которая прорисовывает пункт меню
	extern void 	(*menu_item_print)(uint8_t pos, const char *s);
	extern uint8_t	(*menu_item_edit)(void);//указатель на функцию для редактирования параметров	
	extern void 	(*dunamic_data_print)(void);//указатель на функцию для вывода динамических данных
	extern void 	(*exit_callback_f)(void);//указатель на функцию выполняющуюся при выходе из редактирования пункта меню

	// ФУНКЦИИ
	Menu_Item_t* Menu_GetCurrentMenu(void);//вернуть указатель на текущее меню
	void Menu_Navigate(Menu_Item_t* const NewMenu);//указатель CurrentMenuItem на новое меню
	void Menu_SetGenericWriteCallback(void (*WriteFunc)(void));//назначает функцию, которая будет рисовать меню, при вызове Menu_Navigate
	
	_uchar_menu Menu_InitCurrentItem(void);//при упешном выполнении InitCallback возвращает 1 иначе 0
	_uchar_menu Menu_InitChildItem(void);//если есть потомок возвр 1 (+ выполнется InitCallback потомка, если он задан) иначе 0
	_uchar_menu Menu_EnterCallback(void);//выпонение по нажатию Enter(вернее Set), если EnterCallback задан возвр 1 иначе 0
	void select_callback_exec(void);//принудительное выполнение SelectCallback функции
	_uchar_menu menu_process(_uchar_menu Key);//хождение по меню (здесь работа с кодами нажатий кнопок)
	
	//--------------------------------------------------------------
	//функции работы с переменными меню, работа со структурой FlgMnu
	void menu_set_repaint(void);//послать сообщение о перерисовки меню
	void menu_set_repaint_newlevel(void);//послать сообщение о перерисовки меню и построения новой ветки меню
	_uchar_menu menu_get_repaint(void);
	
	_uchar_menu menu_get_y(void);//y координата выделенного пункта меню
	_uchar_menu menu_get_pos(void);//вернуть позицию выделенного меню
	_uchar_menu menu_get_pos_paint(void);//получить номер рисуемого параметра
	_uchar_menu menu_get_BrenchCount(void);//кол-во пунктов меню в активной ветви меню
	Menu_Item_t *menu_get_ptrGeneric(void);//вернуть указатель на верхнее меню(пункт) в ветке
	void menu_set_ptrGeneric(Menu_Item_t *FirstItem);//изменить указатель на верхнее меню(пункт) в ветке
	void menu_store_level(void);//сохранить "выделение" тек. уровня
	void menu_back_level(void);//восстановить "выделение" предыдущего уровня
	
	void m_clsSelP(void);//сбросить номер редактируемого параметра
	_uchar_menu m_getSelP(void);//получить номер редактируемого параметра
	void m_incSelP(void);//увеличить номер редактируемого параметра
	void menu_set_dunamic_time(_uint_menu t);//установить время через которое будет вызыватся dunamic_data_print
	_uint_menu menu_get_dunamic_time(void);//прочитать время через которое будет вызыватся dunamic_data_print
	_uchar_menu menu_get_state(void);//узнать состояние меню
	
	void menu_start(void);//инициализация меню
	void menu_exit(void);//выйти из меню
	//--------------------------------------------------------------
	
	//присвоение указателю на ф. menu_item_print адреса нужной функции
	void menu_set_print_item_callback(void (*f)(uint8_t pos, const char *s));
	
	void menu_make_screen(Menu_Item_t* const Menu);//функции рисующие меню
	void Vise_Versa_Menu(Menu_Item_t* const NewMenu);
	void Back_Through_Pass (uint8_t access);
	void NextParam (uint8_t numbers_of_param, void (*func)(void), Menu_Item_t* const Menu, uint8_t edit);




	enum TMenuSTATE{//состояния меню
		ST_MENU_IDLE=	0,
		ST_MENU_NAVIGATE,
		ST_MENU_EDIT
	};
	
	void Set_Hided_Menu (uint8_t data);
	
	void show_pop_up_windiw(void (*func1)(void), void (*func2)(void), void (*func3)(void));
	
//вызывы в таком порядке InitFunc->MenuWriteFunc->PrintFunc->SelectFunc
#endif


