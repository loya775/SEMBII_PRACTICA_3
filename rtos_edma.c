/*
 * rtos_edma.c
 *
 *  Created on: 10/11/2018
 *      Author: jorge
 */

#include "rtos_edma.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

edma_handle_t g_EDMA_Handle;
static SemaphoreHandle_t transferComplete;

volatile bool g_Transfer_Done = false;


void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(transferComplete, &xHigherPriorityTaskWoken); //Give semaphore to gpio_handler
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void edma_initialization(edma_config_t Config,DMA_Type *DMA)
{
	EDMA_GetDefaultConfig(&Config);
	EDMA_Init(DMA, &Config);
	EDMA_CreateHandle(&g_EDMA_Handle, DMA, 0);
	EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
	NVIC_SetPriority(DMA0_IRQn, 10);
}

void edma_transfer(edma_transfer_config_t  trans_config, void *srcAddr, void *destAddr, edma_transfer_type_t type, uint16_t size)
{
	EDMA_PrepareTransfer(&trans_config, srcAddr, 1, destAddr, 1, 222, size, type);
	EDMA_SubmitTransfer(&g_EDMA_Handle, &trans_config);
	EDMA_StartTransfer(&g_EDMA_Handle);
}

void dmaMUX_initialization(DMAMUX_Type *base)
{
	transferComplete = xSemaphoreCreateBinary();
    DMAMUX_Init(base);
	#if defined(FSL_FEATURE_DMAMUX_HAS_A_ON) && FSL_FEATURE_DMAMUX_HAS_A_ON
    	DMAMUX_EnableAlwaysOn(base, 0, true);
	#else
    	DMAMUX_SetSource(base, 0, 63);
	#endif /* FSL_FEATURE_DMAMUX_HAS_A_ON */
    DMAMUX_EnableChannel(base, 0);
}

void edma_wait()
{
	xSemaphoreTake(transferComplete,portMAX_DELAY);
}
