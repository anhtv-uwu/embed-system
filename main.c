#include "main.h"
#include "pacman.h"
#include "random.h"


USB_HID_HOST_STATUS_t usb_status; 


int main(void)
{
  SystemInit(); //

  pacman_start();
//   pacman_hw_init();


  while(1)
  {
	// int button = UART_CheckButton();
	// if (button == -1){
	//   continue;
	// }
	// char buf[10];
	// sprintf(buf, "Button: %d", button);
	// UB_Font_DrawString(10, 10, buf, & Arial_7x10, 0x0000, 0xFFFF);
  }
}


