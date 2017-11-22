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

static uint16_t charbuf[MAXBUF];
static uint16_t const pwidth[]={160,6500,180};		//ширина импульса E
static uint16_t const initbytes[] = {0x3430, 0x3430, 0x3430, 0x3c, 0x08, 0x1401, 0x1406, 0x140e};
static uint8_t hinx,tinx,fl;		// hean index, tail index


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
	while(*str) lcd_char(*(str++));
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
	for (int i1 = 0; i1 < 8; i1++) {
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
	GPIOB->ODR |= (tmp)?0:0x200;
	inc_inx(TINX);
	TIM4->CCR3 = 0;
	fl = 1;
}
void TIM4_IRQHandler() {
	if (TIM4->SR & 1) {
		TIM4->SR &= ~1;
		if (hinx!=tinx) {
			send_byte();
		} else {
			OCLOW;
			fl = 0;
		}
	}

}
