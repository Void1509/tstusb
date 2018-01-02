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
void hw_init();
void parser(uint8_t *str);

static uint8_t usbconn;
static uint8_t way = 0;		//направление лево право
static uint16_t speed = 1000;   //скорость

uint8_t const chr[]={18,25,26,27,28,33};
//extern uint8_t usbconn;
//void initialise_monitor_handles(void);
static void lcd_prn_way();
static void lcd_prn_speed();

void set_speed(uint16_t spd) {
	speed = spd;
	lcd_prn_speed();
}
void set_way(uint16_t w) {
	way = w;
	lcd_prn_way();
}

void lcd_prn_usb() {
	lcd_cmd(LCD0 + 16);
	if (usbconn) lcd_str("USB"); else lcd_str("   ");
}

static void lcd_prn() {
	lcd_prn_usb();
	lcd_cmd(LCD1);lcd_str("nepeme\15e\12ue cynnop\14a");// Нельзя заканчивать строку на графический символ
	lcd_cmd(LCD2);lcd_str("\12anpa\10\11e\12ue ckopoc\14b");
	lcd_prn_way();
	lcd_prn_speed();
//	lcd_cmd(LCD3);lcd_str("Xepco\12 2017");


/*
	lcd_cmd(LCD0);lcd_str("Hello world !!!");
	lcd_cmd(LCD1);lcd_str("From Valera !");
	lcd_cmd(LCD2);lcd_str("Kherson 2017");
	lcd_cmd(LCD3);lcd_str("Universal");
*/
}
static void lcd_prn_way() {
	lcd_cmd(LCD3+2);
	if (way) lcd_str("npa\10o"); else lcd_str("\11e\10o ");
}
static void lcd_prn_speed() {
	char str[8];
	lcd_cmd(LCD3+12);
	sprintf(str,"%8d",speed);
	lcd_str(str);
}


int main(int argc, char* argv[]) {
//	initialise_monitor_handles();
	// At this stage the system clock should have already been configured
	// at high speed.
	uint8_t comm[16];
	char str[8];
	uint8_t tmp;
	usbconn = 0;

	myDelay_init();
	hw_init();
	usb_init();
	lcd_init();

	lcd_loadfont(chr,6);

	lcd_prn();
	lcd_cmd(LCD3+12);
	sprintf(str,"%8d",speed);
	lcd_str(str);
	while (1) {

		if ((tmp = getkey())) {
			switch (tmp) {
				case 0x30:
					lcd_cmd(LCLS);
					lcd_prn();
					break;
				case 0x45:
					lcd_cmd(LCLS);
					break;
				case 0x40:
					way = 0;
					lcd_prn_way();
					break;
				case 0x41:
					way = 1;
					lcd_prn_way();
					break;
				case 0x42:
					if (speed > 1000) speed -= 100;
					lcd_prn_speed();
					break;
				case 0x43:
					if (speed < 10000) speed += 100;
					lcd_prn_speed();
					break;
			}
		}
		if (testUSB()!=usbconn) {
			lcd_cmd(LCD0 + 16);
			usbconn = testUSB();
			lcd_prn_usb();
		}
		if (getCommCount()) {
			tmp = getCommBuff(comm);
			comm[tmp] = 0;
			lcd_cmd(LCD0);
			lcd_str((const char*)comm);
			parser(comm);
			//sendCommBuff((uint8_t*)comm,tmp);
			beep();
		}
		myDelay(100);
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
