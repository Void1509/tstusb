/*
 * usb.h
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: valeriy
 */

#ifndef USB_H_
#define USB_H_

#define EPCOUNT		3
#define USB_BASE	((uint32_t)0x40005C00)
#define USB_PBUFFER	((uint32_t)0x40006000)
#define EP_BULK			0
#define EP_CONTROL		0x200
#define EP_ISO			0x400
#define EP_INT			0x600
#define SDIS				0
#define STALL			1
#define NAK				2
#define VALID			3
#define	STRX				12
#define STTX				4
#define SETUP			0x800
#define DTOG_RX			0x4000
#define DTOG_TX			0x40
#define DIR				0x10
#define CTR_RX			0x8000
#define CTR_TX			0x80
#define EP_KIND			0x100

#define	RXCNT(bsize,nblock)		(uint16_t)(((bsize & 1) << 15) | ((nblock & 31) << 10))

#pragma pack(push,1)
typedef struct {
	unsigned rec:5;
	unsigned type:2;
	unsigned dir:1;
}ReqType;

typedef struct {
	ReqType bmReq;
	uint8_t bReq;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLen;
} USBReqestType;
#pragma pack(pop)
typedef struct {
	uint32_t EPR[8];
	uint32_t RESERVED[8];
	uint32_t CNTR;
	uint32_t ISTR;
	uint32_t FNR;
	uint32_t DADDR;
	uint32_t BTABLE;
}USB_TypeDef;

#define USB		((USB_TypeDef *)USB_BASE)

typedef struct {
	uint16_t mem;
	uint16_t reserv;
}PBElement;


typedef struct {
	PBElement addr;
	PBElement count;
}BTElement;

typedef struct {
	BTElement tx;
	BTElement rx;
}BTable;

#pragma pack(push,2)
typedef union {
	struct {
		unsigned count:10;
		unsigned nblok:5;
		unsigned bsize:1;
	} f;
	uint16_t word;
}RXCount;
typedef union {
     uint16_t w;
     struct BW {
     uint8_t bb1;
     uint8_t bb0;
     } bw;
} uint16_t_uint8_t;
#pragma pack(pop)

#ifdef MYUSBLIB
extern BTable *table;
extern PBElement *PBuffer;
extern uint8_t DeviceDescriptor[];
extern uint8_t ConfigDescriptor[];
extern uint8_t StringLangID[];
extern uint8_t StringVendor[];
extern uint8_t StringProduct[];
extern uint8_t StringSerial[];
#endif

void usb_init();
void setTableTx(uint8_t inx, uint16_t addr, uint16_t count);
void setTableRx(uint8_t inx, uint16_t addr, uint16_t count);
uint16_t getTableTxAddr(uint8_t ep);
uint16_t getTableRxAddr(uint8_t ep);
uint16_t getTableRxCount(uint8_t ep);
void setTxCount(uint8_t ep, uint16_t cnt);
void setRxCount(uint8_t ep, uint16_t cnt);
void setEPType(uint8_t ep, uint16_t type);
void setStatTx(uint8_t ep, uint16_t stat);
void setStatRx(uint8_t ep, uint16_t stat);
void clrCTR_tx(uint8_t ep);
void clrCTR_rx(uint8_t ep);
void toggleRx(uint8_t ep);
void toggleTx(uint8_t ep);
void usr2pma(uint8_t *src, uint16_t addr, uint16_t cnt);
void pma2usr(uint8_t *dst, uint16_t addr, uint16_t cnt);

#endif /* USB_H_ */
