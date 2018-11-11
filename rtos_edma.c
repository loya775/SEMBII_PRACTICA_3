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
    //xSemaphoreGiveFromISR(transferComplete, 0); //Give semaphore to gpio_handler
    xSemaphoreGive(transferComplete); //Give semaphore to gpio_handler

}


void edma_initialization(edma_config_t Config,DMA_Type *DMA)
{
	EDMA_GetDefaultConfig(&Config);
	EDMA_Init(DMA, &Config);
	EDMA_CreateHandle(&g_EDMA_Handle, DMA, 0);
	EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
}

void edma_transfer(edma_transfer_config_t  trans_config, void *srcAddr, void *destAddr, edma_transfer_type_t type, uint8_t size)
{

	//uint32_t srcf = *src;
	//AT_NONCACHEABLE_SECTION_INIT(uint32_t src[4]) = {*(srcAddr+1),,,};
	//AT_NONCACHEABLE_SECTION_INIT(uint32_t recv[4]) = {};

	EDMA_PrepareTransfer(&trans_config, srcAddr, 1, destAddr, 1,
	                        1, size, type);
	EDMA_SubmitTransfer(&g_EDMA_Handle, &trans_config);
	EDMA_StartTransfer(&g_EDMA_Handle);
	xSemaphoreTake(transferComplete,portMAX_DELAY);

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

void edma_transfer_nbytes(edma_transfer_config_t  trans_config, void *srcAddr, void *destAddr, edma_transfer_type_t type, uint32_t size)
{
	uint32_t index = 0;
	uint8_t tempSize, tempOffset;
	while(size > 0)
	{
		tempSize = size % 16;
		if (size == 0)
		{
			break;
		}

		else if(size <= 32)
		{

		    switch (tempSize - tempOffset)
		    {
		        case 1U:
		        	edma_transfer(trans_config, srcAddr + index, destAddr + index,  type, 1);
		        	size -= 1;
		        	index += 1;
		        	tempOffset = 0;
		        	break;
		        case 2U:
		        	edma_transfer(trans_config, srcAddr + index, destAddr + index,  type, 2);
		        	size -= 2;
		        	index += 2;
		        	tempOffset = 0;
		            break;
		        case 4U:
		        	edma_transfer(trans_config, srcAddr + index, destAddr + index,  type, 4);
		        	size -= 4;
		        	index += 4;
		        	tempOffset = 0;
		            break;
		        case 0:
		        	edma_transfer(trans_config, srcAddr + index, destAddr + index,  type, 16);
		        	size -= 16;
		        	index += 16;
		        	tempOffset = 0;
		            break;
		        default:
		        	tempOffset++;
		            break;
		    }
		}

		else
		{
			edma_transfer(trans_config, srcAddr + index, destAddr + index,  type, 32);
			size -= 32;
			index += 32;
		}

	}
}

