/*
 * prser.c
 *
 *  Created on: 12 дек. 2017 г.
 *      Author: valeriy
 */
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "string.h"
#include "myDelay.h"
#include "usb.h"
#include "lcd8.h"

void set_speed(uint16_t spd);
void set_way(uint16_t w);

void nop_menu(uint8_t *str);
void way_menu(uint8_t *str);
void speed_menu(uint8_t *str);

char const *menulist[]={"way=","spd="};

void (*const menu_cb[])(uint8_t *str)={nop_menu,way_menu,speed_menu};

void nop_menu(uint8_t *str) {

}
void way_menu(uint8_t *str) {
	uint16_t q1;

	sscanf((const char*)str,"way=%hu",&q1);
	set_way(q1);
}
void speed_menu(uint8_t *str) {
	uint16_t q1;

	sscanf((const char*)str,"spd=%hu",&q1);
	if ((q1 > 9) && (q1 < 101))set_speed(q1*100);
}

void parser(uint8_t *str) {
	uint8_t sel = 0;
	for (uint8_t i1 = 0; i1 < 2; i1++) {
		if (!strncmp((const char*)str,menulist[i1],4)) sel = i1+1;
	}
	menu_cb[sel](str);
}
