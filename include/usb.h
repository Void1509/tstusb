/*
 * usb.h
 *
 *  Created on: 8 июн. 2017 г.
 *      Author: valeriy
 */

#ifndef USB_H_
#define USB_H_

#define USB_BASE	((uint32_t)0x40005C00)
#define USB_PBUFFER	((uint32_t)0x40006000)
#define EP_BULK			0
#define EP_CONTROL		1
#define EP_ISO			2
#define EP_INT			3
#define STALL			1
#define NAK				2
#define VALID			3
#define	RXCNT(bsize,nblock)		(uint16_t)(((bsize & 1) << 15) | ((nblock & 31) << 10))

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
#endif /* USB_H_ */
