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

//extern uint8_t usbconn;
//void initialise_monitor_handles(void);
void hw_init();

int main(int argc, char* argv[]) {
//	initialise_monitor_handles();
	// At this stage the system clock should have already been configured
	// at high speed.
	uint8_t comm[16];
	uint8_t tmp;
	uint8_t send = 0;


	myDelay_init();
	hw_init();
//	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
/*
	LED_ON(8);
	myDelay(2000);
	LED_OFF(8);
*/
//	printf("Hello Semi\n");
//	trace_puts("Hello ARM World!");
	// Infinite loop
	myDelay(2000);
	usb_init();
	lcd_init();
//	LED_ON(0);
	lcd_str("Hello World !!!");
	lcd_cmd(0xC0);lcd_str("From Valeriy !!!");
	lcd_cmd(0x94);lcd_str("Kherson 2017");
	lcd_cmd(0xD4);lcd_str("Universal");

	while (1) {

		if ((tmp = getkey())) {
			if (tmp == 0x30) lcd_cmd(0x1);
		}
/*
		if (usbconn) {
			usb_init();
			LED_ON(0);
		} else {
			usb_deinit();
			LED_OFF(0);
		}
*/

		if (getCommCount()) {
			tmp = getCommBuff(comm);
			comm[tmp] = 0;
			lcd_str((const char*)comm);
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
