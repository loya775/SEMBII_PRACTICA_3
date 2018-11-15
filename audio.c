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
#include "tcpecho.h"

#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define BUFFER_SIZE 249
#define PIT_PERIOD ((1000000U/44100U))
#define PIT_PERIOD2 ((1000000U))

static SemaphoreHandle_t dac_signal[2];
static uint8_t currentBuffer = 0, bufferIndex = 0, bufferIsReady[2] = {0};
static uint16_t dacIndex = 0;
struct netbuf *buf[2];
uint8_t count = 0;

struct netconn *conn;
uint8_t listen = 1;
uint32_t packet = 0;
/*-----------------------------------------------------------------------------------*/
static void
udp_server_thread(void *arg)
{
	dac_init();
	pit_init();
	pit_init_2();

	//struct netconn *conn;
	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, 54001);

	while(1)
	{
	  netconn_recv(conn, &buf[bufferIndex]);
	  edma_wait();
	  packet = (packet + 1)%44100;
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

void PIT1_IRQHandler(void)
{
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
	packet=0;
}
/*-----------------------------------------------------------------------------------*/
void
udp_task_init(void)
{
	sys_thread_new("server_thread", udp_server_thread, NULL, 300, 3);
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
	NVIC_SetPriority(PIT0_IRQn, 4);

	/* Start channel 0 */
	PIT_StartTimer(PIT, kPIT_Chnl_0);
}
void pit_init_2(void)
{
	pit_config_t pitConfig;

	pitConfig.enableRunInDebug = true;
	/* Init pit module */
	PIT_Init(PIT, &pitConfig);
	/* Set timer period for channel 0 */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, USEC_TO_COUNT(PIT_PERIOD2, PIT_SOURCE_CLOCK));
	/* Enable timer interrupts for channel 0 */
	PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);
	/* Enable at the NVIC */
	EnableIRQ(PIT1_IRQn);
	NVIC_SetPriority(PIT1_IRQn, 8);

	/* Start channel 0 */
	//PIT_StartTimer(PIT, kPIT_Chnl_0);
}

void udp_Stop_Audio(uint8_t Open_Close)
{
	if(Open_Close==2)
	{
		PIT_StartTimer(PIT, kPIT_Chnl_0);
	}else if(Open_Close == 1)
	{
		PIT_StopTimer(PIT, kPIT_Chnl_0);
	}
}

void udp_Change_Audio(uint8_t Open_Close)
{
	if(Open_Close==2)
	{
		netconn_bind(conn, IP_ADDR_ANY, 54001);
	}else if(Open_Close == 1)
	{
		netconn_bind(conn, IP_ADDR_ANY, 50000);
	}
}

void pit_start_timer(uint8_t StopStart)
{
	if(StopStart==1)
	{
	PIT_StartTimer(PIT, kPIT_Chnl_1);
	package_display(packet);
	}else if(StopStart==2)
	{
		PIT_StopTimer(PIT, kPIT_Chnl_1);
	}
}


