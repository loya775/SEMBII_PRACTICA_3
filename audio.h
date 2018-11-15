/*
 * audio.h
 *
 *  Created on: 08/11/2018
 *      Author: alan
 */

#ifndef AUDIO_H_
#define AUDIO_H_

/*This function starts of stops the PIT that controls the DAC*/
void udp_Stop_Audio(uint8_t Open_Close);
/*This function selects a socket*/
void udp_Change_Audio(uint8_t Open_Close);
void start_timer(uint8_t StopStart);
/*Create a new task to take care of an UDP client*/
void udp_task_init(void);
/*Give a initial configuration to DAC0*/
void dac_init(void);
#endif /* AUDIO_H_ */
