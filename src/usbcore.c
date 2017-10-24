/*
 * usbcore.c
 *
 *  Created on: 26 янв. 2017 г.
 *      Author: valeriy
 */
#include "stm32f10x.h"
#include "usb.h"

union {
	USBReqestType req;
	uint16_t buf[4];
} reqBuf;

static uint16_t stat;

void usbInit() {
	return;
}
void usb_ctr_int() {
	stat = USB->ISTR;
}
void usb_pma_int() {
	stat = USB->ISTR;
}
void usb_err_int() {
	stat = USB->ISTR;
}
void usb_reset_int() {
	stat = USB->ISTR;
}
void usb_sof_int() {
	stat = USB->ISTR;
}
void usb_esof_int() {
	stat = USB->ISTR;
}
