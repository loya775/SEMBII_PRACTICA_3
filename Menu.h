/*
 * Menu.h
 *
 *  Created on: 10/11/2018
 *      Author: jorge
 */

#ifndef MENU_H_
#define MENU_H_
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include <stdint.h>
#include "tcpecho.h"
#include "lwip/opt.h"

void Menu_StopReady();
void Menu_SelectAudio();
void Menu_Display();
void choose_function(uint8_t State);


#endif /* MENU_H_ */
