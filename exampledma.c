/*
 * exampledma.c
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
#include "rtos_edma.h"


#define EXAMPLE_DMA DMA0
#define EXAMPLE_DMAMUX DMAMUX0

#define BUFF_LENGTH 4U

AT_NONCACHEABLE_SECTION_INIT(uint32_t srcAddr[BUFF_LENGTH]) = {0x01, 0x02, 0x03, 0x04};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr[BUFF_LENGTH]) = {0x00, 0x00, 0x00, 0x00};


int main43(void)
{
	uint32_t i = 0;
    edma_transfer_config_t transferConfig;
    edma_config_t userConfig;

    PRINTF("EDMA memory to memory transfer example begin.\r\n\r\n");
        PRINTF("Destination Buffer:\r\n");
        for (i = 0; i < BUFF_LENGTH; i++)
        {
            PRINTF("%d\t", destAddr[i]);
        }
    dmaMUX_initialization(DMAMUX0);
    edma_initialization(userConfig,DMA0);
    edma_transfer_nbytes(transferConfig, srcAddr, destAddr, kEDMA_MemoryToMemory,4);

    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    while (1)
    {
    }
}

