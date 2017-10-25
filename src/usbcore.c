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

#pragma pack(push,1)
union {
	USBReqestType req;
	uint16_t buf[4];
} reqBuf;
#pragma pack(pop)
static uint16_t stat;
void EP0Interrupt();

uint16_t *pbufcpy(uint16_t *dst, uint8_t ep) {
	uint16_t * raddr = dst;
	uint16_t addr = (getTableRxAddr(ep) >> 1);
	uint16_t cnt = (getTableRxCount(ep) >> 1);
	while (cnt--)
		*(raddr++) = PBuffer[addr++].mem;
	return dst;
}

void usbInit() {
	return;
}
void usb_ctr_int() {
	stat = USB->ISTR;
	pbufcpy(reqBuf.buf, 0);
	switch (stat & 0xf) {
	case 0:
		EP0Interrupt();
		break;
	}
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
	setTableRx(0, pmem + 16, RXCNT(0, 8));
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
void EP0Interrupt() {
	uint16_t tmp;
	if (USB->ISTR & DIR) {
		if (USB->EPR[0] & SETUP) {
			pbufcpy(reqBuf.buf, 0);
			switch (reqBuf.req.bReq) {
			case 0:
				pbufcpy(reqBuf.buf, 0);
				break;
			case 1:
				pbufcpy(reqBuf.buf, 0);
				break;
			case 3:
				pbufcpy(reqBuf.buf, 0);
				break;
			case 5:
				//USB->ISTR = 0;
				tmp = USB->EPR[0];
				tmp ^= (VALID <<STTX); // | (VALID << STRX); //| DTOG_TX;
				tmp |= CTR_TX;// | EP_KIND;
				if (tmp & CTR_RX) tmp &= ~(CTR_RX);
				setTxCount(0,0);
				//USB->DADDR = 0x80 | reqBuf.req.wValue;
				USB->EPR[0] = tmp;
/*
				USB->EPR[0] = EP_CONTROL;
				//USB->DADDR = 0x80 | reqBuf.req.wValue;
				setTxCount(0, 0);
				USB->EPR[0] ^= (VALID << STTX) | DTOG_TX | (VALID << STRX);
*/
				break;
			case 6:
				pbufcpy(reqBuf.buf, 0);
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;
			}
		}
	} else {
//	if (USB->EPR[0] & CTR_TX){
		USB->DADDR = 0x80 | reqBuf.req.wValue;
		tmp = USB->EPR[0];
		//USB->ISTR = 0;
		tmp ^= (STALL << STRX)|(STALL << STTX);
		if (tmp & CTR_TX) tmp &= ~(CTR_TX);
		USB->EPR[0] = tmp;
		pbufcpy(reqBuf.buf, 0);
	}
}
