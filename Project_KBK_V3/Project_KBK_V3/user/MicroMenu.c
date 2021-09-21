#include "MicroMenu.h"
#include "math.h"
extern Menu_Item_t* main_CurrentMenu;
void (*EndEnteringParams)(void) = NULL;
extern void lcd44780_ClearLCD(void);
extern void lcd44780_SetLCDPosition(char x, char y);
extern void	lcd44780_ShowStr(const char *s);
int8_t str_number = 0, Cycle_Flag_done = 1;
uint8_t debug, list = 0, total_list = 0;
uint8_t old_list_numb, save, load;
_uchar_menu Position = 0;
extern uint8_t pass_flag;
Menu_Item_t* NextMenu;
Menu_Item_t* PrevMenu;
Menu_Item_t* CurrMenu;
Menu_Item_t* debugMenu;
Menu_Item_t* SavePassMenu;
uint8_t Menu_Level;
extern uint8_t m_number;

uint8_t edit_init = 1;

//void (*UserCallBack)(void);
//����� 
#define MENU_REPAINT	0x01 //������������ ����
#define MENU_REPAINT_NEW_LEVEL	0x02 //������������ ���� + ������������� ����� ����� ����

Menu_Item_t MENU_ITEM_STORAGE NULL_MENU = {0};
extern uint8_t PRESS_Button, HOLD_Button;

static Menu_Item_t* CurrentMenuItem = &NULL_MENU;
void (*UserCallBack)(void);
uint8_t nParam = 1;
uint8_t pop_flag = 0;

// ���������� ����������.
static FlgMnu mData;//����� ����
void (*Enter_edit)(void);

// ��������� �� ������� ������������ � ����
void	(*menu_item_print)(uint8_t pos, const char *s)=NULL;//��������� �� ������� ��� ��������� ������ ����
uint8_t	(*menu_item_edit)(void);//��������� �� ������� ��� �������������� ����������	
void 	(*dunamic_data_print)(void);//��������� �� ������� ������ ������������ ������
void 	(*dunamic_data_print_old)(void);
void 	(*exit_callback_f)(void);//��������� �� ������� ������������� ��� ������ �� �������������� ������ ����
//-----------------------------------------
static void (*MenuWriteFunc)(void) = NULL;//���������� ��������� �� �. 
//-----------------------------------------------------------------------------------------------------------------
//****************������ � �����������***********************
void m_incSelP(void){//��������� ����� �������������� ���������
	++mData.SelectParametr;
}

void menu_set_repaint(void){//������� ��������� � ����������� ����
	mData.Repaint=MENU_REPAINT;
}

void menu_set_repaint_newlevel(void){//������� ��������� � ����������� ���� � ���������� ����� ����� ����
	mData.Repaint=MENU_REPAINT_NEW_LEVEL;
}
_uchar_menu menu_get_repaint(void){//������ � ������������� ����������� ����
	return mData.Repaint;
}
_uchar_menu menu_get_pos(void){ //������� ������� ����������� ����
	return mData.posSelMenu;
}

Menu_Item_t *menu_get_ptrGeneric(void){//������� ��������� �� ������� ����(�����) � �����
	return mData.ptrGeneric_Menu;
}

void menu_set_ptrGeneric(Menu_Item_t *FirstItem){//�������� ��������� �� ������� ����(�����) � �����
	mData.ptrGeneric_Menu=FirstItem;
	mData.Level=0;//���������� ������ �������
	mData.ptrFirstItem[0]=_RM(FirstItem->Parent);//�������� � ������ �������

	if ((FirstItem == &NULL_MENU) || (FirstItem == NULL))
		return;
	//���� ���� ������������� ����� ����, �� ���������
	void (*InitCallback)(void) = _RM(FirstItem->InitCallback);

	if (InitCallback){
		InitCallback();
	}
}

void menu_store_level(void){
	if (mData.Level<MENU_MEM_ITEM)
	{
		mData.ptrFirstItem[mData.Level]=mData.ptrGeneric_Menu;//��������� ���������� 1� �������
		mData.ptrFirstItemPrint[mData.Level]=CurrMenu;//��������� ���������� 1� �������
	}
	
	
		
	mData.OldListNum[mData.Level] = list;
	mData.OldStrNum[mData.Level] = str_number;
	mData.OldTotalList[mData.Level] = total_list;
	mData.LastPosMenu[mData.Level] = Position;
	
	list = 0;
	str_number = 0;
	total_list = 0;
	mData.Level++;
	save = 1;
	load = 0;

	mData.ptrGeneric_Menu=MENU_CHILD;//������������ ����� ���� ����� �������� ���� � MenuWriteFunc() (�������� � menu_make_screen())
}

extern Menu_Item_t* main_HidedMenu1;
extern Menu_Item_t* main_HidedMenu2;

void Set_Hided_Menu (uint8_t data)
{
	if (data == 1)
	{
		mData.ptrGeneric_Menu = main_HidedMenu1;
		CurrentMenuItem = main_HidedMenu1;
	}
	if (data == 2)
	{
		mData.ptrGeneric_Menu = main_HidedMenu2;
		CurrentMenuItem = main_HidedMenu2;
	}

}

void menu_back_level(void){
	//mData.Repaint=MENU_REPAINT_NEW_LEVEL;//��� ������ ���������� ����� ����� ����
	
	if (mData.Level)
		mData.Level--;
	if (mData.Level<MENU_MEM_ITEM)
		mData.ptrGeneric_Menu=mData.ptrFirstItem[mData.Level];
	else
		mData.ptrGeneric_Menu=MENU_PARENT;
	
	if (UserCallBack && mData.Level == 2) 
	{
		CurrentMenuItem = mData.ptrGeneric_Menu;
		UserCallBack = NULL;
	}
	
	list = mData.OldListNum[mData.Level];
	str_number = mData.OldStrNum[mData.Level];
	total_list = mData.OldTotalList[mData.Level];
	mData.OldPosMenu = mData.LastPosMenu[mData.Level];
	Position = mData.LastPosMenu[mData.Level];
	load = 1;
}
//********************************************
//-----------------------------------------------------------------------------------------------------------------


Menu_Item_t* Menu_GetCurrentMenu(void)
{
	return CurrentMenuItem;
}

void Menu_SetGenericWriteCallback(void (*WriteFunc)(void))
{
	MenuWriteFunc = WriteFunc;//��� ��������� �� �. ���������� ����. 
}

extern Menu_Item_t* main_CurrentMenu;

void Vise_Versa_Menu(Menu_Item_t* const NewMenu)
{
	main_CurrentMenu = CurrentMenuItem;
	CurrentMenuItem = NewMenu;
	
}

void Menu_Navigate(Menu_Item_t* const NewMenu)
{
	if ((NewMenu == &NULL_MENU) || (NewMenu == NULL))
		return;

	CurrentMenuItem = NewMenu;//���� ���� ����� ���� �� ����������� ��������� ��� ������

	if (MenuWriteFunc)//���� ����� �������� �� ����
		MenuWriteFunc();//���������� �. ��� ��������� ���� ������� ����� ����.

	//��������� ��������� �� � � ����������� ��� ������
	void (*SelectCallback)(void) = _RM(CurrentMenuItem->SelectCallback);

	if (SelectCallback)//���� ����� �������� �� ����
		SelectCallback();//�� ����, ���� ��������� ��������� void (*SelectCallback)(void);
}

//��������� ���� ������� ��� �������� ����, ���� ������� ���� ������� 1 ����� 0
_uchar_menu Menu_InitCurrentItem(void)
{
	if ((CurrentMenuItem == &NULL_MENU) || (CurrentMenuItem == NULL))
		return 0;

	void (*InitCallback)(void) = _RM(CurrentMenuItem->InitCallback);

	if (InitCallback){
		InitCallback();
		return 1;
	}
	return 0;
}

//���������, ���� �� Enter �. � ��������� ��, ���� �. ��������� ���������� 1
//����� 0
_uchar_menu Menu_EnterCallback(void)
{
	if ((CurrentMenuItem == &NULL_MENU) || (CurrentMenuItem == NULL))
		return 0;

	void (*EnterCallback)(void) = _RM(CurrentMenuItem->EnterCallback);
	if (EndEnteringParams) 
	{
		EndEnteringParams();
		EndEnteringParams = NULL;
	}
	else if (EnterCallback){
		EnterCallback();
		return 1;
	}
	else Enter_edit = NULL;
	return 0;
}

// ������ �������!
// Menu_InitChildItem ����������
// ���� ���� �������, ��  1
// ���� ���� ������� � ������������� ������� �� ������� � ��������������� ������������� 1
// ���� ��� �������, �� 0
_uchar_menu Menu_InitChildItem(void)
{
	// Menu_EnterCallback();
	
	Menu_Item_t* ChildMenuItem = _RM(CurrentMenuItem->Child);

	if ((ChildMenuItem == &NULL_MENU) || (ChildMenuItem == NULL)){
		return 0;
	}
	
	void (*InitCallback)(void) = _RM(ChildMenuItem->InitCallback);

	if (InitCallback){
		InitCallback();
	}
	
	return 1;
}

// ��������� ����-�� �� �. ����� �-��� ������� ������ ����� ����
// � ���� ��������� ������ ������������ ���, �� ������ printItemCallback (����� �. ������� �� �������) ����������
// menu_set_print_item_callback, � ������� ��������� menu_item_print ������ ������� ��� ��������� ������
void menu_set_print_item_callback(void (*f)(uint8_t pos, const char *s))
{
	menu_item_print = f;
}

/**----------------��� ������� menu_make_screen------------------------**/
// ���������� ����� ����� MenuWriteFunc � Menu_Navigate
// ���������� ���������� ���������� � �������
// ��������� mData
// -----------------------------------------------------------------------
// Menu_Item_t* const Menu - ��������� �� ������ ����� ���� � �����, ������������ ���� ��� ���������
// TSettingfontDot *sF �����
void menu_make_screen(Menu_Item_t* const Menu){
	static Menu_Item_t* OldMenu;
	NextMenu=Menu;

	static uint8_t flagA, flagB;
	_uchar_menu CountM=0; // ����� ������� ����
	Position=0;// ���������� ����� ������� ����� ������� � ������� ���������� ��� ����������� ����
	_uchar_menu j, Repaint=0, maxstr, MaxStr, y;
	
	if (mData.Repaint==0)
	if ( mData.MenuState==ST_MENU_EDIT){
		menu_item_print(menu_get_pos(),Menu_GetCurrentMenu()->Text);
		return;
	}

	MaxStr=2;//������� ��������� ����� �� ������
	

		do{	
			debugMenu = Menu_GetCurrentMenu();
			if (debugMenu==NextMenu) {//������� ������� ���� ������������ Menu (������� ������)
				Position=CountM;
			
			}
			
			CountM++;//������������ ���-�� ������� ����
			total_list = CountM - 2;
			NextMenu=_RM(NextMenu->Next);//��������� ���������� �� ��������� ����
		}
		while ( (Menu!=NextMenu) && (NextMenu!=&NULL_MENU) );
	
	
	mData.BrenchCountMenu=CountM;
	
	if (mData.Repaint==MENU_REPAINT_NEW_LEVEL) {//���� ����� ����� ����, �� ������������� � ������ ����������
		mData.EndPosMenu=MaxStr - 1;
		mData.BegPosMenu=0;
		Repaint=1;
	}
	else
	if (mData.Repaint==MENU_REPAINT) {//������ ����������
		Repaint=1;
	}
	debug = Position;
	if (Position>mData.OldPosMenu)//�������� ���� �� ����
	{
		str_number++;
	}
	else{//�������� ����� �� ����
		if (Position < mData.OldPosMenu) 
		{
			str_number--;
		}
	}
	if (str_number > 1) {str_number = 1; list++;}
	if (str_number < 0) {str_number = 0; list--;}
	
	if (Repaint) {
		CurrMenu = Menu;
	}
	if (list > 250) list = 0;
	else if (list > total_list) list = total_list;
	flagA = (list - old_list_numb > 0) && (list - old_list_numb < 250);
	flagB = (list - old_list_numb) < 0;
	
	if ((flagA || flagB))
	{
		CurrMenu = Menu;
		for (int i = 0; i < list; i++)
		{
			CurrMenu =_RM(CurrMenu->Next);
		}

	}
	

	maxstr = MaxStr;
//	if (CountM<=MaxStr){
//		maxstr=CountM; //���� ������� ������ ��� ���������� �� ������
//		if (mData.Repaint==0) Repaint=0;//�� ������� ����� ���� ��� ������ ����������
//	} 
//	else 
//		maxstr=MaxStr+1;
	if (OldMenu == CurrMenu) Repaint = 0;
	if (Repaint) lcd44780_ClearLCD();	//����� ������ ����������� ������� �����
	

		NextMenu = CurrMenu;
		Repaint = 1;


	
		if (str_number > 1) str_number = 1;
	if (str_number < 0) str_number = 0;
	
	for(j=0; j<mData.BegPosMenu; ++j)
		//NextMenu=_RM(NextMenu->Next);//������� ��������� �� ������ ����
	//���� ��������� � ������� NextMenu, ����� ��������� ������������ �� ��������� ����
	//mData.BegPosMenu ������ ��������������� ������ NextMenu (������������� ���� � �����)
	//Pos �������� �� mData.BegPosMenu ������ �� ���������� ����
	mData.posSelMenu=0xFF;
	for(j=mData.BegPosMenu; j<maxstr+mData.BegPosMenu; ++j){//
		mData.posPaintMenu=j;
		if (Position==j){
			mData.ySelMenu=y;//���������� y ����������� ������
			mData.posSelMenu=Position;//������� ������������ Menu_Item_t* const Menu
		}
//********************************************************************************************** 
		// ����� ���� ��� ����, ��� �� ������������������ ����������, ������� ��������� ��������
		// �� ���� �� � ������� ������ ����������� � ����� ����.
		// 3 ������� 1)������ ����������� ������ 2)InitCallback �������� 3)InitCallback �� ����� ����,  
		if ( Repaint ) {
			void (*InitCallback)(void) = _RM(NextMenu->InitCallback);
			if ( InitCallback )
				
					InitCallback();
		}
// ������� �� ���� ���� �������������� � ���������� (�� � �������) �����������, ���� ������ ����� ������ ����������� ������, ������ ���, ��� ��������.
//**********************************************************************************************		
		
		void (*PrintItemCallback)(void) = _RM(NextMenu->printItemCallback);
		// if ((PrintItemCallback != &NULL_MENU) && (PrintItemCallback != NULL))
		if (PrintItemCallback)
			PrintItemCallback();//������ ����� ���� �������� ����� ��������
		if (NextMenu->Text != NULL)
		{
			if (Repaint) {
				menu_item_print(j,NextMenu->Text);//���� ����� �������, �� ������� ���
			}		
			else{
				if (Position==j) menu_item_print(j,NextMenu->Text);//����� ������������ ������ 2 ������
				if ( (mData.OldPosMenu==j) && (Position!=mData.OldPosMenu) ) menu_item_print(j,NextMenu->Text);
			}
		}
		NextMenu=_RM(NextMenu->Next);
		y+=1;
	}
	
	void (*PrintItemCallback)(void) = _RM(Menu_GetCurrentMenu()->printItemCallback);	
		// if ((PrintItemCallback != &NULL_MENU) && (PrintItemCallback != NULL))
		if (PrintItemCallback)
			PrintItemCallback();//������ ����� ����� �() �� ������� �����
	OldMenu = CurrMenu;
	mData.OldPosMenu=Position;//��������� ������� ����������� ����
	mData.Repaint=0;
	old_list_numb = list;
}

void Back_Through_Pass (uint8_t access)
{
		if (access == 1) 
		{
			UserCallBack = menu_back_level;
			SavePassMenu = CurrentMenuItem->Child;
		}
		else 
		{
			UserCallBack = NULL;
			menu_back_level();
		}
}

void NextParam (uint8_t numbers_of_param, void (*func)(void), Menu_Item_t* const Menu, uint8_t edit)
{

		nParam++;
		if (nParam > numbers_of_param)
		{
			func();
			nParam = 1;
			mData.ptrGeneric_Menu = Menu;
			CurrentMenuItem = Menu;
		}

		Enter_edit = menu_back_level;
		if (edit && edit_init) 
		{
			edit_init = 0;
		}
}

//********************************************************
void menu_exit(void){//��������� ����
	mData.Level=0;
	mData.MenuState=ST_MENU_IDLE;
}

void menu_start(void){
	// ������������� ���������� ������������� � ����
	// ��������� �� �������
	dunamic_data_print=NULL;
	menu_item_edit=NULL;
	exit_callback_f=NULL;
		
	menu_set_repaint_newlevel();//��� ������ ���������� ����� ����� ����	
		
	Menu_Navigate(menu_get_ptrGeneric());//������� � ����
	mData.MenuState=ST_MENU_NAVIGATE;
}

void (*show_time_f)(void);
void (*show_version_f)(void);
void (*show_load_usb_f)(void);

void show_pop_up_windiw(void (*func1)(void), void (*func2)(void), void (*func3)(void))
{
	show_time_f = func1;
	show_version_f = func2;
	show_load_usb_f = func3;
}
extern uint8_t SD_USB_Disp_in;
uint8_t time_flag = 0, version_flag = 0, load_usb_flag = 0;
extern uint8_t mode, TS_kruka, TS_strely, flag_button_unlocking, flag_unlocking, beep_off;

uint8_t pop_up_window (uint8_t priority, uint8_t type, uint32_t var, void (*show_func)(void))
{
	static uint8_t init_local_flag = 0;
	static uint8_t local_flag = 0, old_local_flag = 0;
	static void (*identify_func) (void) = NULL;
	static uint8_t identify_priority = 0;
	
	switch (type)
	{
		case ONE_ZERO:
			local_flag = var;
			if (old_local_flag != local_flag) init_local_flag = 1;
			break;
		case DOUBLE_CALL:
			if (var != 0) init_local_flag = !init_local_flag;
			break;
	}
	
	if (pop_flag == 0 && init_local_flag)
	{
		identify_priority = priority;
		identify_func = show_func;
		dunamic_data_print_old = dunamic_data_print;
		dunamic_data_print = show_func;
		pop_flag = 1;
		init_local_flag = 0;
		var = 0;
	}
	else if (pop_flag == 1 && init_local_flag && identify_func == show_func)
	{
		identify_func = NULL;
		dunamic_data_print = dunamic_data_print_old;
		dunamic_data_print_old = NULL;
		pop_flag = 0;
		init_local_flag = 0;
		var = 0;

		Menu_Navigate(CurrentMenuItem);		
	}
	else if ( priority < identify_priority && init_local_flag)
	{
		identify_func = show_func;
		dunamic_data_print = show_func;
		init_local_flag = 0;
		var = 0;
	}		
	else {var = 0; init_local_flag = 0;}
	old_local_flag = local_flag;
	return pop_flag;	
}

_uchar_menu menu_process(_uchar_menu Key){	
//	if (Key == 0x10000000) // 6 ������ - ����� ������� 
//	{
//		if (mode == 140)	{mode = 117;}
//		else							{mode = 140;}
//	}
//	if (Key == 0x100000) // 7 ������ - ������������ �������� ������
//	{
//		if (TS_strely == 1){TS_strely = 0;}
//		else{TS_strely = 1;}
//	}
//	
//	if (Key == 0x10000) // 8 ������ - ������������ �������� �����
//	{
//		if (TS_kruka == 1){TS_kruka = 0;}
//		else{TS_kruka = 1;}
//	}
//	


//	if (Key == 0x20000000){ //���������� �������
//     if (beep_off == 0) beep_off = 1;
//			else beep_off = 0;
//	}
	
	switch (mData.MenuState)
	{
	case ST_MENU_IDLE:
		{
			mData.MenuState=ST_MENU_NAVIGATE;
			PRESS_Button = 0;
		break;
		}
	case ST_MENU_NAVIGATE:
		{//����� �� ����				   priority, type, 			variable, 						function
									pop_up_window(0, ONE_ZERO, 		SD_USB_Disp_in, 				show_load_usb_f);
//									pop_up_window(1, DOUBLE_CALL, 	PRESS_Button & BUTTON_TIME, 	show_time_f);
//			if (mData.Level == 0) 
//				pop_up_window(1, DOUBLE_CALL, 	HOLD_Button & BUTTON_DOWN, 	show_time_f);	
//				pop_up_window(1, DOUBLE_CALL, 	PRESS_Button & BUTTIN_UP_HOLD, 	show_version_f);
			
			if (dunamic_data_print) 
			{
				dunamic_data_print();
			}
			
			if (pop_flag == 0)
			{
				switch (Key){
					

						case BUTTON_ENTER:
							{
									Menu_EnterCallback();//���� ������, �� ����� ���������
									if (CurrentMenuItem->Child == CurrentMenuItem->Parent)
									{
								
										dunamic_data_print=NULL;
										menu_item_edit=NULL;
						
										list = 0;
										str_number = 0;
										old_list_numb = 0;
										mData.OldPosMenu = 0;
										m_number = 0;
										menu_set_repaint_newlevel();//��� ������ ���������� ����� ����� ����
										menu_back_level();
										Menu_Navigate(MENU_PARENT);
										edit_init = 1;
										break;
									}
							
									// Menu_InitChildItem ����������
									// ���� ���� ������� 1  (�� ������� �� �������)
									// ���� ���� ������� � ������������� ������� 1 (�� ������� � ��������������� �������������)
									// ���� ��� �������  0

									switch (Menu_InitChildItem()){//������ � ������������� ������ �������
										case 0:// 0-��� �������	
																	
											if (Menu_InitCurrentItem()==0)//���� ���� ���� �. �������� ����, �� ������� � ��������������
												break;	//����� ����� 
											//���� ���� ����� �� ����, �� ������ ��� � Menu_EnterCallback
											dunamic_data_print=NULL;
											menu_item_edit=NULL;
											m_incSelP();
											if (MenuWriteFunc)
												MenuWriteFunc();//�������� ��������� ������
											
										break;
											
										case 1:// 1-������� �� �������
												dunamic_data_print=NULL;
												menu_item_edit=NULL;
												menu_set_repaint();
												menu_store_level();//����� ���������� mData.ptrGeneric_Menu=MENU_CHILD;
												if (Enter_edit) 
													Enter_edit();								
												Menu_Navigate(MENU_CHILD);//����������� Menu_Navigate(MENU_CHILD);

										break;	
									}
									PRESS_Button = 0;
									break;
						}
						case BUTTON_DOWN:{
							
							Menu_Navigate(MENU_NEXT);
							PRESS_Button = 0;
							break;
						}
						case BUTTON_UP:{
							Menu_Navigate(MENU_PREVIOUS);
							PRESS_Button = 0;
							break;
						}
						case BUTTON_BACK:{
							if (UserCallBack && mData.ptrGeneric_Menu == SavePassMenu) 
								UserCallBack();
							pass_flag = 1;
							dunamic_data_print=NULL;
							menu_item_edit=NULL;
							edit_init = 1;
							if (MENU_PARENT==&NULL_MENU) {
								mData.MenuState=ST_MENU_IDLE;
							}else{
								list = 0;
								str_number = 0;
								old_list_numb = 0;
								mData.OldPosMenu = 0;
								m_number = 0;
								nParam = 1;
								menu_set_repaint_newlevel();//��� ������ ���������� ����� ����� ����
								menu_back_level();
								Menu_Navigate(MENU_PARENT);
							}
							PRESS_Button = 0;
							break;
						}
					}	
				break;
			}
		}
	}
	Menu_Level = mData.Level;
	return mData.MenuState;
}

