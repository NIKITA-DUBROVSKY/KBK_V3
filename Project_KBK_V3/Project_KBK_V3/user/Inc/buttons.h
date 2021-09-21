#ifndef __BUTTONS
#define __BUTTONS

#include "stdint.h"

#define SB1_GPIO_PORT GPIOB
#define SB1_GPIO_PIN GPIO_Pin_5

#define SB2_GPIO_PORT GPIOB
#define SB2_GPIO_PIN GPIO_Pin_4

#define SB3_GPIO_PORT GPIOB
#define SB3_GPIO_PIN GPIO_Pin_2

#define SB4_GPIO_PORT GPIOB
#define SB4_GPIO_PIN GPIO_Pin_0

//#define SB1_GPIO_PORT GPIOB
//#define SB1_GPIO_PIN GPIO_Pin_5

//#define SB2_GPIO_PORT GPIOB
//#define SB2_GPIO_PIN GPIO_Pin_4

//#define SB3_GPIO_PORT GPIOB
//#define SB3_GPIO_PIN GPIO_Pin_2

//#define SB4_GPIO_PORT GPIOB
//#define SB4_GPIO_PIN GPIO_Pin_0

void Init_button(void);
uint8_t NumbPressButton(void);
void ProcessKeyFSM(void);
uint8_t GetKeyCode(void);
void program_key_click (void);

#endif //__BUTTONS
