/*
 * audio.c
 *
 *  Created on: 08/11/2018
 *      Author: alan
 */

#include "lwip/opt.h"
#include "audio.h"
#include "fsl_pit.h"
#include "board.h"

#include "fsl_dac.h"
#include "fsl_common.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "lwip/api.h"
#include "lwip/sys.h"

#include "rtos_edma.h"

#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define BUFFER_SIZE 249
#define PIT_PERIOD (1000000U/44100U)

static SemaphoreHandle_t dac_signal[2];
static uint8_t currentBuffer = 0, bufferIndex = 0, bufferIsReady[2] = {0};
static uint16_t dacIndex = 0;
struct netbuf *buf[2];
uint8_t count = 0;
/*-----------------------------------------------------------------------------------*/
static void
udp_server_thread(void *arg)
{
	dac_init();
	pit_init();

	struct netconn *conn;
	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, 54001);

	while(1)
	{
	  netconn_recv(conn, &buf[bufferIndex]);
	  edma_wait();
	  //PRINTF("RB%d\n\r",bufferIndex);
	  bufferIsReady[bufferIndex] = 1;
	  bufferIndex = (bufferIndex + 1)%2;
	  //PRINTF("TDS%d\n\r",bufferIndex);
	  xSemaphoreTake(dac_signal[bufferIndex],portMAX_DELAY);
	  netbuf_delete(buf[bufferIndex]);
	  //PRINTF("RDS%d\n\r",bufferIndex);
	}
}

void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);

    if(bufferIsReady[currentBuffer])
    {
		uint16_t *temp = (uint16_t*) buf[currentBuffer]->ptr->payload;

		if(*(temp + dacIndex) > 4095 || *(temp + dacIndex) < 0)
		{
			*(temp + dacIndex) = 0;
		}

		DAC_SetBufferValue(DAC0, 0U, *(temp + dacIndex));
		dacIndex++;

		if(dacIndex == BUFFER_SIZE)
		{
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			//PRINTF("GDS%d\n\r",currentBuffer);
			xSemaphoreGiveFromISR(dac_signal[currentBuffer],&xHigherPriorityTaskWoken);

			dacIndex = 0;
			bufferIsReady[currentBuffer] = 0;
			currentBuffer = (currentBuffer + 1) % 2;

			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
    }
}
/*-----------------------------------------------------------------------------------*/
void
udp_task_init(void)
{
	sys_thread_new("server_thread", udp_server_thread, NULL, 1000, 10);
}

void dac_init(void)
{
	dac_config_t dacConfigStruct;

	dac_signal[0] = xSemaphoreCreateBinary();
	dac_signal[1] = xSemaphoreCreateBinary();
	xSemaphoreGive(dac_signal[1]);

	DAC_GetDefaultConfig(&dacConfigStruct);
	dacConfigStruct.enableLowPowerMode = true;
	DAC_Init(DAC0, &dacConfigStruct);
	DAC_Enable(DAC0, true);             /* Enable output. */
	DAC_SetBufferReadPointer(DAC0, 0U);
}

void pit_init(void)
{
	pit_config_t pitConfig;

	pitConfig.enableRunInDebug = true;
	/* Init pit module */
	PIT_Init(PIT, &pitConfig);
	/* Set timer period for channel 0 */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(PIT_PERIOD, PIT_SOURCE_CLOCK));
	/* Enable timer interrupts for channel 0 */
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	/* Enable at the NVIC */
	EnableIRQ(PIT0_IRQn);
	NVIC_SetPriority(PIT0_IRQn, 5);

	/* Start channel 0 */
	PIT_StartTimer(PIT, kPIT_Chnl_0);
}
