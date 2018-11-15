/*
 * pit.h
 *
 *  Created on: 15/11/2018
 *      Author: alan
 */

#ifndef PIT_H_
#define PIT_H_

#include "pit.h"
#include "fsl_pit.h"
#include "board.h"
#include "pin_mux.h"

void pit_init(pit_chnl_t channel, uint64_t period,uint8_t priority);
void pit_start_timer(pit_chnl_t channel);
void pit_stop_timer(pit_chnl_t channel);

#endif /* PIT_H_ */
