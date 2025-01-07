#include<stm32_button_user.h>

static void initButtonUser(void){
	GPIO_InitTypeDef GPIO_InitStructure ;

	/*Configure GPIO pin : A0: User Button */
	GPIO_InitStructure.Pin = GPIO_PIN_0;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	GPIO_Init ( GPIOA ,  & GPIO_InitStructure ) ;
}
