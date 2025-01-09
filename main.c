//--------------------------------------------------------------
// File     : main.c
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
//--------------------------------------------------------------

#include "main.h"
#include "pacman.h"


USB_HID_HOST_STATUS_t usb_status;  // trang thai Keyboard


int main(void)
{
  SystemInit(); //

  pacman_start();
//  pacman_hw_init();
//  skin_init();
  while(1)
  {
//	  Image2LCD_t test;
//	    test.dest_xp = 10;
//	    test.dest_yp = 0;
//	    test.source_xp = 0;
//	    test.source_yp = 0;
//	    test.w = 130;
//	    test.h = 130;
//	    UB_Graphic2D_DrawImageRect(test);
  }
}


