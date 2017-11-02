//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "stm32f10x.h"
#include "myDelay.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 em pty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define LED_PIN		0x100

#define LED_OFF		GPIOB->BSRR |= LED_PIN
#define LED_ON		GPIOB->BSRR |= (LED_PIN << 16)
//void initialise_monitor_handles(void);
void hw_init();

int main(int argc, char* argv[]) {
//	initialise_monitor_handles();
	// At this stage the system clock should have already been configured
	// at high speed.
	myDelay_init();
	hw_init();
//	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	LED_ON;
	myDelay(3000);
	LED_OFF;
//	printf("Hello Semi\n");
//	trace_puts("Hello ARM World!");
	// Infinite loop
	usb_init();
	while (1) {
//		if (GPIOC->IDR & (1 << 11)) LED_ON; else LED_OFF;
//		myDelay(1000);
	}
}
void EXTI15_10_IRQHandler() {
	if (EXTI->PR & 0x800) {
		EXTI->PR |= 0x800;
	}
	if (GPIOC->IDR & 0x800) GPIOA->BSRR = 0x20;
	else GPIOA->BSRR = (0x20 << 16);
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
