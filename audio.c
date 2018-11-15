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
#include "pit.h"

#define BUFFER_SIZE 249
#define PIT_PERIOD ((1000000U/44100U))
#define PIT_PRIO 4
#define PIT_PERIOD2 ((1000000U))
#define PIT_PRIO2 8
#define MAX_PACKETS 176 //Needs to be checked
#define NUMBER_OF_BUFFERS 2
#define MAX_DAC_VALUE 4095

static SemaphoreHandle_t dac_signal[NUMBER_OF_BUFFERS];
static uint8_t currentBuffer = 0, bufferIndex = 0, bufferIsReady[NUMBER_OF_BUFFERS] = {0};
static uint16_t dacIndex = 0;
struct netbuf *buf[NUMBER_OF_BUFFERS];
uint8_t count = 0;

struct netconn *conn;
uint8_t listen = 1;
uint32_t packet = 0;
/*-----------------------------------------------------------------------------------*/
/*This task receives the data from UDP*/
static void
udp_server_thread(void *arg)
{
	dac_init();
	pit_init(kPIT_Chnl_0,PIT_PERIOD,PIT_PRIO);
	pit_init(kPIT_Chnl_1,PIT_PERIOD2,PIT_PRIO2);

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, 54001);

	while(1)
	{
		/*Waits for a package to be received, data is copied with DMA in fsl_enet.c*/
		netconn_recv(conn, &buf[bufferIndex]);
		/*Suspends task until data is copied*/
		edma_wait();
		/*Received package count*/
		packet = (packet + 1)%44100;
		bufferIsReady[bufferIndex] = 1;
		/*Change buffer*/
		bufferIndex = (bufferIndex + 1)%NUMBER_OF_BUFFERS;
		/*Waits until current buffer is used by the DAC*/
		xSemaphoreTake(dac_signal[bufferIndex],portMAX_DELAY);
		/*Delete current buffer to store new data*/
		netbuf_delete(buf[bufferIndex]);
	}
}

void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);

    /*If we have data available in buffer*/
    if(bufferIsReady[currentBuffer])
    {
		uint16_t *temp = (uint16_t*) buf[currentBuffer]->ptr->payload;
		/*DAC protection*/
		if(*(temp + dacIndex) > MAX_DAC_VALUE || *(temp + dacIndex) < 0)
		{
			*(temp + dacIndex) = 0;
		}
		/*Update DAC value*/
		DAC_SetBufferValue(DAC0, 0U, *(temp + dacIndex));
		dacIndex++;

		/*If buffer is empty*/
		if(dacIndex == BUFFER_SIZE)
		{
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;

			xSemaphoreGiveFromISR(dac_signal[currentBuffer],&xHigherPriorityTaskWoken);

			dacIndex = 0;
			/*Change buffer and reset flag*/
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
void udp_task_init(void)
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

/*Stops audio reproduction timer*/
void udp_Stop_Audio(uint8_t Open_Close)
{
	if(Open_Close==2)
	{
		pit_start_timer(kPIT_Chnl_0);
	}else if(Open_Close == 1)
	{
		pit_stop_timer(kPIT_Chnl_0);
	}
}

/*Connects to a selected socket*/
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

/*Starts or stops pit timer 1*/
void start_timer(uint8_t StopStart)
{
	if(StopStart==1)
	{
		pit_start_timer(kPIT_Chnl_1);
		package_display(packet);
	}
	else if(StopStart==2)
	{
		pit_stop_timer(kPIT_Chnl_1);
	}
}


