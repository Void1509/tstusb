/*
 * hw_init.c
 *
 *  Created on: 25 янв. 2017 г.
 *      Author: valeriy
 */

#include "stm32f10x.h"
#include "usb.h"

uint8_t usbconn;

void hw_init() {
	// USB CLK Init инициализация тактирования USB
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);


	// USB Pin Config
	GPIO_InitTypeDef io;
	io.GPIO_Mode = GPIO_Mode_AF_PP;
	io.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	io.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &io);

	// port B init (leds) open Drain
	GPIOB->ODR |= 1;
	GPIOB->CRL |= 1;
	GPIOC->ODR |= (1 << 13);
	GPIOC->CRH |= (1 << 20);
	AFIO->EXTICR[0] |= (1 << 4);

	EXTI->IMR |= EXTI_IMR_MR1;
	EXTI->RTSR |= EXTI_RTSR_TR1;
	EXTI->FTSR |= EXTI_FTSR_TR1;
	NVIC_EnableIRQ(EXTI1_IRQn);
	usbconn = 0;
/*
	io.GPIO_Mode = GPIO_Mode_Out_PP;
	io.GPIO_Pin = GPIO_Pin_5;
	io.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &io);

	io.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	io.GPIO_Pin = GPIO_Pin_11;
	io.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &io);

	io.GPIO_Mode = GPIO_Mode_Out_OD;
	io.GPIO_Pin = GPIO_Pin_12;
	io.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &io);

	AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI11_PC;
	EXTI->IMR |= EXTI_IMR_MR11;
	EXTI->RTSR |= EXTI_RTSR_TR11;
	EXTI->FTSR |= EXTI_FTSR_TR11;
*/
}
void EXTI1_IRQHandler() {
	EXTI->PR |= EXTI_PR_PR1;
	if (GPIOB->IDR & 2) {
		GPIOC->BSRR = (1 << 29);
		usbconn = 1;
	} else {
		GPIOC->BSRR = (1 << 13);
		usbconn = 0;
	}
}


void intToUni(uint32_t ui, uint8_t *buf, uint8_t len) {

	uint8_t i1,i2;

	for (i1 = 0; i1 < len; i1++) {
		i2 = ui >> 28;
		buf[2*i1] = i2 + (i2 < 0xa)?'0':('A' - 10);
		ui <<= 4;
		buf[(2*i1)+1] = 0;
		}
}
