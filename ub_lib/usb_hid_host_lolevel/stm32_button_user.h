#include "stm32f4xx_gpio.h"

#define PRESSED_BUTTON_NONE 0x00
#define PRESSED_BUTTON_USER 0x01

tU8 Read_User_Button(void);


static void initButtonUser(void);
