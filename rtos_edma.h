/*
 * rtos_edma.h
 *
 *  Created on: 10/11/2018
 *      Author: jorge
 */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"

#include "pin_mux.h"
#include "clock_config.h"

#ifndef RTOS_EDMA_H_
#define RTOS_EDMA_H_


void edma_initialization(edma_config_t Config,DMA_Type *DMA);
void edma_transfer(edma_transfer_config_t trans_config, void *srcAddr, void *destAddr, edma_transfer_type_t type, uint16_t size);
void dmaMUX_initialization(DMAMUX_Type *base);


#endif /* RTOS_EDMA_H_ */
