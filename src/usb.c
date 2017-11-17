/*
 * usb.c
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: valeriy
 */
#include "stm32f10x.h"
#include "usb.h"
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

void usb_init() {
	intStat = 0;
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	USB->BTABLE = 0;
	USB->DADDR = 0;
	USB->CNTR = (1 << 10);
	USB->ISTR = 0;
	GPIOC->CRL |= GPIO_CRL_MODE7_1;
}

void usb_deinit(){
	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	USB->CNTR = 3;
}

void setkind() {
	uint16_t tmp = USB->EPR[0] & 0x868f;
	tmp |= 0x100;
	USB->EPR[0] = tmp;
}
void clrkind() {
	uint16_t tmp = USB->EPR[0] & 0x868f;
	USB->EPR[0] = tmp;
}
void setTableTx(uint8_t inx, uint16_t addr, uint16_t count) {
	table[inx].tx.addr.mem = addr;
	table[inx].tx.count.mem = count;
}
inline void setTxCount(uint8_t ep, uint16_t cnt) {
	table[ep].tx.count.mem = cnt;
}
inline uint16_t getTableTxAddr(uint8_t ep) {
	return (table[ep].tx.addr.mem);
}

void setTableRx(uint8_t inx, uint16_t addr, uint16_t count) {
	table[inx].rx.addr.mem = addr;
	table[inx].rx.count.mem = count;
}
inline void setRxCount(uint8_t ep, uint16_t cnt) {
	table[ep].rx.count.mem = cnt;
}
inline uint16_t getTableRxAddr(uint8_t ep) {
	return (table[ep].rx.addr.mem);
}
inline uint16_t getTableRxCount(uint8_t ep) {
	return ((table[ep].rx.count.mem) & 0x3ff);
}

void setEPType(uint8_t ep, uint16_t type) {
	register uint16_t tmp = USB->EPR[ep] & 0x10f;
	tmp |= type | CTR_RX | CTR_TX;
	USB->EPR[ep] = tmp;
}
void toggleRx(uint8_t ep) {
	register uint16_t tmp = USB->EPR[ep] & 0x70f;
	tmp |= CTR_RX | CTR_TX | 0x4000;
	USB->EPR[ep] = tmp;
}
void toggleTx(uint8_t ep) {
	register uint16_t tmp = USB->EPR[ep] & 0x70f;
	tmp |= CTR_RX | CTR_TX | 0x40;
	USB->EPR[ep] = tmp;
}
void setStatTx(uint8_t ep, uint16_t stat) {
	register uint16_t tmp = USB->EPR[ep] & 0x73f;
	tmp ^= (stat << STTX);
	USB->EPR[ep] = tmp | CTR_RX | CTR_TX;
}
void setStatRx(uint8_t ep, uint16_t stat) {
	register uint16_t tmp = USB->EPR[ep] & 0x370f;
	tmp ^= (stat << STRX);
	USB->EPR[ep] = tmp | CTR_RX | CTR_TX;
}
inline void clrCTR_rx(uint8_t ep) {
	USB->EPR[ep] &= 0x78f;
}
inline void clrCTR_tx(uint8_t ep) {
	USB->EPR[ep] &= 0x870f;
}
void usr2pma(uint8_t *src, uint16_t addr, uint16_t cnt) {
	while (cnt--) {
		if (addr & 1) {
			PBuffer[addr >> 1].mem |= (*(src++) << 8);
			addr++;
		} else {
			PBuffer[addr >> 1].mem = *(src++);
			addr++;
		}
	}
}
void pma2usr(uint8_t *dst, uint16_t addr, uint16_t cnt) {
	while (cnt--) {
		*(dst++) =
				(addr & 1) ?
						((PBuffer[addr >> 1].mem) >> 8) :
						(PBuffer[addr >> 1].mem);
		addr++;
	}
}

void tableInit() {
	uint16_t tstart = EPCOUNT << 3;
	setTableTx(0, tstart, 16);
	setTableRx(0, tstart + 16, RXCNT(0, 4));

	setTableTx(1, tstart + 24, 64);			// IN 1 (81)

	setTableTx(2, tstart + 88, 8);			// IN 2 (82)

	setTableRx(3, tstart + 96, RXCNT(0, 4));		// OUT 3(3)
}

void ep_init() {

	tableInit();
	for (uint8_t i = 0; i < 8; i++) {
		uint16_t tmp = USB->EPR[i];
		tmp &= 0x7070;
		tmp |= i;
		USB->EPR[i] = tmp;
	}
	USB->ISTR = 0;
	USB->CNTR |= 0x8600;
	setEPType(0, EP_CONTROL);
	setStatRx(0, VALID);
	USB->DADDR = 0x80;
}

void USB_LP_CAN1_RX0_IRQHandler() {
	if (USB->ISTR & (1 << 15)) {
		usb_ctr_int();
		return;
	}
	if (USB->ISTR & (1 << 14)) {
		USB->ISTR = ~(1 << 14);
		usb_pma_int();
		return;
	}

	if (USB->ISTR & (1 << 13)) {
		usb_err_int();
		USB->ISTR = ~(1 << 13);
		return;
	}

	if (USB->ISTR & (1 << 10)) {
		USB->ISTR = ~(1 << 10);
		usb_reset_int();
		return;
	}
	if (USB->ISTR & (1 << 9)) {
		USB->ISTR = ~(1 << 9);
		usb_sof_int();
		return;
	}
	if (USB->ISTR & (1 << 8)) {
		USB->ISTR = ~(1 << 8);
		usb_esof_int();
		return;
	}

}
void USB_HP_CAN1_TX_IRQHandler(void) {
	uint16_t isr, ep;

	isr = USB->ISTR;
	if (isr & 0x8000) {
		ep = isr & 0xf;
		USB->EPR[ep] &= 0x3f;
	}
}
