/*
 * Menu.c
 *
 *  Created on: 10/11/2018
 *      Author: jorge
 */
#include "Menu.h"


typedef const struct Function
		{
			void(*fptr)();
			uint8_t state;
		}Function;

		const Function FSMMoore[3]=
		{
				{Menu_StopReady,1},
				{Menu_SelectAudio,2},
				{Menu_Display,3},

};

		void Menu_StopReady()
		{
			printf_menu1();
		}

		void Menu_SelectAudio()
		{
			printf_menu2();
		}
		void Menu_Display()
		{
			printf_menu3();
		}

		void choose_function(uint8_t State)
		{
			FSMMoore[State].fptr();
		}
