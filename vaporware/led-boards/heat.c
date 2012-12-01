#include "heat.h"

#include "config.h"
#include "error.h"
#include "fail.h"

#include "stm_include/stm32/adc.h"
#include "stm_include/stm32/dma.h"
#include "stm_include/stm32/nvic.h"
#include "stm_include/stm32/systick.h"

/*
 * The function to be called when overheat occurs.
 */
static heat_handler_t overheat_handler;

/*
 * The samples got from the ADC will be writter here by DMA.
 */
static int adc_samples[HEAT_SENSOR_LEN];

/*
 * The failure logs for each heat sensor
 */
static fail_t heat_fails[HEAT_SENSOR_LEN];

/*
 * Returns the sequence register where the n'th conversion
 * is to be stored. If n = 17, the sequence length field is returned.
 */
static volatile uint32_t *seq_register_for(unsigned int n) {
	if (n > 17) {
		error(ER_BUG, STR_WITH_LEN("ADC sequence too long"), EA_PANIC);
	}

	if (n <= 6) {
		return &ADC1_SQR3;
	} else if (n <= 12) {
		return &ADC1_SQR2;
	} else {
		return &ADC1_SQR1;
	}
}

/*
 * Returns the bit position in the sequence register where
 * the n'th conversion is to be stored. If n = 17 the position
 * of the sequence length field is returned.
 */
static int bit_position_for(unsigned int n) {
	if (n > 17) {
		error(ER_BUG, STR_WITH_LEN("ADC sequence too long"), EA_PANIC);
	}

	// Each register contains the following conversions (for one k)
	// 6k + 1 at bits  4 to  0
	// ...
	// 6k + 6 at bits 29 to 25
	// This formula follows:
	return ((n-1) % 6) * 5;
}

/*
 * Initializes temperature sensors. When this function has been called,
 * a heat check will be done at each system timer tick.
 */
void heat_init(heat_handler_t on_overheat) {
	overheat_handler = on_overheat;

	for (int s = 0; s < HEAT_SENSOR_LEN; s++) {
		fail_init(&heat_fails[s], HEAT_FAIL_TRESHOLD);
	}

	// Configure the ADC in the following way:
	// Put the values from HEAT_ADC_PORTS into the sequence registers
	// Start ADC in scan mode with continuous bit set.
	// Configure DMA to write samples to adc_samples.

	// Write ADC ports into sequence registers.
	_Static_assert(HEAT_SENSOR_LEN <= 16, "Too many heat sensors!");
	for (int i = 0; i < HEAT_SENSOR_LEN; i++) {
		*seq_register_for(i) = (HEAT_ADC_PORTS[i] & 0xf) << bit_position_for(i);
	}
	// Write length into sequence length field.
	*seq_register_for(17) = HEAT_SENSOR_LEN << bit_position_for(17);

	// Set up ADC
	ADC1_CR1 = ADC_CR1_SCAN | // Scan mode
		ADC_CR1_EOCIE;    // End of conversion interrupt enable
	ADC1_CR2 = ADC_CR2_CONT | // Continuous mode.
		ADC_CR2_DMA;      // DMA mode
	ADC1_SMPR1 = ADC_SAMPLE_TIME_1;
	ADC1_SMPR2 = ADC_SAMPLE_TIME_2;
	

	// Set up DMA (using channel 1 of DMA 1 here)
	DMA1_CPAR1 = (uint32_t) &ADC1_DR;
	DMA1_CMAR1 = (uint32_t) &adc_samples;
	DMA1_CNDTR1 = HEAT_SENSOR_LEN;
	DMA1_CCR1 = (DMA_CCR1_PL_HIGH << DMA_CCR1_PL_LSB) |    // High priority
		(DMA_CCR1_MSIZE_16BIT << DMA_CCR1_MSIZE_LSB) | // 16 bit memory size
		(DMA_CCR1_PSIZE_16BIT << DMA_CCR1_PSIZE_LSB) | // 16 bit peripheral size
		DMA_CCR1_MINC |                                // Memory auto-increment
		DMA_CCR1_CIRC |                                // Circular mode
		DMA_CCR1_EN;                                   // enable

	// Enable ADC interrupt in NVIC
	NVIC_ISER(0) |= (1 << NVIC_ADC1_2_IRQ);

	ADC1_CR2 |= ADC_CR2_ADON;

	// Enable Systick timer, enable interrupt.
	// (heat_timer_tick will be called at each systick)
	STK_CTRL = STK_CTRL_TICKINT | STK_CTRL_ENABLE;
}

/*
 * This function does a heat check on every system timer tick.
 */
void heat_timer_tick() {
	for (int s = 0; s < HEAT_SENSOR_LEN; s++) {
		if (fail_event(&heat_fails[s], (adc_samples[s] > config.heat_limit[s]))) {
			overheat_handler();
			return;
		}
	}
}
