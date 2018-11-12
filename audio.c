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

#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define BUFFER_SIZE 250
#define PIT_PERIOD (1000000U/2000U)

static SemaphoreHandle_t sample_signal, buffer_ready_signal;
static uint16_t buffer[2][BUFFER_SIZE];
static uint8_t currentBuffer = 0, bufferIndex = 0;

/*-----------------------------------------------------------------------------------*/
static void
udp_server_thread(void *arg)
{
  struct netconn *conn;
  struct netbuf *buf;
  LWIP_UNUSED_ARG(arg);
  conn = netconn_new(NETCONN_UDP);
  netconn_bind(conn, IP_ADDR_ANY, 54001);
  while(1)
  {
	  netconn_recv(conn, &buf);
	  netbuf_copy(buf,&buffer[bufferIndex][0],sizeof(buffer));
	  netbuf_delete(buf);
	  bufferIndex = (bufferIndex + 1)%2;
	  xSemaphoreGive(buffer_ready_signal);
  }
}

/*-----------------------------------------------------------------------------------*/
void
udp_task_init(void)
{
	buffer_ready_signal = xSemaphoreCreateBinary();
	sys_thread_new("server_thread", udp_server_thread, NULL, 1000, 5);
}

void
dac_task(void *arg)
{
	pit_config_t pitConfig;
	dac_config_t dacConfigStruct;

	DAC_GetDefaultConfig(&dacConfigStruct);
	dacConfigStruct.enableLowPowerMode = true;
	DAC_Init(DAC0, &dacConfigStruct);
	DAC_Enable(DAC0, true);             /* Enable output. */
	DAC_SetBufferReadPointer(DAC0, 0U);

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

	xSemaphoreTake(buffer_ready_signal,portMAX_DELAY);

	/* Start channel 0 */
	PIT_StartTimer(PIT, kPIT_Chnl_0);

	uint16_t dacIndex = 0;
	for(;;)
	{
		xSemaphoreTake(sample_signal,portMAX_DELAY);

		DAC_SetBufferValue(DAC0, 0U, buffer[currentBuffer][dacIndex]);
		//PRINTF("%d ",buffer[currentBuffer][dacIndex]);
		dacIndex++;

		if(dacIndex == BUFFER_SIZE)
		{
			dacIndex = 0;
			currentBuffer = (currentBuffer + 1) % 2;
		}
	}
}

void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    xSemaphoreGiveFromISR(sample_signal, 0); //Give semaphore to gpio_handler
    //PIT_DisableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
}

void
dac_task_init(void)
{
	sample_signal = xSemaphoreCreateBinary();
	sys_thread_new("dac_task", dac_task, NULL, 300, 4);
}
