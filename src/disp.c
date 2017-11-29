/*
 * disp.c
 *
 *  Created on: 13 нояб. 2017 г.
 *      Author: valeriy
 */

#define	MAXBUF		80
#define PULSE_DUTY	10
#define HINX			1
#define TINX			0
#define START		1
#define STOP			0
#define OCLOW		{TIM4->CCMR2 &= ~ 0x78;TIM4->CCMR2 |= 0x40;}
#define OCWORK		{TIM4->CCMR2 &= ~ 0x78;TIM4->CCMR2 |= 0x68;}
#include "stm32f10x.h"
#include "myDelay.h"

static void send_byte();
void lcd_cmd(char ch);
void lcd_char(char ch);

static uint8_t const font[] = {31,16,16,30,17,17,30,0,31,17,16,16,16,16,16,0,6,10,10,10,10,31,17,0, //Б,Г,Д
		21,21,14,4,14,21,21,0, 6,9,1,6,1,17,14,0,17,17,19,21,25,17,17,0,		//Ж,З,И
		15,9,9,9,9,9,17,0, 31,17,17,17,17,17,17,0, 17,17,17,15,1,17,14,0,		//Л,П,У
		14,21,21,21,14,4,4,0, 18,18,18,18,18,18,31,1, 17,17,17,15,1,1,1,0,	//Ф,Ц,Ч
		21,21,21,21,21,21,31,0, 21,21,21,21,21,21,31,1, 14,17,1,7,1,17,14,0,	//Ш,Щ,Э
		18,21,21,29,21,21,18,0, 15,17,17,15,9,9,17,0,	 0,14,16,30,17,17,14,0,	//Ю,Я,б
		0,0,28,18,30,17,30,0,0,0,31,17,16,16,16,0,0,0,6,10,10,31,17,0,		//в,г,д
		0,0,21,14,4,14,21,0, 0,0,14,1,6,17,14,0,	0,0,17,19,21,25,17,0,		//ж,з,и
		10,4,17,19,21,25,17,0, 0,0,7,9,9,9,17,0, 0,0,17,17,31,17,17,0,		//й,л,н
		0,0,31,17,17,17,17,0, 0,0,31,4,4,4,4,0, 0,0,14,21,21,14,4,0,			//п,т,ф
		0,0,18,18,18,18,31,1, 0,0,17,17,15,1,1,0, 0,0,21,21,21,21,31,0,		//ц,ч,ш
		0,0,21,21,21,21,31,1, 24,8,10,13,9,9,14,0, 0,0,17,17,25,21,25,0,		//щ,ъ,ы
		0,0,18,21,29,21,18,0, 0,0,7,9,7,9,17,0};		//ю,я

static uint16_t charbuf[MAXBUF];
static uint16_t const pwidth[] = {160,6500,180};		//ширина импульса E
static uint16_t const initbytes[] = {0x3430,0x3430,0x3430,0x3c,0x08,0x1401,0x1406,0x140e};
static volatile uint8_t hinx, tinx, fl;		// hean index, tail index

void lcd_loadfont(uint8_t const *chr, uint8_t n) {
	while (fl)
		;
	lcd_cmd(64);
	for (uint8_t i0 = 0; i0 < n; i0++) {
		for (uint8_t i1 = 0; i1 < 8; i1++) {
			lcd_char(font[(chr[i0] * 8) + i1]);
		}
	}
	lcd_cmd(1);
	while (fl)
		;

}
void inc_inx(uint8_t fl) {
	if (fl) {
		hinx++;
		if (hinx == MAXBUF) hinx = 0;
	} else {
		tinx++;
		if (tinx == MAXBUF) tinx = 0;
	}
}

void lcd_char(char ch) {
	charbuf[hinx] = ch;			// Символ просто заносим в буфер
	inc_inx(HINX);
	if (!fl) {
		send_byte();
		//pwm(START);
	}
}

void lcd_str(const char *str) {
	while (*str)
		lcd_char(*(str++));
}

void lcd_cmd(char ch) {
	uint16_t tmp;
	switch (ch) {
		case 1:
			tmp = 1;
		break;
		case 2:
			tmp = 1;
		break;
		case 3:
			tmp = 1;
		break;
/*
		case 0x80:
			tmp = 1;
		break;
*/
		default:
			tmp = 2;
		break;
	}
	charbuf[hinx] = ch | (tmp << 8);
	inc_inx(HINX);
	if (!fl) {
		send_byte();
		//pwm(START);
	}
}
void lcd_init() {
	GPIOB->CRL = 0x11111111;
	GPIOB->CRH &= 0xffffff00;
	GPIOB->CRH |= 0x19;
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	TIM4->CNT = 0;
	TIM4->DIER = 1;
	TIM4->PSC = 17;
	TIM4->ARR = 18;
	TIM4->CCR3 = 0;
	OCLOW;
//	set_oc_mode(4,2);
	TIM4->CCER |= 1 << 8;
	hinx = tinx = fl = 0;
	NVIC_EnableIRQ(TIM4_IRQn);
	for (uint8_t i1 = 0; i1 < 8; i1++) {
		lcd_cmd(initbytes[i1] & 0xff);
		myDelay((initbytes[i1] & 0xff00) >> 8);
	}
}

static void send_byte() {
	uint16_t tmp;

	tmp = charbuf[tinx] & 0xf00;
	tmp >>= 8;
	// штшциализация таймера
	OCLOW;		// OC3 inactive

 	TIM4->ARR = pwidth[tmp];
	TIM4->CCR3 = PULSE_DUTY;
	OCWORK;
	TIM4->CR1 |= 9;		// Start PWM One puls mode

	// вывод данных на шину
	GPIOB->ODR &= 0xfd00;
	GPIOB->ODR |= charbuf[tinx] & 0xff;
	GPIOB->ODR |= (tmp) ? 0 : 0x200;
	inc_inx(TINX);
	TIM4->CCR3 = 0;
	fl = 1;
}
void TIM4_IRQHandler() {
	if (TIM4->SR & 1) {
		TIM4->SR &= ~1;
		if (hinx != tinx) {
			send_byte();
		} else {
			OCLOW
			;
			fl = 0;
		}
	}

}
