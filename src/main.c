//
// This file is part of the GNU ARM Eclipse distribution.
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "stm32f10x.h"
#include "string.h"
#include "myDelay.h"
#include "usb.h"
#include "lcd8.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 em pty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

uint8_t getkey();
void beep();

uint8_t const chr[]={8,26,18,25,37,32,28};
//extern uint8_t usbconn;
//void initialise_monitor_handles(void);
void hw_init();

void lcd_prn() {
	lcd_cmd(LCD0);lcd_str("\10\11u\12epca\13b\11a\14 ");// Нельзя заканчивать строку на графический символ
	lcd_cmd(LCD1);lcd_str("Tec\16 Ma\15u\11a");
	lcd_cmd(LCD2);lcd_str("Xepco\11 2017");


/*
	lcd_cmd(LCD0);lcd_str("Hello world !!!");
	lcd_cmd(LCD1);lcd_str("From Valera !");
	lcd_cmd(LCD2);lcd_str("Kherson 2017");
	lcd_cmd(LCD3);lcd_str("Universal");
*/
}

int main(int argc, char* argv[]) {
//	initialise_monitor_handles();
	// At this stage the system clock should have already been configured
	// at high speed.
	uint8_t comm[16];
	uint8_t tmp;
	uint8_t send = 0;

	myDelay_init();
	hw_init();
	usb_init();
	lcd_init();//myDelay(10);


//	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
/*
	LED_ON(8);
	myDelay(2000);
	LED_OFF(8);
*/
//	printf("Hello Semi\n");
//	trace_puts("Hello ARM World!");
	// Infinite loop
//
//	LED_ON(0);
	lcd_loadfont(chr,7);//myDelay(10);

	lcd_prn();

	while (1) {

		if ((tmp = getkey())) {
			if (tmp == 0x30) lcd_cmd(LCLS);
			if (tmp == 0x31) {
//				lcd_cmd(LCD0);
				lcd_prn();
			}
			beep();
		}
		lcd_cmd(LCD0 + 16);
		//if (testUSB()) lcd_str("USB"); else lcd_str("   ");
		if (getCommCount()) {
			tmp = getCommBuff(comm);
			comm[tmp] = 0;
			lcd_str((const char*)comm);
			sendCommBuff((uint8_t*)comm,tmp);
			beep();
/*
			if (!strcmp((const char*)comm,"q1\n")) send = 1;
			if (!strcmp((const char*)comm,"q0\n")) send = 2;
			if (!strcmp((const char*)comm,"w1\n")) send = 3;
			if (!strcmp((const char*)comm,"w0\n")) send = 4;
*/
		}
/*
		switch (send) {
			case 1:
//				sendCommBuff((uint8_t*)"Hello world!!!\n",15);
				lcd_str(" send q1 ");
				send = 0;
				break;
			case 2:
//				sendCommBuff((uint8_t*)"Hello off     \n",15);
				lcd_str(" send q0 ");
				send = 0;
				break;
			case 3:
//				sendCommBuff((uint8_t*)"Valera on     \n",15);
				lcd_str(" send w1 ");
				send = 0;
				break;
			case 4:
//				sendCommBuff((uint8_t*)"Valera off    \n",15);
				lcd_str(" send w0 ");
				send = 0;
				break;
			default:
				break;
		}
*/

		myDelay(100);

//		if (GPIOC->IDR & (1 << 11)) LED_ON; else LED_OFF;
	}
}
void EXTI15_10_IRQHandler() {
	if (EXTI->PR & 0x800) {
		EXTI->PR |= 0x800;
	}
	if (GPIOC->IDR & 0x800) GPIOA->BSRR = 0x20;
	else GPIOA->BSRR = (0x20 << 16);
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
