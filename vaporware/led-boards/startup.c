#include "config.h"
#include "debug.h"
#include "error.h"
#include "heat.h"
#include "led.h"
#include "main.h"
#include "usart.h"

#include "stm_include/stm32/rcc.h"
#include "stm_include/stm32/gpio.h"
#include "stm_include/stm32/nvic.h"
#include "stm_include/stm32/systick.h"

/*
 * Forward declaration of interrupt handlers.
 */
void startup();
static void systick();
void unexpected_interrupt();
void serious_error();
void ignore();

/*
 * Symbols provided by the linker.
 */
extern int _stack_end;
extern int _data_start;
extern int _data_flash_start;
extern int _data_flash_end;
extern int _bss_start;
extern int _bss_end;

/*
 * The interrupt vector. This goes at the very start of Flash (starting at
 * 0x0800 0000)
 */
void (*volatile interrupts[])() __attribute__ ((section (".isr_vector"))) = {
	(void (*)()) &_stack_end, /* 0x0000: top of stack */
	startup, /* 0x0004: reset */
	serious_error, /* 0x0008: NMI */
	serious_error, /* 0x000c: hard fault */
	serious_error, /* 0x0010: memory management fault */
	serious_error, /* 0x0014: bus fault */
	serious_error, /* 0x0018: illegal instruction */
	serious_error, /* 0x001c: reserved */
	serious_error, /* 0x0020: reserved */
	serious_error, /* 0x0024: reserved */
	serious_error, /* 0x0028: reserved */
	serious_error, /* 0x002c: system call (unused) */
	serious_error, /* 0x0030: debug */
	serious_error, /* 0x0034: reserved */
	serious_error, /* 0x0038: system call pendable */
	systick, /* 0x003c: systick */
	unexpected_interrupt, /* 0x0040: unused */
	unexpected_interrupt, /* 0x0044: unused */
	unexpected_interrupt, /* 0x0048: unused */
	unexpected_interrupt, /* 0x004c: unused */
	unexpected_interrupt, /* 0x0050: unused */
	unexpected_interrupt, /* 0x0054: unused */
	unexpected_interrupt, /* 0x0058: unused */
	unexpected_interrupt, /* 0x005c: unused */
	unexpected_interrupt, /* 0x0060: unused */
	unexpected_interrupt, /* 0x0064: unused */
	unexpected_interrupt, /* 0x0068: unused */
	unexpected_interrupt, /* 0x006c: DMA1 channel 1 */
	unexpected_interrupt, /* 0x0070: DMA1 channel 2 */
	unexpected_interrupt, /* 0x0074: DMA1 channel 3 */
	unexpected_interrupt, /* 0x0078: DMA1 channel 4 */
	unexpected_interrupt, /* 0x007c: DMA1 channel 5 */
	unexpected_interrupt, /* 0x0080: DMA1 channel 6 */
	unexpected_interrupt, /* 0x0084: DMA1 channel 7 */
	ignore, /* 0x0088: ADC1 */
	unexpected_interrupt, /* 0x008c: unused */
	unexpected_interrupt, /* 0x0090: unused */
	unexpected_interrupt, /* 0x0094: unused */
	unexpected_interrupt, /* 0x0098: unused */
	unexpected_interrupt, /* 0x009c: unused */
	unexpected_interrupt, /* 0x00a0: unused */
	unexpected_interrupt, /* 0x00a4: unused */
	unexpected_interrupt, /* 0x00a8: unused */
	unexpected_interrupt, /* 0x00ac: unused */
	unexpected_interrupt, /* 0x00b0: unused */
	unexpected_interrupt, /* 0x00b4: unused */
	unexpected_interrupt, /* 0x00b8: unused */
	unexpected_interrupt, /* 0x00bc: unused */
	unexpected_interrupt, /* 0x00c0: unused */
	unexpected_interrupt, /* 0x00c4: unused */
	unexpected_interrupt, /* 0x00c8: unused */
	unexpected_interrupt, /* 0x00cc: unused */
	unexpected_interrupt, /* 0x00d0: unused */
	unexpected_interrupt, /* 0x00d4: USART1 */
	isr_usart2, /* 0x00d8: USART2 */
	unexpected_interrupt, /* 0x00dc: unused */
	unexpected_interrupt, /* 0x00e0: unused */
	unexpected_interrupt, /* 0x00e4: unused */
	unexpected_interrupt, /* 0x00e8: unused */
	unexpected_interrupt, /* 0x00ec: unused */
	unexpected_interrupt, /* 0x00f0: unused */
	unexpected_interrupt, /* 0x00f4: unused */
	unexpected_interrupt, /* 0x00f8: reserved */
	unexpected_interrupt, /* 0x00fc: reserved */
	unexpected_interrupt, /* 0x0100: unused */
	unexpected_interrupt, /* 0x0104: unused */
	unexpected_interrupt, /* 0x0108: unused */
	unexpected_interrupt, /* 0x010c: unused */
	unexpected_interrupt, /* 0x0110: unused */
	unexpected_interrupt, /* 0x0114: unused */
	unexpected_interrupt, /* 0x0118: unused */
	unexpected_interrupt, /* 0x011c: unused */
	unexpected_interrupt, /* 0x0120: unused */
};

/*
 * Initializes the processor's clock tree.
 */
static inline void clock_init() {
	// PLL source: Quartz via Prediv.
	// PLL multiplication factor 3 (8MHz -> 24MHz)
	// ADC prescaler 2
	// other prescalers off
	RCC_CFGR |= (RCC_CFGR_PLLSRC_PREDIV1_CLK << 16) |
		(RCC_CFGR_PLLMUL_PLL_CLK_MUL3 << 18);

	// Start and wait for HSE.
	RCC_CR |= RCC_CR_HSEON;
	while (!(RCC_CR & RCC_CR_HSERDY));

	// Start and wait for PLL.
	RCC_CR |= RCC_CR_PLLON;
	while (!(RCC_CR & RCC_CR_PLLRDY));

	// Switch sysclock to PLL.
	RCC_CFGR |= (RCC_CFGR_SW_SYSCLKSEL_PLLCLK << 0); // Assuming that HSI was selected before.

	// Enable peripherals:
	//  - Flash programming
	//  - SRAM clock
	//  - DMA 1 and 2
	RCC_AHBENR |= RCC_AHBENR_FLITFEN |
		RCC_AHBENR_SRAMEN |
		RCC_AHBENR_DMA2EN |
		RCC_AHBENR_DMA1EN;

	// Enable peripherals:
	//  - Timers (17, 16, 15,1)
	//  - USART 1
	//  - ADC 1
	//  - IO ports D, C, B, A
	//  - Alternate function IO
	RCC_APB2ENR |= RCC_APB2ENR_TIM17EN |
		RCC_APB2ENR_TIM16EN |
		RCC_APB2ENR_TIM15EN |
		RCC_APB2ENR_USART1EN |
		RCC_APB2ENR_TIM1EN |
		RCC_APB2ENR_ADC1EN |
		RCC_APB2ENR_IOPDEN |
		RCC_APB2ENR_IOPCEN |
		RCC_APB2ENR_IOPBEN |
		RCC_APB2ENR_IOPAEN |
		RCC_APB2ENR_AFIOEN;

	// Enable peripherals
	//  - Power interface
	//  - USART 2
	//  - Timers (3, 2)
	RCC_APB1ENR |= RCC_APB1ENR_PWREN |
		RCC_APB1ENR_USART2EN |
		RCC_APB1ENR_TIM3EN |
		RCC_APB1ENR_TIM2EN;

	// Set systick timer to 0.1s
	// Divisor of 299999 at 3MHz
	STK_LOAD = 299999;

	// Systick will be enabled when the heat check is initialized.
}

/*
 * Initializes the IO port remapping.
 */
static void remap_init() {
	// Alternate function, push-pull, 10 MHz
	// CNF1 = 1, CNF0 = 0, MODE = 01
	
	GPIOA_CRL = (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 2) | // TIM2_CH1
		(GPIO_MODE_OUTPUT_10_MHZ << 0) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 6) |     // TIM2_CH2
		(GPIO_MODE_OUTPUT_10_MHZ << 4) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 10) |    // USART2_TX
		(GPIO_MODE_OUTPUT_10_MHZ << 8) |
		(GPIO_CNF_INPUT_PULL_UPDOWN << 14) |        // USART2_RX
		(GPIO_CNF_INPUT_ANALOG << 18) |             // ADC1_IN4
		(GPIO_CNF_INPUT_ANALOG << 22) |             // ADC1_IN5
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 30) |    // TIM1_CH1N
		(GPIO_MODE_OUTPUT_10_MHZ << 28);

	GPIOA_CRH = (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 6) | // USART1_TX
		(GPIO_MODE_OUTPUT_10_MHZ << 4) |
		(GPIO_CNF_INPUT_PULL_UPDOWN << 10) |        // USART1_RX
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 14) |    // TIM1_CH4
		(GPIO_MODE_OUTPUT_10_MHZ << 12);

	GPIOB_CRL = (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 2) | // TIM1_CH2N
		(GPIO_MODE_OUTPUT_10_MHZ << 0) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 6) |     // TIM1_CH3N
		(GPIO_MODE_OUTPUT_10_MHZ << 4);

	GPIOB_CRH = (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 2) | // TIM16_CH1
		(GPIO_MODE_OUTPUT_10_MHZ << 0) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 6) |     // TIM17_CH1
		(GPIO_MODE_OUTPUT_10_MHZ << 4) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 10) |    // TIM2_CH3
		(GPIO_MODE_OUTPUT_10_MHZ << 8) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 14) |    // TIM2_CH4
		(GPIO_MODE_OUTPUT_10_MHZ << 12) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 26) |    // TIM15_CH1
		(GPIO_MODE_OUTPUT_10_MHZ << 24) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 30) |    // TIM15_CH2
		(GPIO_MODE_OUTPUT_10_MHZ << 28);

	GPIOC_CRL = (GPIO_CNF_INPUT_ANALOG << 2) |          // ADC1_IN10
		(GPIO_CNF_INPUT_ANALOG << 6) |              // ADC1_IN11
		(GPIO_CNF_INPUT_ANALOG << 10) |             // ADC1_IN12
		(GPIO_CNF_INPUT_ANALOG << 14) |             // ADC1_IN13
		(GPIO_CNF_INPUT_ANALOG << 18) |             // ADC1_IN14
		(GPIO_CNF_INPUT_ANALOG << 22) |             // ADC1_IN15
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 26) |    // TIM3_CH1
		(GPIO_MODE_OUTPUT_10_MHZ << 24) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 30) |    // TIM3_CH2
		(GPIO_MODE_OUTPUT_10_MHZ << 28);

	GPIOC_CRH = (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 2) | // TIM3_CH3
		(GPIO_MODE_OUTPUT_10_MHZ << 0) |
		(GPIO_CNF_OUTPUT_ALTFN_PUSHPULL << 6) |     // TIM3_CH4
		(GPIO_MODE_OUTPUT_10_MHZ << 4) |
		(GPIO_CNF_OUTPUT_PUSHPULL << 18) |          // debug LED
		(GPIO_MODE_OUTPUT_10_MHZ << 16);

	// Nothing to do for GPIOD_CRL

	AFIO_MAPR = AFIO_MAPR_TIM3_REMAP_FULL_REMAP |
		AFIO_MAPR_TIM2_REMAP_PARTIAL_REMAP2 |
		AFIO_MAPR_TIM1_REMAP_PARTIAL_REMAP;

	AFIO_MAPR2 = AFIO_MAPR2_TIM15_REMAP; // TODO: Put this into the header file.

}

/*
 * Initialisation code. Bootstraps the processor and initializes RAM.
 */
void startup() {
	clock_init();

	remap_init();

#ifdef TRACE_STARTUP
	// As the RAM sections are not yet initialized, there is nothing
	// else we can do to signal startup complete.
	dled_on();
#endif

	int *src, *dest;
	for (src = &_data_flash_start, dest = &_data_start;
	     src < &_data_flash_end;
	     src++, dest++) {
		*dest = *src;
	}

#ifdef TRACE_STARTUP
	dled_off();
#endif

	for (int *bss = &_bss_start; bss < &_bss_end; bss++) {
		*bss = 0;
	}

#ifdef TRACE_STARTUP
	dled_on();
#endif

	main();
	
	while (1);
}

/*
 * Interrupt handler for the systick. This calls the heat checking function
 * from time to time.
 */
void __attribute__ ((interrupt("IRQ"))) systick() {
#ifndef OMIT_HEAT_CHECK
	if (do_heat_check) {
		// There has been no heat check since last time the
		// flag was set.
		error(ER_BUG, STR_WITH_LEN("No heat check happened"), EA_RESUME);
	}
	do_heat_check = 1;
#endif
}

/*
 * Handler for unexpected interrupts. This is reported via the normal error function.
 */
void __attribute__ ((interrupt("IRQ"))) unexpected_interrupt() {
	int iabr;
	
	while (1) {
		dled_on();
		debug_string("Unexpected interrupt\n");

		for (int i = 0; i < 3; i++) {
			iabr = NVIC_IABR(i);
			debug_write((char*) &iabr, 4);
		}
	}
}

/*
 * Handler for serious exceptions.Probably, thing are so bad that we cannot even
 * report the error. Just die.
 */
void __attribute__ ((interrupt("IRQ"))) serious_error() {
	led_set_state(LED_STOP);

	// The processor is running at 24MHz, so counting to 12000000 gives about
	// 2s LED blinking time (assuming 4 cycles for the loop), which is
	// separate from every other pattern.
	
	while(1) {
		dled_on();
		for (int i = 0; i < 12000000; i++) {
			__asm("nop");
		}
		dled_off();
		for (int i = 0; i < 12000000; i++) {
			__asm("nop");
		}
	}
}

/*
 * Handler for interrupts which are to be ignored.
 */
void __attribute__ ((interrupt("IRQ"))) ignore() {
	return;
}
