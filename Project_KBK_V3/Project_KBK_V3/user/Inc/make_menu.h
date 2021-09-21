#include "MicroMenu.h"

#ifndef _MAKE_MENU_H_
#define _MAKE_MENU_H_

//MENU_ITEM(Name,	Next,								Previous,							Parent,		Child,			SelectF,		EnterF, InitF, 					PrintText, 			TEXT)
MENU_ITEM(Menu_1,	Menu_Limitations,		Menu_TIME_PRINT,			Menu_2,		Menu_1_1_A,		main_page_0,	NULL,	m_init_dunamic, m_print_simple, "");
MENU_ITEM(Menu_2,	Menu_Limitations,		Menu_TIME_PRINT,			Menu_3,		Menu_1_1_A,		main_page_1,	NULL,	m_init_dunamic,	m_print_simple, "");
MENU_ITEM(Menu_3,	Menu_Limitations,		Menu_TIME_PRINT,			Menu_1,		Menu_1_1_A,		main_page_2,	NULL,	m_init_dunamic,	m_print_simple, "");

	MENU_ITEM(Menu_TIME_PRINT,	NULL_MENU,		NULL_MENU,			Menu_1,		NULL_MENU,		TIME_PRINT,	NULL,	NULL,	m_print_simple, NULL);
	MENU_ITEM(Menu_Limitations,	NULL_MENU,		NULL_MENU,			Menu_1,		NULL_MENU,		limitations,	NULL,	NULL,	m_print_simple, NULL);

//MENU_ITEM(Name,					Next,				Previous,		Parent,		Child,			SelectF,							EnterF, 					InitF, 						PrintText, 			TEXT)
/**/MENU_ITEM(Menu_1_1_A,	Menu_1_1_B,	NULL_MENU,	Menu_1,		NULL_MENU,	dunamic_f_TS_KOLONNA,	enter_f_TS,				m_init_f_sample, 	m_print_skroll, "Трансп. скор. ");
/**/MENU_ITEM(Menu_1_1_B,	Menu_1_1,		Menu_1_1_A,	Menu_1,		NULL_MENU,	dunamic_f_TS_KOLONNA,	enter_f_KOLONNA,	m_init_f_sample, 	m_print_skroll, "Режим колонна ");

/**/MENU_ITEM(Menu_1_1,		Menu_1_2,		Menu_1_1_B,		Menu_1,		Menu_1_1_1,		dunamic_f_TS_KOLONNA,	enter_f_LEP,	NULL, 	m_print_skroll, "Диапазон ЛЭП    ");
		MENU_ITEM(Menu_1_1_1,	NULL_MENU,	NULL_MENU,		Menu_1_1,		Menu_1_1,	Setup_LEP_sample,	save_range_LEP,	m_init_dunamic, 	m_print_simple, "Выбор диапазона         ");

/**/MENU_ITEM(Menu_1_2,		Menu_1_3,		Menu_1_1,			Menu_1,		Menu_1_2_1,		NULL,	NULL,	NULL,		m_print_skroll, "Датчики         ");
	/***/MENU_ITEM(Menu_1_2_1,	Menu_1_2_2,	NULL_MENU,		Menu_1_2,		Menu_1_2_1_1,	dunamic_f_sample,	NULL,	m_init_f_sample, 	m_print_skroll, "Датчик ЛЭП   :");
		/****/MENU_ITEM(Menu_1_2_1_1,	NULL_MENU,	NULL_MENU,		Menu_1_2_1,		NULL_MENU,	Read_sensor_LEP,	NULL,	m_init_dunamic, 	m_print_simple, NULL);
		
	/***/MENU_ITEM(Menu_1_2_2,	Menu_1_2_3,	Menu_1_2_1,		Menu_1_2,		Menu_1_2_2_1,	dunamic_f_sample,	NULL,	m_init_f_sample, 	m_print_skroll, "Датчик Усилия:");
		/****/MENU_ITEM(Menu_1_2_2_1,	NULL_MENU,	NULL_MENU,		Menu_1_2_2,		NULL_MENU,	Read_sensor_USILIE,	NULL,	m_init_dunamic, 	m_print_simple, NULL);

	/***/MENU_ITEM(Menu_1_2_3,	Menu_1_2_4,	Menu_1_2_2,		Menu_1_2,		Menu_1_2_3_1,	dunamic_f_sample,	NULL,	m_init_f_sample, 	m_print_skroll, "ДУ на стреле :");
		/****/MENU_ITEM(Menu_1_2_3_1,	NULL_MENU,	NULL_MENU,		Menu_1_2_3,		NULL_MENU,	Read_sensor_DUG1,	NULL,	m_init_dunamic, 	m_print_simple, NULL);
			
	/***/MENU_ITEM(Menu_1_2_4,	NULL_MENU,	Menu_1_2_3,		Menu_1_2,		Menu_1_2_4_1,	dunamic_f_sample,	NULL,	m_init_f_sample, 	m_print_skroll, "ДУ на раме   :");
		/****/MENU_ITEM(Menu_1_2_4_1,	NULL_MENU,	NULL_MENU,		Menu_1_2_4,		NULL_MENU,	Read_sensor_DUG2,	NULL,	m_init_dunamic, 	m_print_simple, NULL);
			
/**/MENU_ITEM(Menu_1_3,		Menu_1_4,		Menu_1_2,			Menu_1,		SD_card,		NULL,	SD_card_init_f,	NULL,		m_print_skroll, "Карта памяти    ");
MENU_ITEM(SD_card,	NULL_MENU,		NULL_MENU,			Menu_1_3,		NULL_MENU,		gorizontal_scroll,	SD_card_enter_f,	m_init_dunamic, m_print_simple, "");

/**/MENU_ITEM(Menu_1_4,		NULL_MENU,	Menu_1_3,			Menu_1,		Menu_pass_entry1,		Settings_init_f,	NULL,	NULL,		m_print_skroll, "Настройки       ");
  /***/MENU_ITEM(Menu_pass_entry1,	Menu_pass_entry2,		NULL_MENU,		Menu_1_4,			Menu_Settings_Calib_Q,		entry_password,	Check_password,	m_init_dunamic,	m_print_simple, " Введите пароль  ");
	/***/MENU_ITEM(Menu_pass_entry2,	NULL_MENU,		Menu_pass_entry1,		Menu_1_4,			Menu_Holy_egg1,						entry_password,	Check_password,	m_init_dunamic,	m_print_simple, "");
		
		/****/MENU_ITEM(Menu_Settings_Calib_Q,	Menu_Settings_Calib_R,		NULL_MENU,								Menu_pass_entry1,			Menu_Calib_Q_Entry,		read_flash,	NULL,	NULL,	m_print_skroll, "Калибровка Q    ");
			/*****/MENU_ITEM(Menu_Calib_Q_Entry,	NULL_MENU,	NULL_MENU,		Menu_Settings_Calib_Q,		Menu_Settings_Calib_Q,		calibration_Q,	save_Calib,	m_init_dunamic, 	m_print_simple, NULL);

		
		/****/MENU_ITEM(Menu_Settings_Calib_R,	Menu_Settings_Calib_Kren,	Menu_Settings_Calib_Q,		Menu_pass_entry1,			Menu_Calib_R_Entry,		read_flash,	NULL,	NULL,	m_print_skroll, "Калибровка R    ");
			 /*****/MENU_ITEM(Menu_Calib_R_Entry,	NULL_MENU,	NULL_MENU,		Menu_Settings_Calib_R,		Menu_Settings_Calib_R,	calibration_R,	save_params,	m_init_dunamic, 	m_print_simple, NULL);

		/****/MENU_ITEM(Menu_Settings_Calib_Kren,	Menu_Settings_Time,	Menu_Settings_Calib_R,		Menu_pass_entry1,			Menu_Calib_Kren_Entry,		read_flash,	NULL,	NULL,	m_print_skroll, "Ввод крена      ");
			 /*****/MENU_ITEM(Menu_Calib_Kren_Entry,	NULL_MENU,	NULL_MENU,		Menu_Settings_Calib_Kren,		Menu_Settings_Calib_Kren,	vvod_krena,	save_params,	m_init_dunamic, 	m_print_simple, NULL);
		
		/****/MENU_ITEM(Menu_Settings_Time,	Menu_Settings_Service,		Menu_Settings_Calib_Kren,		Menu_pass_entry1,			Menu_Time_Entry,		read_flash,	NULL,	NULL,	m_print_skroll, "Дата и время    ");
			/*****/MENU_ITEM(Menu_Time_Entry,	NULL_MENU,	NULL_MENU,		Menu_Settings_Time,		Menu_Settings_Time,	date,	Edit_Time,	m_init_dunamic, 	m_print_simple, NULL);
		
		/****/MENU_ITEM(Menu_Settings_Service,	Menu_Settings_Crane,		Menu_Settings_Time,		Menu_pass_entry1,			Menu_Service_Entry,		read_flash,	NULL,	NULL,	m_print_skroll, "Сервис          ");
			/*****/MENU_ITEM(Menu_Service_Entry,	NULL_MENU,	NULL_MENU,		Menu_Settings_Service,		Menu_Settings_Service,	service,	Edit_Number_KBK,	m_init_dunamic, 	m_print_simple, NULL);		
		
		/****/MENU_ITEM(Menu_Settings_Crane,	NULL_MENU,		Menu_Settings_Service,		Menu_pass_entry1,			Menu_Crane_Entry,		read_flash,	NULL,	NULL,	m_print_skroll, "Выбор крана     ");
			/*****/MENU_ITEM(Menu_Crane_Entry,	NULL_MENU,	NULL_MENU,		Menu_Settings_Crane,		Menu_Settings_Crane,	Crane_selection,	save_params,	m_init_dunamic, 	m_print_simple, "Выбор крана     ");
			
			 
		/****/MENU_ITEM(Menu_Holy_egg1,	Menu_Holy_egg2,		NULL_MENU,		Menu_pass_entry1, 	NULL_MENU,	NULL,	NULL,	NULL,	m_print_skroll, "NULL_MENU");
		/****/MENU_ITEM(Menu_Holy_egg2,	NULL_MENU,	Menu_Holy_egg1,		Menu_pass_entry1,		NULL_MENU,		NULL,	NULL,	NULL,	m_print_skroll, "NULL_MENU");


#endif        
