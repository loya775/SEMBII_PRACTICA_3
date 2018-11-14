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
#define BUFFER_SIZE 499
#define PIT_PERIOD 199U//(1000000U/5000U)

static SemaphoreHandle_t sample_signal, buffer_ready_signal[2], dac_signal[2];
static uint8_t currentBuffer = 0, bufferIndex = 0;
struct netbuf *buf[2];
uint8_t count = 0;
/*-----------------------------------------------------------------------------------*/
static void
udp_server_thread(void *arg)
{
  struct netconn *conn;
  LWIP_UNUSED_ARG(arg);
  conn = netconn_new(NETCONN_UDP);
  netconn_bind(conn, IP_ADDR_ANY, 54001);

  while(1)
  {
	  netconn_recv(conn, &buf[bufferIndex]);
	  edma_wait();
	  //PRINTF("RB%d\n\r",bufferIndex);
	  xSemaphoreGive(buffer_ready_signal[bufferIndex]);
	  bufferIndex = (bufferIndex + 1)%2;
	  //PRINTF("TDS%d\n\r",bufferIndex);
	  xSemaphoreTake(dac_signal[bufferIndex],portMAX_DELAY);
	  netbuf_delete(buf[bufferIndex]);
	  //PRINTF("RDS%d\n\r",bufferIndex);
  }
}

/*-----------------------------------------------------------------------------------*/
void
udp_task_init(void)
{
	buffer_ready_signal[0] = xSemaphoreCreateBinary();
	buffer_ready_signal[1] = xSemaphoreCreateBinary();
	sys_thread_new("server_thread", udp_server_thread, NULL, 1000, 10);
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

	xSemaphoreTake(buffer_ready_signal[0],portMAX_DELAY);
	/* Start channel 0 */
	PIT_StartTimer(PIT, kPIT_Chnl_0);

	uint16_t dacIndex = 0;
	for(;;)
	{
		xSemaphoreTake(sample_signal,portMAX_DELAY);
		uint16_t *temp = (uint16_t*) buf[currentBuffer]->ptr->payload;

		if(*(temp + dacIndex) > 4095 || *(temp + dacIndex) < 0)
		{
			*(temp + dacIndex) = 0;
		}

		DAC_SetBufferValue(DAC0, 0U, *(temp + dacIndex));
		dacIndex++;

		if(dacIndex == BUFFER_SIZE)
		{
			//uint16_t *temp2 = (uint16_t*) buf[(currentBuffer + 1) % 2]->ptr->payload;
			//PRINTF("CB%d %d %d\n\r",currentBuffer,*(temp+1),*(temp + dacIndex - 2));
			//PRINTF("NB %d %d\n\r",*(temp2+1),*(temp2 + dacIndex - 2));
			dacIndex = 0;
			//PRINTF("GDS%d\n\r",currentBuffer);
			xSemaphoreGive(dac_signal[currentBuffer]);
			currentBuffer = (currentBuffer + 1) % 2;
			//PRINTF("TBS%d\n\r",currentBuffer);
			xSemaphoreTake(buffer_ready_signal[currentBuffer],portMAX_DELAY);
			//PRINTF("RBS%d\n\r",currentBuffer);
		}
	}
}

void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    xSemaphoreGiveFromISR(sample_signal, 0); //Give semaphore to gpio_handler
}

void
dac_task_init(void)
{
	dac_signal[0] = xSemaphoreCreateBinary();
	dac_signal[1] = xSemaphoreCreateBinary();
	sample_signal = xSemaphoreCreateBinary();
	xSemaphoreGive(dac_signal[1]);
	sys_thread_new("dac_task", dac_task, NULL, 300, 4);
}
