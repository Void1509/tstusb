/*
 * usbcore.c
 *
 *  Created on: 26 янв. 2017 г.
 *      Author: valeriy
 */
#define MYUSBLIB
#include "stm32f10x.h"
#include "usb.h"

#define 	DSETADDR		1
#define	POST0		2
#define	POSTDATA		3
#define BUF0SIZE		8
void ep_init();
#pragma pack(push,1)
union {
	USBReqestType req;
	uint8_t buf[4];
} reqBuf;
#pragma pack(pop)
struct {
	uint16_t daddr;		// device addr
	uint8_t * baddr;		// buffer addres to TX (IN)
	uint16_t count;		// count tx bytes
} usbData;

const uint8_t *strdesc[]={StringLangID,StringVendor,StringProduct,StringSerial};

static uint16_t stat,saveRXst, saveTXst, in_stat, bmap;
void EP0Int();
void getDesc();
void getDeviceDesc(USBReqestType *ur);
void getConfigDesc();
void getStringDesc();
void setup_process();
void getConfig();
void setConfig();

void usb_ctr_int() {
	stat = USB->ISTR;
//	pbufcpy(reqBuf.buf, 0);
	switch (stat & 0xf) {
	case 0:
		saveRXst = (USB->EPR[0] & 0x3000) >> STRX;
		saveTXst = (USB->EPR[0] & 0x30) >> STTX;
		EP0Int();
		setStatRx(0, saveRXst);
		setStatTx(0, saveTXst);
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
//	lens[dcount++] = dcount;
}
void usb_sof_int() {
	stat = USB->ISTR;
//	lens[dcount++] = dcount;
}
void usb_esof_int() {
	stat = USB->ISTR;
}
void EP0Int() {
	if (USB->ISTR & DIR) {
		clrCTR_rx(0);			// OUT Process (RX)
		pma2usr(reqBuf.buf, getTableRxAddr(0), getTableRxCount(0));
		if (USB->EPR[0] & SETUP) {
			setup_process();			// Получили пакет SETUP
		} else {
			// Получили пакет обычный
			saveRXst = VALID;
			if (usbData.baddr) {
				saveRXst = VALID;
				usbData.baddr = 0;
			}
		}
	} else {
		clrCTR_tx(0);			// IN Process (TX)
				/*
				 if (usbData.count == 0x8000) {		// TX STALLed
				 clrCTR_rx(0);
				 saveTXst = NAK;
				 saveRXst = VALID;
				 return;
				 }
				 */
		if (usbData.count & 0x3ff) { 	// Проверяем есть ли пакет для отправки
			// Если да - отправляем
			uint8_t tmp =
					((usbData.count & 0x3ff) > BUF0SIZE) ?
							BUF0SIZE : (usbData.count & 0x3ff);
			usr2pma(usbData.baddr, getTableTxAddr(0), tmp);
			usbData.baddr += tmp;
			usbData.count -= tmp;
			setTxCount(0, tmp);
			//toggleTx(0);
			in_stat += tmp;
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
	switch (reqBuf.req.bReq) {
	case 0:			// GET_STATUS
		bmap |= 1;
		break;
	case 1:			// CLEAR_FEATURE
		bmap |= 2;
		break;
	case 3:			// SET_FEATURE
		bmap |= 3;
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
	case 7:			// SET_DESCRIPTOR
		bmap |= 0x20;
		break;
	case 8:			// GET_CONFIGURATION
		bmap |= 0x40;
		getConfig();
		break;
	case 9:			// SET_CONFIGURATION
		bmap |= 0x80;
		setConfig();
		break;
	}
}
/*
void EP0Interrupt() {
	uint16_t tmp;
	if (USB->ISTR & DIR) {      // OUT reqest
		clrCTR_rx(0);
		if (USB->EPR[0] & SETUP) {
			pbufcpy(reqBuf.buf, 0);
			switch (reqBuf.req.bReq) {
			case 0:
				bmap |= 1;
				pbufcpy(reqBuf.buf, 0);
				break;
			case 1:
				bmap |= 2;
				pbufcpy(reqBuf.buf, 0);
				break;
			case 3:
				bmap |= 4;
				pbufcpy(reqBuf.buf, 0);
				break;
			case 5:
				bmap |= 8;
				usbData.daddr = reqBuf.req.wValue;
				in_stat = DSETADDR;
				setTxCount(0, 0);
				saveTXst = VALID;
				//saveRXst = STALL;
				break;
			case 6:
				bmap |= 0x10;
				getDesc();
				break;
			case 7:
				bmap |= 0x20;
				break;
			case 8:
				bmap |= 0x40;
				break;
			case 9:
				bmap |= 0x80;
				break;
			}
		} else {
			tmp = getTableRxCount(0);
			if (tmp) {
				saveRXst = VALID;
			} else {
				saveRXst = VALID;
				if (usbData.count)
					saveTXst = VALID;
			}
			//pbufcpy(reqBuf.buf, 0);
			clrCTR_rx(0);

			 saveTXst = VALID;
			 saveRXst = VALID;

		}
	} else { 				// IN Request
//	if (USB->EPR[0] & CTR_TX){
		bmap |= 0x100;
		//clrCTR_tx(0);
		switch (in_stat) {
		case DSETADDR:
			USB->DADDR = 0x80 | usbData.daddr;
			saveRXst = VALID;
			break;
		case POST0:
			setTxCount(0, 0);
			saveRXst = VALID;
			break;
		case POSTDATA:
			if (usbData.count == 0) {
				saveRXst = VALID;
				return;
			}
			if (usbData.count >= BUF0SIZE)
				tmp = BUF0SIZE;
			else
				tmp = usbData.count;
			userpbuf(usbData.baddr, getTableTxAddr(0), tmp);
			saveTXst = VALID;

			setTxCount(0, tmp);
			usbData.count -= tmp;
			usbData.baddr += tmp;
			if (usbData.count == 0)
				in_stat = POST0;
			clrCTR_rx(0);
			//saveRXst = VALID;
			//toggleTx(0);
			break;
		}
	}
}
*/
void getDesc() {
	uint8_t sw;
	sw = reqBuf.req.wValue >> 8;
	switch (sw) {
	case 1:
		getDeviceDesc(&reqBuf.req);
//		lens[dcount++] = ur.wLen;
		break;
	case 2:
		getConfigDesc();
		break;
	case 3:
		getStringDesc();
		break;
	}
}
void getConfig() {
	uint8_t cfg = 1;
	usr2pma(&cfg,getTableTxAddr(0),1);
	setTxCount(0,1);
	usbData.count = 0;
	saveTXst = VALID;
}
void setConfig() {
	usbData.count = 0;
	setTxCount(0,0);
	saveTXst = VALID;
}
void getDeviceDesc(USBReqestType *ur) {

	if (ur->wLen > BUF0SIZE) {
//		userpbuf((uint8_t*) DeviceDescriptor, getTableTxAddr(0), BUF0SIZE);
		usr2pma((uint8_t*) DeviceDescriptor, getTableTxAddr(0), BUF0SIZE);
		usbData.baddr = &DeviceDescriptor[BUF0SIZE];
		usbData.count = ur->wLen - BUF0SIZE;
		setTxCount(0, BUF0SIZE);
		saveTXst = VALID;
	} else {
//		userpbuf((uint8_t*) DeviceDescriptor, getTableTxAddr(0), ur->wLen);
		usr2pma((uint8_t*) DeviceDescriptor, getTableTxAddr(0), ur->wLen);
		usbData.count = 0;
		setTxCount(0, ur->wLen);
		saveTXst = VALID;
		saveRXst = VALID;
	}
	/*
	 usbData.baddr = &DeviceDescriptor[BUF0SIZE];
	 usbData.count = ur->wLen - BUF0SIZE;
	 in_stat = POSTDATA;
	 setTxCount(0, BUF0SIZE);
	 saveTXst = VALID;
	 */

}
void getConfigDesc() {
	uint16_t len = reqBuf.req.wLen;
	uint16_t gsize = (len > BUF0SIZE)? BUF0SIZE:len;
	usr2pma((uint8_t*) ConfigDescriptor, getTableTxAddr(0), gsize);
	usbData.baddr = &ConfigDescriptor[gsize];
	usbData.count = len - gsize;
//	in_stat = POSTDATA;
	setTxCount(0, gsize);
	saveTXst = VALID;
}
void getStringDesc() {
	uint8_t *sel;
	uint8_t cnt;

	sel = strdesc[reqBuf.req.wValue & 0xff];

	cnt = sel[0];
	if (cnt > BUF0SIZE) {
		usr2pma((uint8_t*) sel, getTableTxAddr(0), BUF0SIZE);
		usbData.baddr = &sel[BUF0SIZE];
		usbData.count = cnt - BUF0SIZE;
//		in_stat = POSTDATA;
		setTxCount(0, BUF0SIZE);
		saveTXst = VALID;
	} else {
		usr2pma((uint8_t*) sel, getTableTxAddr(0), cnt);
//		in_stat = POST0;
		setTxCount(0, cnt);
		saveTXst = VALID;
	}

}
