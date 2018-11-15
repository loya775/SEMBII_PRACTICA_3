/*
 * Menu.c
 *
 *  Created on: 10/11/2018
 *      Author: jorge
 */
#include "Menu.h"
#include "audio.h"
#include "tcpecho.h"

uint8_t FlagMenu1=0;
uint8_t FlagMenu2=0;
uint8_t FlagMenu3=0;
uint8_t Valor;
typedef const struct Function
		{
			void(*fptr)(uint8_t StopStart);
			uint8_t state;
		}Function;

		const Function FSMMoore[3]=
		{
				{Menu_StopReady,1},
				{Menu_SelectAudio,2},
				{Menu_Display,3},

};

		void Menu_StopReady(uint8_t StopStart)
		{
			Valor=0;
			printf_menu1();
			while(Valor != 3)
			{
			Valor = receiveDataForMenu();
			udp_Stop_Audio(Valor);
			}
		}

		void Menu_SelectAudio(uint8_t StopStart)
		{
			Valor = 0;
			printf_menu2();
			while(Valor != 3)
			{
				Valor = receiveDataForMenu();
				udp_Change_Audio(Valor);
			}


		}
		void Menu_Display(uint8_t StopStart)
		{
			Valor = 0;
			printf_menu3();
			while(Valor != 3)
			{
				Valor = receiveDataForMenu();
				pit_start_timer(Valor);
			}

		}

		void choose_function(uint8_t State)
		{
			FSMMoore[State].fptr(State);
		}
