#include "stm32f0xx.h"
#include "ADC.h"

extern uint16_t ADCBuffer[];

void ADC_DMA_init(void){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB,  ENABLE );
	
		GPIOB->MODER |= GPIO_MODER_MODER1; // Конфигурируем PB1 как аналоговый вход
	
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
 
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCBuffer;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //внешний триггер и выбор фронта
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO; //Выбор внешнего триггера
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //Выравнивание данных
		ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward; //направление сканирования (Upward scan (from CHSEL0 to CHSEL18))
    ADC_Init(ADC1, &ADC_InitStructure);

		ADC_ChannelConfig(ADC1, ADC_Channel_9, ADC_SampleTime_239_5Cycles);	//канал для ДАТЧИКА Т
		
//		ADC_ChannelConfig(ADC1, ADC_Channel_Vrefint, ADC_SampleTime_239_5Cycles);	//канал для опорного напряжения
//    ADC_VrefintCmd(ENABLE);
    
		ADC_GetCalibrationFactor(ADC1);
		
		ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
		
		ADC_DMACmd(ADC1 , ENABLE ); //разрешаем отправку по ДМА
		 
		ADC_Cmd(ADC1 , ENABLE ) ; //Включаем АЦП
		
		/* Wait the ADRDY flag */
		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY))
			; 
		
    ADC_StartOfConversion(ADC1); //Запускаем преобразование
		
		DMA_Cmd(DMA1_Channel1 , ENABLE ) ; //разрешаем работу указанного канала
}
