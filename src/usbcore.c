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
struct {
	uint16_t daddr;		// device addr
	uint8_t * baddr;		// buffer addres to TX (IN)
	uint16_t count;		// count tx bytes
} usbData;
struct {
	uint8_t	service;		// кто запрашивал чтение
	uint8_t *baddr;
	uint16_t count;
} readData;

uint8_t linecoding[8];

const uint8_t *strdesc[] = { StringLangID, StringVendor, StringProduct,
		StringSerial };

static uint16_t stat, saveRXst, saveTXst, in_stat, bmap;
void EP0Int();
void getDesc();
void setup_process();
void getConfig();
void setConfig();
void getStatus();
void getdsc(uint8_t q);

void usb_ctr_int() {
	stat = USB->ISTR;
	switch (stat & 0xf) {
	case 0:
		saveRXst = (USB->EPR[0] & 0x3000) >> STRX;
		saveTXst = (USB->EPR[0] & 0x30) >> STTX;
		EP0Int();
		setStatRx(0, saveRXst);
		setStatTx(0, saveTXst);
		break;
	case 1:
		clrCTR_tx(1);
		break;
	case 3:
		clrCTR_rx(3);
		break;
	default:
		getdsc((stat & 0xf) | 0x80);
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
	in_stat = usbData.daddr = bmap = 0;
	setEPType(3,EP_BULK);
	setStatRx(3,VALID);
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
void EP0Int() {
	if (USB->ISTR & DIR) {
		clrCTR_rx(0);			// OUT Process (RX)
		pma2usr(reqBuf.buf, getTableRxAddr(0), getTableRxCount(0));
		if (USB->EPR[0] & SETUP) {
			setup_process();			// Получили пакет SETUP
		} else {
			// Получили пакет обычный
			if (getTableRxCount(0) == 0) {

				saveRXst = VALID;
				usbData.baddr = 0;
				usbData.count = 0;
			} else {
				uint8_t s = readData.service;
				switch (s) {
					case SET_LINE_CODING:
						pma2usr(linecoding, getTableRxAddr(0), readData.count);
						readData.count = 0;
						post0tx();
						break;
				}
			}
		}
	} else {
		clrCTR_tx(0);			// IN Process (TX)
		if (usbData.count & 0x3ff) { 	// Проверяем есть ли пакет для отправки
			// Если да - отправляем
			uint8_t tmp =
					((usbData.count & 0x3ff) > BUF0SIZE) ?
							BUF0SIZE : (usbData.count & 0x3ff);
			usr2pma(usbData.baddr, getTableTxAddr(0), tmp);
			usbData.baddr += tmp;
			usbData.count -= tmp;
			setTxCount(0, tmp);
			saveRXst = NAK;
			saveTXst = VALID;
		} else {		// Если нет
			if (((USB->DADDR & 0x7f) == 0) && (usbData.daddr)) {
				// Проверяем установлен ли адрес устройства
				// Если нет устанавливаем
				USB->DADDR = 0x80 | usbData.daddr;
				saveRXst = VALID;
			} else {
				// и отправляем нулевой пакет
				setTxCount(0, 0);
				if (!(USB->EPR[0] & 0x40))
					toggleTx(0);
				saveTXst = VALID;
				saveRXst = VALID;
			}
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
		usbData.daddr = reqBuf.req.wValue;
		usbData.count &= 0xf000;
		setTxCount(0, 0);
		saveTXst = VALID;
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
		usbData.baddr = 0;
		usbData.count = 0;
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
			usbData.count = 0;
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
	usbData.baddr = &desc[gsize];
	usbData.count = len - gsize;
	setTxCount(0, gsize);
	saveTXst = VALID;
}
void getdsc(uint8_t q) {
	in_stat = q;

	usbData.count = 0;
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
	usbData.count = 0;
	saveTXst = VALID;
}
void getStatus() {
	uint16_t cfg = 1;
	usr2pma((uint8_t*) &cfg, getTableTxAddr(0), 2);
	setTxCount(0, 2);
	usbData.count = 0;
	saveTXst = VALID;
}
void setConfig() {
	usbData.count = 0;
	setTxCount(0, 0);
	saveTXst = VALID;
}
