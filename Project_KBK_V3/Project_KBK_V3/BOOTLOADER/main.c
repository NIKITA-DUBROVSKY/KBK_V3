#include "stm32f0xx.h"
#include "hd44780.h"
#include "SPI_MSD_Driver.h"
#include "ff.h"
         
FIL fsrc;         
FRESULT result;
UINT br;
FATFS fs;
FATFS* fs_ptr = &fs;
DWORD fre_clust, fre_sect, tot_sect, avalible_SD, total_SD;
DIR dir;
FILINFO fileInfo;

void Init_RCC(void)	{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
		RCC_WaitForHSEStartUp();
		RCC_PREDIV1Config(RCC_PREDIV1_Div1);
		RCC_PLLConfig(RCC_PLLSource_HSE, RCC_PLLMul_6);//40���
		RCC_PLLCmd(ENABLE);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLKConfig(RCC_HCLK_Div1);

		FLASH_SetLatency(FLASH_Latency_1);
		FLASH_PrefetchBufferCmd(ENABLE);	
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOA,  ENABLE );
}

typedef void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t  jumpAddress;
	
	
// Bootloader key configuration
#define BOOTLOADER_KEY_START_ADDRESS            (uint32_t)0x0801F800
#define BOOTLOADER_KEY_PAGE_NUMBER              63
#define BOOTLOADER_KEY_VALUE                    0xAAAA5555
 
// Flash configuration
#define MAIN_PROGRAM_START_ADDRESS              (uint32_t)0x08004000
#define MAIN_PROGRAM_PAGE_NUMBER                8  //�������� � ������� ���������� �������� ���������
#define NUM_OF_PAGES                            62 //����� ���������� ������� �����
#define FLASH_PAGE_SIZE                         2048

void SetKey()
{
  FLASH_Unlock();
  FLASH_ProgramWord(BOOTLOADER_KEY_START_ADDRESS, BOOTLOADER_KEY_VALUE);
  FLASH_Lock();
} // End of SetKey()
 
void ResetKey()
{
  FLASH_Unlock();
  FLASH_ErasePage(BOOTLOADER_KEY_START_ADDRESS);
  FLASH_Lock();
} // End of ResetKey()

uint32_t ReadKey()
{
  return (*(__IO uint32_t*) BOOTLOADER_KEY_START_ADDRESS);
} // End of ReadKey()

uint32_t programBytesToRead;
uint32_t programBytesCounter;
uint32_t currentAddress;
UINT readBytes;
uint8_t readBuffer[512];

uint16_t led_status[31];

void Init_leds(void) {
	//������������ ���� ��� �����������
	GPIO_InitTypeDef GPIO_InitStructure; //��������� ���������� ��������� �����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_15; //�������� ������ ������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//����� �������� ������� �����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //�������� ����� ������
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); //����� ������� �������������
}

/*������� ��������� ����������� (���������� ������ ��������� �����
� ����������� �� �������� � �������, ������ ������� ������� �������������
���������� �� �����) */
void Led_show(void)    {
	uint8_t l = 0;
    while(l <= 30) //���������� ����������� �� 0 �� 30
     {
            GPIO_ResetBits(GPIOB,GPIO_Pin_12| GPIO_Pin_13|GPIO_Pin_15);
            
            if (led_status[30-l] == 0) GPIO_SetBits(GPIOB,GPIO_Pin_13);//��������
            else if (led_status[30-l] == 1) 
            {
                GPIO_SetBits(GPIOB,GPIO_Pin_15);//��������� 1
                GPIO_SetBits(GPIOB,GPIO_Pin_13);//��������
            }
            
            l++;
     }
        GPIO_SetBits(GPIOB,GPIO_Pin_12);//��������
}

void GPIO_USB_Init (void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

void delay_ms(uint32_t ms)
{
	for (uint32_t i = 0; i <= ms; i++){
		delay_micros(1000);
	}
}
	
int main(void)
 {
	Init_RCC();
	 
	GPIO_USB_Init(); 
	RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
	TIM16->PSC = 48-1;
	TIM16->CR1 |= TIM_CR1_CEN;

	for (uint32_t i = 0; i < 10000; i++){} //��� ����� �� �������� ������� ��� ��������		
	
	Init_leds();
	uint8_t l = 0;
	while (l <= 30){
	led_status[l] = 0;
	l++;
	}
	Led_show();
	 
	lcd44780_init_pins();   //  ���������������� ������� �������
	lcd44780_init();        //  ������������� �������   

	if (ReadKey() == BOOTLOADER_KEY_VALUE) //��������� ���� � ���� �� ������ ������ �������� ������ �������, ��������� � �������� ��������� 
  {
    ResetKey(); //���������� ����
    __disable_irq();    
    jumpAddress = *(__IO uint32_t*) (MAIN_PROGRAM_START_ADDRESS + 4); //����� �� �������� ����� ������� ����� �� �������� 4 ������������ ������ �������� ��������, ����� ��� ������ (��� RESET � ����� .map)
    Jump_To_Application = (pFunction) jumpAddress;                    //������ ����������� ���� ����� ������� ��� ��������
    __set_MSP(*(__IO uint32_t*) MAIN_PROGRAM_START_ADDRESS);          // �������������� SP
    Jump_To_Application();                                           //��� ��� � ���� ������� �� �������� ���������� �� ������, ������� �� ����� �� RESET ��� ������ ����������.
  }
  else
  { //���� ���� �� ������, ���� �������� �� ������	 
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("     ������     ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("  ������������  ");
		delay_ms(1000);
		
    MSD_SPI_Configuration();
		    
    FATFS   SDfs;
    FIL program;
    
    if(f_mount(0,&fs) == FR_OK)
    {
      uint8_t path[12] = "program.bin";
      path[11] = '\0';
          
      result = f_open(&program, (char*)path, FA_READ);
          
      if (result == FR_OK)
      { //����� ���� � ���������
        
				lcd44780_SetLCDPosition(0, 0);
				lcd44780_ShowStr("     �������    ");
				lcd44780_SetLCDPosition(0, 1);
				lcd44780_ShowStr("    ��������    ");
        // Program        
        FLASH_Unlock();
        //������� ��� ���� ��� ���� ���������
        for(uint8_t i = 0; i < (NUM_OF_PAGES - MAIN_PROGRAM_PAGE_NUMBER); i++)
        {
           FLASH_ErasePage(MAIN_PROGRAM_START_ADDRESS + i * FLASH_PAGE_SIZE);
        }
        
        programBytesToRead = program.fsize; //������ ��������, ������� �� ����� ������ � ������
        programBytesCounter = 0;
        currentAddress = MAIN_PROGRAM_START_ADDRESS;
        
        while ((programBytesToRead -  programBytesCounter) >= 512) //���� �������� �������� ������ 512 ����, �� ��������� �� �� ����� � ����������
        {//��������� �� ����� 512 ����, ���������� �� �� Flash. ����� ��������� ��������� 512 ���� � ��� �����
          f_read(&program, readBuffer, 512, &readBytes); 
          programBytesCounter += 512;
          
          for (uint32_t i = 0; i < 512; i += 4)
          {         
            FLASH_ProgramWord(currentAddress, *(uint32_t*)&readBuffer[i]);
              
            currentAddress += 4;
          }        
        }
          
        if (programBytesToRead != programBytesCounter) //���� �������� �������� ������ 512 ����, ��������� ���������� ���������� � ���������� �� Flash ��� ��������� �����
        {
          f_read(&program, readBuffer, (programBytesToRead - programBytesCounter), &readBytes);
            
          for (uint32_t i = 0; i < (programBytesToRead - programBytesCounter); i += 4)
          {     
            FLASH_ProgramWord(currentAddress, *(uint32_t*)&readBuffer[i]);
                
            currentAddress += 4;
          }
          programBytesCounter = programBytesToRead;
        }      
        FLASH_Lock(); //��������� ����
        
        f_close(&program); //��������� ����
        
        f_unlink((char*)path); //������� ���� ��������
        
        ResetKey();
        SetKey(); //��������� ����

				lcd44780_SetLCDPosition(0, 0);
				lcd44780_ShowStr("    ��������    ");
				lcd44780_SetLCDPosition(0, 1);
				lcd44780_ShowStr("   ���������    ");
				delay_ms(1000);
				
        NVIC_SystemReset(); //������������� ����������
      }
      else
      {//���� ���� � ��������� �� ������
        // Jump
        if (((*(uint32_t*)MAIN_PROGRAM_START_ADDRESS) & 0x2FFF0000 ) == 0x20000000) //����������� �����, ���������� �� ������ �������� ���������  
        {                                                                           //� ���� ��� ������������� ��� � ���, ��� �������� ��������� ��� ��������� �� Flash				
					ResetKey();
          SetKey();           //��������� ����
          NVIC_SystemReset(); //������������� ����������
        }
        else //e��� �������� ��������� ����������� � ������, ������ ������������� ���������� 
        {
					lcd44780_SetLCDPosition(0, 0);		
					lcd44780_ShowStr("    ��������    ");
					lcd44780_SetLCDPosition(0, 1);
					lcd44780_ShowStr("  �� ���������  ");
					while(1){} //��������
          NVIC_SystemReset();
        }
      }
    }
  }
	while(1)
{
}
}
