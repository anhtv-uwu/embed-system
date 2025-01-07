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

  while(1)
  {
  }
}


