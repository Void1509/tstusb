/*
 * hw_init.c
 *
 *  Created on: 25 янв. 2017 г.
 *      Author: valeriy
 */

#include "stm32f10x.h"
#include "usb.h"


void beep();

extern uint8_t denois;
uint8_t key;

void hw_init() {
	// USB CLK Init инициализация тактирования USB
	RCC->APB2ENR |=
			RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	AFIO->MAPR |= (2 << 24);				// JTAG disable; SWD Enable

//	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_SetPriority(EXTI0_IRQn, ((3 << 2) | 0));	// Priority 3, subpriority 0
	NVIC_SetPriority(EXTI1_IRQn, ((3 << 2) | 1));	// Priority 3, subpriority 1
	NVIC_SetPriority(EXTI15_10_IRQn, ((3 << 2) | 2));	// Priority 3, subpriority 2
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, ((2 << 2) | 0));	// Priority 2, subpriority 0
	NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, ((1 << 2) | 0));// Priority 1, subpriority 0
	NVIC_SetPriority(TIM4_IRQn, ((2 << 2) | 1));		// Priority 2, subpriority 1
	// USB Pin Config
	GPIO_InitTypeDef io;
	io.GPIO_Mode = GPIO_Mode_AF_PP;
	io.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	io.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &io);

	// Настройка порта B
	GPIOB->CRL = 0x11111111;		//0..7 out 10MHz
	GPIOB->CRH = 0x88888819;		// 8 - TIM4 (E) 9 - RS; 10..15 Encoder input
	GPIOB->BSRR = 0xFC00;		// 10..15 PULL-UP


	// кнопки PC0 PC1 input pull up
	GPIOC->CRL &= 0xffffff00;
	GPIOC->CRL |= 0x88;
	GPIOC->ODR = 3;

	AFIO->EXTICR[0] = 0x22;
	// EXTI port B int 10,12,13,15
	AFIO->EXTICR[2] = 0x100;
	AFIO->EXTICR[3] = 0x1011;
	EXTI->IMR = 0xB403;
	EXTI->FTSR = 0xB403;
	EXTI->RTSR = 0;
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	key = 0;
}

void EXTI15_10_IRQHandler() {
	if (EXTI->PR & 0x400) {
		key = (GPIOB->IDR & 0x800)?0x40:0x41;
		EXTI->PR = 0x400;
	}
	if (EXTI->PR & 0x2000) {
		key = (GPIOB->IDR & 0x4000)?0x42:0x43;
		EXTI->PR = 0x2000;
	}
	if (EXTI->PR & 0x1000) {
		key = 0x44;
		EXTI->PR = 0x1000;
	}
	if (EXTI->PR & 0x8000) {
		key = 0x45;
		EXTI->PR = 0x8000;
	}
	beep();
}

uint8_t getkey() {
	uint8_t tmp = key;
	key = 0;
	return tmp;
}
void EXTI1_IRQHandler() {
	EXTI->PR |= EXTI_PR_PR1;
	if (!denois) {
		key = 0x31;
		beep();
		denois = 100;
	}
}
void EXTI0_IRQHandler() {
	EXTI->PR |= EXTI_PR_PR0;
	if (!denois) {
		key = 0x30;
		beep();
		denois = 100;
	}
}

