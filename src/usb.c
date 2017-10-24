/*
 * usb.c
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: valeriy
 */
#include "stm32f10x.h"
#include "usb.h"
#define EPCOUNT		3
#define BLSIZE		0x8000

void usb_ctr_int();
void usb_pma_int();
void usb_err_int();
void usb_reset_int();
void usb_sof_int();
void usb_esof_int();

volatile uint16_t intStat;

BTable *table = (BTable*) USB_PBUFFER;
PBElement *PBuffer = (PBElement*) USB_PBUFFER;
uint16_t startPB;
volatile RXCount rx1;
static void ep_init();

void usb_init() {
	intStat = 0;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef io;
	io.GPIO_Mode = GPIO_Mode_AF_PP;
	io.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	io.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &io);

	GPIOC->CRH &= ~(0xff << 12); 			// очищаем 4 бита
	GPIOC->CRH |= (0x18 << 12);			// устанавливаем mode 0b11 режим 50МГц

	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
//	ep_init();
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	USB->BTABLE = 0;
	USB->DADDR = 0;
	USB->CNTR = (1 << 10);
	USB->ISTR = 0;
//	USB->DADDR = 0x80;
//	USB_CNTR_CTRM;
}
void setTableTx(uint8_t inx, uint16_t addr, uint16_t count) {
	table[inx].tx.addr.mem = addr;
	table[inx].tx.count.mem = count;
}
uint16_t getTableTxAddr(uint8_t ep) {
	return (table[ep].tx.addr.mem);
}

void setTableRx(uint8_t inx, uint16_t addr, uint16_t count) {
	table[inx].rx.addr.mem = addr;
	table[inx].rx.count.mem = count;
}
uint16_t getTableRxAddr(uint8_t ep) {
	return (table[ep].rx.addr.mem);
}
uint16_t getTableRxCount(uint8_t ep) {
	return ((table[ep].rx.count.mem) & 0x3ff);
}
uint16_t rxcnt(uint16_t bsize, uint16_t nblock) {
	uint16_t tmp = ((bsize & 1) << 15) | ((nblock & 31) << 10);
	return tmp;
}
void ep_init() {
	// init table and pocked
//	PBuffer = &PBuffer[EPCOUNT * 8];
//	startPB = EPCOUNT * 8;
	rx1.f.bsize = 1;
	rx1.f.nblok = 2;
	setTableTx(0, startPB, 16);
//	setTableRx(0, startPB + 16, rxcnt(0, 8));
	setTableRx(0, startPB + 16, RXCNT(0, 8));
	// list 1
	setTableTx(1, startPB + 32, 64);
	setTableRx(2, startPB + 96, rx1.word);
	USB->EPR[0] = (EP_CONTROL << 8);
	USB->EPR[1] = (EP_BULK << 8) | 1;
	USB->EPR[2] = (EP_BULK << 8) | 2;
}

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) {
	uint8_t idx = 0;

	for (idx = 0; idx < len; idx++) {
		if (((value >> 28)) < 0xA) {
			pbuf[2 * idx] = (value >> 28) + '0';
		} else {
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}

void USB_LP_CAN1_RX0_IRQHandler() {
	if (USB->ISTR & (1 << 15)) {
		usb_ctr_int();
		return;
	}
	if (USB->ISTR & (1 << 14)) {
		usb_pma_int();
		USB->ISTR = ~(1 << 14);
		return;
	}
	/*
	 if (USB->ISTR & (1 << 13)) {
	 usb_err_int();
	 USB->ISTR = ~(1 << 13);
	 return;
	 }
	 */
	if (USB->ISTR & (1 << 10)) {
		usb_reset_int();
		USB->ISTR = ~(1 << 10);
		return;
	}
	if (USB->ISTR & (1 << 9)) {
		usb_sof_int();
		USB->ISTR = ~(1 << 9);
		return;
	}
	if (USB->ISTR & (1 << 8)) {
		usb_esof_int();
		USB->ISTR = ~(1 << 8);
		return;
	}

}
void USB_HP_CAN1_TX_IRQHandler(void) {

}
