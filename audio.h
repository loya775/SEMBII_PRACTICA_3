/*
 * audio.h
 *
 *  Created on: 08/11/2018
 *      Author: alan
 */

#ifndef AUDIO_H_
#define AUDIO_H_

void udp_Stop_Audio(uint8_t Open_Close);
void udp_Change_Audio(uint8_t Open_Close);
void pit_init_2(void);
void Package(uint8_t Open_Close);
void pit_start_timer(uint8_t StopStart);
void udp_task_init(void);
void dac_init(void);
void pit_init(void);
#endif /* AUDIO_H_ */
