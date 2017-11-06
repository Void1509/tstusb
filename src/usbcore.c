/*
 * usbcore.c
 *
 *  Created on: 26 янв. 2017 г.
 *      Author: valeriy
 */

#include "stm32f10x.h"

#define MYUSBLIB
#include "usb.h"

#define SET_LINE_CODING	0x20
#define GET_LINE_CODING	0x21
#define SET_CONTROL_LINE_STATE	0x22

#define EP0		USB->EPR[0]

#define 	DSETADDR		1
#define	POST0		2
#define	POSTDATA		3
#define BUF0SIZE		8
void ep_init();
#pragma pack(push,1)
union {
	USBReqestType req;
	uint8_t buf[8];
} reqBuf;
#pragma pack(pop)

static struct {			// структура для передачи данных
	uint8_t service;		// кто запрашивал запись
	uint8_t *baddr;
	uint16_t count;
} writeData;

static struct {			// структура для чтения данных
	uint8_t service;		// кто запрашивал чтение
	uint8_t *baddr;
	uint16_t count;
} readData;

uint8_t linecoding[8];		// line coding параметры передачи данных

static uint8_t readBuf[16];
static uint8_t readcount = 0;
static uint16_t DeviceAddress, saveTXst, saveRXst;

const uint8_t *strdesc[] = { StringLangID, StringVendor, StringProduct,
		StringSerial };

static uint16_t stat, in_stat, bmap;
void EP0Interrupt();
void getDesc();
void setup_process();
void getConfig();
void setConfig();
void getStatus();
void getdsc(uint8_t q);

uint8_t getCommBuff(uint8_t *dst) {
	uint8_t * tmp = readBuf;
	uint8_t ctmp = readcount;
	while (readcount--)
		*(dst++) = *(tmp++);
	readcount = 0;
	return ctmp;
}
uint8_t getCommCount() {
	return readcount;
}

void usb_ctr_int() {
	stat = USB->ISTR;
	switch (stat & 0xf) {
	case 0:
		saveRXst = (USB->EPR[0] & 0x3000) >> STRX;
		saveTXst = (USB->EPR[0] & 0x30) >> STTX;
		EP0Interrupt();
		setStatRx(0, saveRXst);
		setStatTx(0, saveTXst);
		break;
	case 1:
		clrCTR_tx(1);
		break;
	case 3:
		clrCTR_rx(3);
		pma2usr(readBuf, getTableRxAddr(3), getTableRxCount(3));
		readcount = getTableRxCount(3);
		setStatRx(3, VALID);
		break;
	default:
		clrCTR_rx(3);
//		getdsc((stat & 0xf) | 0x80);
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

	ep_init();
	in_stat = bmap = 0;
	DeviceAddress = 0;
	readcount = 0;
	readData.count = 0;
	writeData.count = 0;
}
void usb_sof_int() {
	stat = USB->ISTR;
}
void usb_esof_int() {
	stat = USB->ISTR;
}
void post0tx() {
	setTxCount(0, 0);
	if (!(USB->EPR[0] & 0x40))
		toggleTx(0);
	saveTXst = VALID;
	saveRXst = VALID;
}
void EP0Interrupt() {
	if (USB->ISTR & DIR) {
		clrCTR_rx(0);			//  прием данных
		if (EP0& SETUP) {
			pma2usr(reqBuf.buf, getTableRxAddr(0), getTableRxCount(0));
			setup_process();
		} else {
			if (!getTableRxCount(0)) {
				saveRXst = VALID;
				readData.baddr = 0;
				readData.count = 0;
			} else {
				uint8_t s = readData.service;
				switch (s) {
					case SET_LINE_CODING:
					pma2usr(linecoding, getTableRxAddr(0), readData.count);
					readData.service = 0;
					readData.count = 0;
					break;
				}
				post0tx();
			}
		}
	} else {
		clrCTR_tx(0);			// передача данных
		if (writeData.count) { 	// Проверяем есть ли пакет для отправки
			// Если да - отправляем
			uint8_t tmp = (writeData.count > BUF0SIZE) ? BUF0SIZE : (writeData.count & 0x3ff);
			usr2pma(writeData.baddr, getTableTxAddr(0), tmp);
			writeData.baddr += tmp;
			writeData.count -= tmp;
			setTxCount(0, tmp);
			saveTXst = VALID;
		} else {
			if (DeviceAddress && ((USB->DADDR & 0x7f) == 0)) {
				USB->DADDR = 0x80 | DeviceAddress;
			}
			setTxCount(0, 0);
			saveTXst = NAK;
			saveRXst = VALID;
		}
	}
}

void setup_process() {
	uint8_t sw = reqBuf.req.bReq;
	switch (sw) {
	case 0:			// GET_STATUS
		bmap |= 1;
		getStatus();
		break;
	case 1:
		if (reqBuf.req.wIndex) {
			if (reqBuf.req.wIndex == 0x82) {
				setEPType(2, EP_INT);
				post0tx();
			} else {
				setEPType(reqBuf.req.wIndex & 0xf, EP_BULK);
				if (reqBuf.req.wIndex & 0x80) {
//					uint16_t bf = 0x0a0d;
//					usr2pma((uint8_t*)&bf, getTableTxAddr(1), 2);
					setTxCount(1, 0);
					setStatTx(1, NAK);
					post0tx();
				} else {
					setStatRx(3, VALID);
					post0tx();
				}
			}
		}
		break;
	case 5:			// SET_ADDRESS
		bmap |= 8;
		DeviceAddress = reqBuf.req.wValue;
		setTxCount(0, 0);
//		setkind();
//		setStatTx(0, VALID);
		setEPType(1, EP_BULK);
		setEPType(2, EP_INT);
		setEPType(3, EP_BULK);
		setStatTx(1, NAK);
		setStatTx(2, SDIS);
		setStatRx(3, VALID);
		saveTXst = VALID;
		saveRXst = VALID;
		break;
	case 6:			// GET_DESCRIPTOR
		bmap |= 0x10;
		getDesc();
		break;
	case 8:			// GET_CONFIGURATION
		bmap |= 0x40;
		getConfig();
		break;
	case 9:			// SET_CONFIGURATION
		bmap |= 0x80;
		setConfig();
		break;
	case SET_LINE_CODING:
		readData.service = SET_LINE_CODING;
		readData.baddr = linecoding;
		readData.count = reqBuf.req.wLen;
		saveRXst = VALID;
		break;
	case GET_LINE_CODING:
		usr2pma(linecoding, getTableTxAddr(0), reqBuf.req.wLen);
		writeData.baddr = 0;
		writeData.count = 0;
		setTxCount(0, reqBuf.req.wLen);
		saveTXst = VALID;
		break;
	case SET_CONTROL_LINE_STATE:
//		setEPType(1,EP_BULK);
//		setStatRx(1, VALID);
		post0tx();
		break;
	default:
		if (reqBuf.req.wLen == 0) {
			writeData.count = 0;
			setTxCount(0, 0);
			saveTXst = VALID;
		} //else getdsc(sw);
		break;
	}
}
void getDescriptor(uint8_t *desc) {
	uint16_t len = reqBuf.req.wLen;
	uint16_t gsize = (len > BUF0SIZE) ? BUF0SIZE : len;
	usr2pma(desc, getTableTxAddr(0), gsize);
	writeData.baddr = &desc[gsize];
	writeData.count = len - gsize;
	setTxCount(0, gsize);
	saveTXst = VALID;
}
void getdsc(uint8_t q) {
	in_stat = q;

	writeData.count = 0;
	setTxCount(0, 0);
	saveTXst = VALID;
}
void getDesc() {
	uint8_t sw;
	sw = reqBuf.req.wValue >> 8;
	switch (sw) {
	case 1:
		getDescriptor((uint8_t*) DeviceDescriptor);
		break;
	case 2:
		getDescriptor((uint8_t*) ConfigDescriptor);
		break;
	case 3:
		getDescriptor((uint8_t*) strdesc[reqBuf.req.wValue & 0xff]);
		break;
	default:
		getdsc(sw);
		break;
	}
}
void getConfig() {
	uint8_t cfg = 1;
	usr2pma(&cfg, getTableTxAddr(0), 1);
	setTxCount(0, 1);
	writeData.count = 0;
	saveTXst = VALID;
}
void getStatus() {
	uint16_t cfg = 1;
	usr2pma((uint8_t*) &cfg, getTableTxAddr(0), 2);
	setTxCount(0, 2);
	writeData.count = 0;
	saveTXst = VALID;
}
void setConfig() {
	writeData.count = 0;
	setTxCount(0, 0);
	saveTXst = VALID;
}
