/*
 * usbcore.c
 *
 *  Created on: 26 янв. 2017 г.
 *      Author: valeriy
 */
#define MYUSBLIB
#include "stm32f10x.h"
#include "usb.h"
#define EPCOUNT		3

union {
	USBReqestType req;
	uint16_t buf[4];
} reqBuf;

static uint16_t stat;

uint16_t *pbufcpy(uint16_t *dst, uint8_t ep){
	uint16_t * raddr = dst;
	uint16_t addr = (getTableRxAddr(ep) >> 1);
	uint16_t cnt = (getTableRxCount(ep) >> 1);
	while(cnt--) *(raddr++)=PBuffer[addr++].mem;
	return dst;
}

void usbInit() {
	return;
}
void usb_ctr_int() {
	stat = USB->ISTR;
	pbufcpy(reqBuf.buf, 0);
}
void usb_pma_int() {
	stat = USB->ISTR;
}
void usb_err_int() {
	stat = USB->ISTR;
}
void usb_reset_int() {
	uint16_t pmem = EPCOUNT * 8;
	stat = USB->ISTR;
	setTableTx(0, pmem, 16);
	setTableRx(0, pmem+16, RXCNT(0,8));
	USB->EPR[0] = EP_CONTROL;
	USB->EPR[0] ^= (VALID << STRX);
	USB->ISTR = 0;
	USB->CNTR |= (1 << 15);
	USB->DADDR = 0x80;
}
void usb_sof_int() {
	stat = USB->ISTR;
}
void usb_esof_int() {
	stat = USB->ISTR;
}
