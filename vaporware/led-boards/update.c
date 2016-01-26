/* Do not include flash.h, we are redefining these functions here
 * ourselves. */
#define FLASH_H
#define USART_H

#include "update.h"

#include "config.h"
#include "error.h"

#include "sync.h"

#include "stm_include/stm32/crc.h"
#include "stm_include/stm32/flash.h"
#include "stm_include/stm32/nvic.h"
#include "stm_include/stm32/scb.h"
#include "stm_include/stm32/timer.h"
#include "stm_include/stm32/usart.h"

#define UPDATE_TEXT __attribute__ ((section (".update_text")))
#define UPDATE_DATA __attribute__ ((section (".update_data")))

/*
 * Symbols provided by the linker.
 */
// extern int _flash_start;
extern int _update_start;
extern int _update_flash_start;
extern int _update_flash_end;

static void UPDATE_TEXT update_panic() {
	// TODO Make some fuss
	while(1);
}

#ifdef TRACE_UPDATE
/******************************************************************************/
/* USART1 code.                                                               */
/******************************************************************************/

static void UPDATE_TEXT usart1_putchar(unsigned char c) {
	USART1_DR = c;
	while(!(USART1_SR & USART_SR_TXE))
		;
}

/******************************************************************************/
/* End of USART1 code.                                                        */
/******************************************************************************/
#endif

/******************************************************************************/
/* This is a dumbed down copy of flash.c, moved into the .update section.     */
/******************************************************************************/

#define FLASH_START 0x08000000
#define FLASH_END   (FLASH_START + (16 * 1024))
// Size of a flash page
#define FLASH_PAGE_SIZE 1024

/*
 * Checks the flash status for errors.
 */
static error_t UPDATE_TEXT flash_check_error() {
	// Wait for write to finish
	// Need to wait one more cycle, see erratum 2.7
	__asm("nop");
	while (FLASH_SR & FLASH_BSY) {
#ifdef TRACE_UPDATE
		usart1_putchar('f');
#endif
	}

	if (FLASH_SR & (FLASH_PGERR | FLASH_WRPRTERR)) {
		return E_FLASH_WRITE;
	} else {
		return E_SUCCESS;
	}
}

/*
 * Writes a word to flash and checks it afterwards.
 */
static error_t UPDATE_TEXT flash_write_check(uint16_t *address, uint16_t value) {
	error_t error;

	// Enable programming
	FLASH_CR |= FLASH_PG;

	*address = value;

	error = flash_check_error();

	FLASH_CR &= ~FLASH_PG;

	if (error != E_SUCCESS) return error;

	if (*address != value) return E_FLASH_WRITE;

	return E_SUCCESS;
}

/*
 * Erases a page in flash. base_addr must point to the beginning of
 * the page.
 */
static error_t UPDATE_TEXT flash_erase_page(void *base_addr) {
	while(FLASH_SR & FLASH_BSY);

	if (FLASH_CR & FLASH_LOCK)
		update_panic();

	FLASH_CR |= FLASH_PER;
	FLASH_AR = (uint32_t) base_addr;
	FLASH_CR |= FLASH_STRT;

	error_t error = flash_check_error();

	FLASH_CR &= ~FLASH_PER;

	if (error) return error;

	// Verify
	uint16_t *start = (uint16_t*) base_addr;
	for (uint16_t *i = start;
	     i < start + FLASH_PAGE_SIZE/sizeof(uint16_t);
	     i++) {
		if (*i != 0xffff) {
#ifdef TRACE_UPDATE
			usart1_putchar('F');
#endif
			return E_FLASH_WRITE;
		}
	}

	return E_SUCCESS;
}

/******************************************************************************/
/* End of flash code.                                                         */
/******************************************************************************/

/******************************************************************************/
/* CRC code.                                                                  */
/******************************************************************************/

static void UPDATE_TEXT crc_reset() {
	CRC_CR = 1;
}

static void UPDATE_TEXT crc_add(uint32_t x) {
	CRC_DR = x;
}

static uint32_t UPDATE_TEXT crc_get() {
	return CRC_DR;
}

/******************************************************************************/
/* End of CRC code.                                                           */
/******************************************************************************/

typedef enum {
	UCMD_FIRST = 0xf0,
	UCMD_ERASE_PAGE = 0xf0,
	UCMD_WRITE = 0xf1,
	UCMD_CRC = 0xf2,
	UCMD_RETRY = 0xf3,
	UCMD_RESET = 0xf4,
	UCMD_LAST = 0xf5,
} update_command_t;

static void run_erase_page(void);
static void run_write(void);
static void run_crc(void);
static void run_retry(void);
static void run_reset(void);

static int UPDATE_DATA has_error = 0;

static uint16_t UPDATE_DATA *flash_ptr = NULL;

static void UPDATE_TEXT led_tick() {
	if (has_error) {
		TIM2_CCR1 = 0x0000;
	} else {
		TIM2_CCR1 = 0xffff;
	}
}

static uint8_t UPDATE_TEXT getchar() {
	while(!(USART2_SR & USART_SR_RXNE)) {
		led_tick();
	}
	uint8_t ret = USART2_DR & 0xff;
	return ret;
}

static void UPDATE_TEXT run_erase_page() {
	uint8_t page_idx = getchar();
	void *base = (void*)(FLASH_START + page_idx * FLASH_PAGE_SIZE);

	error_t err = flash_erase_page(base);
	if (err != E_SUCCESS) {
#ifdef TRACE_UPDATE
		usart1_putchar('e');
		usart1_putchar(err + '0');
#endif
		has_error = 1;
		return;
	}
	flash_ptr = base;
	crc_reset();
#ifdef TRACE_UPDATE
	usart1_putchar('E');
	usart1_putchar(page_idx + '0');
#endif
}

static void UPDATE_TEXT run_write() {
	uint8_t data[4];
	for (int i = 0; i < 4; i++) {
		data[i] = getchar();
	}

	if (!((uint16_t*)(FLASH_START) <= flash_ptr &&
	      flash_ptr < (uint16_t*)(FLASH_END))) {
		has_error = 1;
		return;
	}

	uint16_t *low  = (uint16_t*) data;
	uint16_t *high = (uint16_t*)(data + 2);

	error_t err = flash_write_check(flash_ptr++, *low);
	err |= flash_write_check(flash_ptr++, *high);
	if (err != E_SUCCESS) {
#ifdef TRACE_UPDATE
		usart1_putchar('w');
		usart1_putchar(err + '0');
#endif
		has_error = 1;
		return;
	}

	uint32_t *all = (uint32_t*) data;
	crc_add(*all);
#ifdef TRACE_UPDATE
	usart1_putchar('W');
#endif
}

static void UPDATE_TEXT run_crc() {
	uint8_t data[4];
	for (int i = 0; i < 4; i++) {
		data[i] = getchar();
	}

	uint32_t *crc_wanted = (uint32_t*) data;
	uint32_t  crc_actual = crc_get();

	if (*crc_wanted != crc_actual) {
#ifdef TRACE_UPDATE
		usart1_putchar('c');
#endif
		has_error = 1;
	}
#ifdef TRACE_UPDATE
	usart1_putchar('C');
#endif
}

static void UPDATE_TEXT run_retry() {
	has_error = 0;
	flash_ptr = NULL;
#ifdef TRACE_UPDATE
	usart1_putchar('R');
#endif
}

static void UPDATE_TEXT run_reset() {
	SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
}

static void __attribute__ ((noinline)) UPDATE_TEXT run_update() {
	// From here on, we cannot assume that anything in the .text
	// section is still available.

	while(1) {
		while(getchar() != START_MARK)
			;

		uint8_t cmd = getchar();

		if (cmd == UCMD_RETRY) {
			run_retry();
		} else if (!has_error) {
			switch(cmd) {
			case UCMD_ERASE_PAGE:
				run_erase_page();
				break;
			case UCMD_WRITE:
				run_write();
				break;
			case UCMD_CRC:
				run_crc();
				break;
			case UCMD_RESET:
				run_reset();
				break;
			default:
				has_error = 1;
#ifdef TRACE_UPDATE
				usart1_putchar('?');
#endif
				break;
			}
		}
	}
}

/*
 * Setup code.
 *
 * This code lives in the normal .text section and prepares the update process.
 */

/*
 * Unlocks the flash memory for programming.
 */
static error_t flash_unlock() {
	if (FLASH_CR & FLASH_LOCK) {
		FLASH_KEYR = FLASH_KEY1;
		FLASH_KEYR = FLASH_KEY2;

		if (FLASH_CR & FLASH_LOCK) {
			return E_FLASH_WRITE;
		}
	}

	return E_SUCCESS;
}

static void prepare_update() {
	// Disable all interrupts used by everything else.
	interrupts_off();

	if (flash_unlock() != E_SUCCESS) {
		// At this point, we can still issue a meaningful message.
		error(ER_FLASH_WRITE, STR_WITH_LEN("Could not unlock flash"), EA_PANIC);
	}

	// Prepare the LEDs for showing diagnostic messages.
	pwm_set_state(PWM_ON);

	// Copy update code to RAM
	int *src, *dest;
	for (src = &_update_flash_start, dest = &_update_start;
	     src < &_update_flash_end;
	     src++, dest++) {
		*dest = *src;
	}

}

void update_start() {
	prepare_update();
	run_update();
}
