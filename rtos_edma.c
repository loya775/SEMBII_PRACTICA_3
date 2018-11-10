/*
 * rtos_edma.c
 *
 *  Created on: 10/11/2018
 *      Author: jorge
 */

#include "rtos_edma.h"




edma_handle_t g_EDMA_Handle;
volatile bool g_Transfer_Done = false;


void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
    }
}


void edma_initialization(edma_config_t Config,DMA_Type *DMA)
{
	EDMA_GetDefaultConfig(&Config);
	EDMA_Init(DMA, &Config);
	EDMA_CreateHandle(&g_EDMA_Handle, DMA, 0);
	EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
}

void edma_transfer(edma_transfer_config_t  trans_config, uint32_t *srcAddr, uint32_t *destAddr, edma_transfer_type_t type)
{

	//uint32_t srcf = *src;
	//AT_NONCACHEABLE_SECTION_INIT(uint32_t src[4]) = {*(srcAddr+1),,,};
	//AT_NONCACHEABLE_SECTION_INIT(uint32_t recv[4]) = {};

	EDMA_PrepareTransfer(&trans_config, srcAddr, 4, destAddr, 4,
	                        4, 16, type);
	EDMA_SubmitTransfer(&g_EDMA_Handle, &trans_config);
	EDMA_StartTransfer(&g_EDMA_Handle);
    while (g_Transfer_Done != true)
    {
    }
}

void dmaMUX_initialization(DMAMUX_Type *base)
{
    DMAMUX_Init(base);
	#if defined(FSL_FEATURE_DMAMUX_HAS_A_ON) && FSL_FEATURE_DMAMUX_HAS_A_ON
    	DMAMUX_EnableAlwaysOn(base, 0, true);
	#else
    	DMAMUX_SetSource(base, 0, 63);
	#endif /* FSL_FEATURE_DMAMUX_HAS_A_ON */
    DMAMUX_EnableChannel(base, 0);
}

