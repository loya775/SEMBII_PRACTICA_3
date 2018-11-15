/*
 * pit.c
 *
 *  Created on: 15/11/2018
 *      Author: alan
 */

#include "pit.h"
#include "fsl_pit.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

void pit_init(pit_chnl_t channel, uint64_t period,uint8_t priority)
{
	pit_config_t pitConfig;

	pitConfig.enableRunInDebug = true;
	/* Init pit module */
	PIT_Init(PIT, &pitConfig);
	/* Set timer period for channel 0 */
	PIT_SetTimerPeriod(PIT, channel, USEC_TO_COUNT(period, PIT_SOURCE_CLOCK));
	/* Enable timer interrupts for channel 0 */
	PIT_EnableInterrupts(PIT, channel, kPIT_TimerInterruptEnable);
	/* Enable at the NVIC */
	EnableIRQ(PIT0_IRQn + channel);
	NVIC_SetPriority(PIT0_IRQn + channel, priority);

	/* Start channel 0 */
	PIT_StartTimer(PIT, channel);
}

void pit_start_timer(pit_chnl_t channel)
{
	PIT_StartTimer(PIT, channel);
}

void pit_stop_timer(pit_chnl_t channel)
{
	PIT_StopTimer(PIT, channel);
}
