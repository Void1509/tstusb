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
	uint16_t buf[4];
} reqBuf;
#pragma pack(pop)
struct {
	uint16_t daddr;		// device addr
	uint8_t * baddr;		// buffer addres to TX (IN)
	uint16_t count;		// count tx bytes
} usbData;
volatile uint16_t dcount = 0;
volatile uint16_t stat,lens[128];
static uint16_t saveRXst, saveTXst, in_stat, bmap;
void EP0Interrupt();
void getDesc();
void getDeviceDesc(USBReqestType *ur);
void getConfigDesc();
void getStringDesc();

uint16_t *pbufcpy(uint16_t *dst, uint8_t ep) {
	uint16_t * raddr = dst;
	uint16_t addr = (getTableRxAddr(ep) >> 1);
	uint16_t cnt = (getTableRxCount(ep) >> 1);
	while (cnt--)
		*(raddr++) = PBuffer[addr++].mem;
	return dst;
}
void userpbuf(uint8_t *src, uint16_t addr, uint16_t cnt) {
	uint16_t tmp, taddr;
	taddr = addr >> 1;
	cnt &= 0xfe;
	while (cnt) {
		tmp = *(src + 1);
		tmp <<= 8;
		tmp |= *src;
		PBuffer[taddr++].mem = tmp;
		src += 2;
		cnt -= 2;
	}
}
void pbufuser(uint8_t *dst, uint16_t addr, uint16_t cnt) {
	uint16_t tmp, taddr;
	taddr = addr >> 1;
	cnt &= 0xfe;
	while (cnt) {
		tmp = PBuffer[taddr++].mem;
		*(dst++) = tmp & 0xff;
		tmp >>= 8;
		*(dst++) = tmp & 0xff;
		cnt -= 2;
	}
}

void usb_ctr_int() {
	stat = USB->ISTR;
//	pbufcpy(reqBuf.buf, 0);
	switch (stat & 0xf) {
	case 0:
		saveRXst = (USB->EPR[0] & 0x3000) >> STRX;
		saveTXst = (USB->EPR[0] & 0x30) >> STTX;
		EP0Interrupt();
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
	lens[dcount++] = dcount;
}
void usb_sof_int() {
	stat = USB->ISTR;
//	lens[dcount++] = dcount;
}
void usb_esof_int() {
	stat = USB->ISTR;
}
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
/*
			saveTXst = VALID;
			saveRXst = VALID;
*/
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
void getDesc() {
	USBReqestType ur;
	uint8_t sw;

	//pbufuser((uint8_t*) &ur, getTableRxAddr(0), getTableRxCount(0));
	pma2usr((uint8_t*)&ur,getTableRxAddr(0),getTableRxCount(0));
	sw = ur.wValue >> 8;
	switch (sw) {
	case 1:
		getDeviceDesc(&ur);
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
void getDeviceDesc(USBReqestType *ur) {

	if (ur->wLen > BUF0SIZE) {
//		userpbuf((uint8_t*) DeviceDescriptor, getTableTxAddr(0), BUF0SIZE);
		usr2pma((uint8_t*) DeviceDescriptor, getTableTxAddr(0), BUF0SIZE);
		usbData.baddr = &DeviceDescriptor[BUF0SIZE];
		usbData.count = ur->wLen - BUF0SIZE;
		in_stat = POSTDATA;
		setTxCount(0,BUF0SIZE);
		saveTXst = VALID;
	} else {
//		userpbuf((uint8_t*) DeviceDescriptor, getTableTxAddr(0), ur->wLen);
		usr2pma((uint8_t*) DeviceDescriptor, getTableTxAddr(0), ur->wLen);
		in_stat = POSTDATA;
		usbData.count = 0;
		setTxCount(0,ur->wLen);
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
	userpbuf((uint8_t*) ConfigDescriptor, getTableTxAddr(0), BUF0SIZE);
	usbData.baddr = &ConfigDescriptor[BUF0SIZE];
	usbData.count = 32 - BUF0SIZE;
	in_stat = POSTDATA;
	setTxCount(0, BUF0SIZE);
	saveTXst = VALID;
}
void getStringDesc() {
	uint8_t *sel;
	uint8_t cnt;

	sel = StringLangID;

	cnt = sel[0];
	if (cnt > BUF0SIZE) {
		userpbuf((uint8_t*) sel, getTableTxAddr(0), BUF0SIZE);
		usbData.baddr = &sel[BUF0SIZE];
		usbData.count = cnt - BUF0SIZE;
		in_stat = POSTDATA;
		setTxCount(0, BUF0SIZE);
		saveTXst = VALID;
	} else {
		userpbuf((uint8_t*) sel, getTableTxAddr(0), cnt);
		in_stat = POST0;
		setTxCount(0, cnt);
		saveTXst = VALID;
	}

}
