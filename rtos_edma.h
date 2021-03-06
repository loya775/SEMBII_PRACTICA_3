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

/*Applies a given EDMA configuration to a DMA*/
void edma_initialization(edma_config_t Config,DMA_Type *DMA);
/*Starts a DMA transfer*/
void edma_transfer(edma_transfer_config_t trans_config, void *srcAddr, void *destAddr, edma_transfer_type_t type, uint16_t size);
/*Gives an initial configuration to a DMAMUX*/
void dmaMUX_initialization(DMAMUX_Type *base);
/*Takes a semaphore and waits for transference to be completed*/
void edma_wait();

#endif /* RTOS_EDMA_H_ */
