/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "tcpecho.h"
#include "Menu.h"
#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
#include "fsl_pit.h"


#define GET_ARGS(args,type) *((type*)args)

uint32_t size = sizeof("1. Detener/Reproducir audio\n2. Seleccionar audio\n3. Desplegar estadisticas de la comunicacion\n");
uint32_t* Menu = (uint32_t*)"1. Detener/Reproducir audio\n2. Seleccionar audio\n3. Desplegar estadisticas de la comunicacion\n";

uint32_t size1 = sizeof("Ingresa un valor valido entre 1 y 3\n");
uint32_t* NoValue = (uint32_t*)"Ingresa un valor valido entre 1 y 3\n";

uint32_t size2 = sizeof("1. Detener\n2. Continuar\n");
uint32_t* NoValue2 = (uint32_t*)"1. Detener\n2. Continuar\n";

uint32_t size3 = sizeof("Audio\n1. Audio 1\n2. Audio 2\n");
uint32_t* NoValue3 = (uint32_t*)"Audio\n1. Audio 1\n2. Audio 2\n";

uint32_t size4 = sizeof("Paquete recibidos:\nPaquetes perdidos\n Calidad de Comunicación\n1. Calcular\n2. Detener\n3. Salir");
uint32_t* NoValue4 = (uint32_t*)"Paquete recibidos:\nPaquetes perdidos\n Calidad de Comunicación\n1. Calcular\n2. Detener\n3. Salir";

uint32_t size5 = sizeof("Paquete recibidos:\n");
uint32_t* NoValue5 = (uint32_t*)"Paquete recibidos:\n";

uint32_t size6 = sizeof("Paquetes perdidos\n");
uint32_t* NoValue6 = (uint32_t*)"Paquetes perdidos\n";

uint32_t size7 = sizeof("Calidad de Comunicación\n");
uint32_t* NoValue7 = (uint32_t*)"Calidad de Comunicación\n";

uint32_t size8 = sizeof("00000\n");
uint32_t* NoValue8 = (uint32_t*)"00000\n";

//void* pVoid;
uint8_t *MenuValue;
uint8_t MenuValue1;
uint8_t *RcvForMenu;
uint8_t RcvForMenu1;
struct netbuf *buftcp;
void *rcvdata;
u16_t rcvlen;
struct netconn *newconnG;
err_t err1;
/*-----------------------------------------------------------------------------------*/


static void 
tcpecho_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 63000);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 63000);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
    newconnG=newconn;
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {
      void *data;
      u16_t len;
      netconn_write(newconn, Menu, size, NETCONN_COPY);

      while ((err = netconn_recv(newconn, &buftcp)) == ERR_OK) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buftcp, &data, &len);
             MenuValue = data;
             MenuValue1 = (*MenuValue-48);
             if ((MenuValue1) > 0 && (MenuValue1))
             {
            	 choose_function((MenuValue1-1));
             }else
             {
            	 netconn_write(newconn, NoValue, size1, NETCONN_COPY);
             }
             //err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buftcp) >= 0);
        netbuf_delete(buftcp);
      }
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void printf_menu1(void)
{
	netconn_write(newconnG, NoValue2, size2, NETCONN_COPY);
}

void printf_menu2(void)
{
	netconn_write(newconnG, NoValue3, size3, NETCONN_COPY);
}

void printf_menu3( )
{
	netconn_write(newconnG, NoValue4, size4, NETCONN_COPY);
}

void package_display(uint32_t Rcv_Package )
{
	//uint8_t msg[] = "Paquetes recibidos =      \n\rPaquetes perdidos =   \n\rCalidad de comunicacion =   ";

	uint32_t LostPackage;
	uint32_t PercentPackage;
	uint32_t* pRcvPackage;
	uint32_t* pLostPackage;
	uint32_t* pPercent;
	uint32_t RcvPackageA[7];
	uint32_t LostPackageA[6];
	uint32_t PercentA[4];
	PIT_StopTimer(PIT, kPIT_Chnl_1);
	RcvPackageA[0] = (Rcv_Package/10000)+48;
	RcvPackageA[1] = (((Rcv_Package-((RcvPackageA[0]-48)*10000))/1000)+48);
	RcvPackageA[2] = ((Rcv_Package-((RcvPackageA[0]-48)*10000)-((RcvPackageA[1]-48)*1000))/100)+48;
	RcvPackageA[3] = ((Rcv_Package-((RcvPackageA[0]-48)*10000)-((RcvPackageA[1]-48)*1000)-((RcvPackageA[2]-48)*100))/10)+48;
	RcvPackageA[4] = ((Rcv_Package-((RcvPackageA[0]-48)*10000)-((RcvPackageA[1]-48)*1000)-((RcvPackageA[2]-48)*100)-((RcvPackageA[3]-48)*10)))+48;
	RcvPackageA[0] += (RcvPackageA[1]<<8) + (RcvPackageA[2]<<16) + (RcvPackageA[3] <<24);
	RcvPackageA[1] = RcvPackageA[4] + 0xA00;
	pRcvPackage = &RcvPackageA[0];
	LostPackage = 44100-Rcv_Package;

	PercentPackage = ((Rcv_Package*100)/44100);
	netconn_write(newconnG, NoValue5, size5, NETCONN_COPY);
	netconn_write(newconnG, pRcvPackage, 5, NETCONN_COPY);

	LostPackageA[0] = (LostPackage/10000)+48;
	LostPackageA[1] = ((LostPackage-((LostPackageA[0]-48)*10000))/1000)+48;
	LostPackageA[2] = ((LostPackage-((LostPackageA[0]-48)*10000)-((LostPackageA[1]-48))*1000)/100)+48;
	LostPackageA[3] = ((LostPackage-((LostPackageA[0]-48)*10000)-((LostPackageA[1]-48))*1000-((LostPackageA[2]-48)*100))/10)+48;
	LostPackageA[4] = ((LostPackage-((LostPackageA[0]-48)*10000)-((LostPackageA[1]-48))*1000-((LostPackageA[2]-48)*100)-((LostPackageA[3]-48)*10)))+48;
	LostPackageA[0] += (LostPackageA[1]<<8) + (LostPackageA[2]<<16) + (LostPackageA[3] <<24);
	LostPackageA[1] = LostPackageA[4] + 0xA00;
	pLostPackage = &LostPackageA[0];

	netconn_write(newconnG, NoValue6, size6, NETCONN_COPY);
	netconn_write(newconnG, pLostPackage, 5, NETCONN_COPY);

	PercentA[0] = (PercentPackage/100)+48;
	PercentA[1] = ((PercentPackage-((PercentA[0]-48)*100))/10)+48;
	PercentA[2] = ((PercentPackage-((PercentA[0]-48)*100)-((PercentA[1]-48)*10)))+48;
	PercentA[0] += (PercentA[1]<<8) + (PercentA[2]<<16);
	pPercent = &PercentA[0];
	netconn_write(newconnG, NoValue7, size7, NETCONN_COPY);
	netconn_write(newconnG, pPercent, 3, NETCONN_COPY);
	PIT_StartTimer(PIT, kPIT_Chnl_1);
}

uint8_t receiveDataForMenu(void)
{
	netbuf_delete(buftcp);
	if((err1 = netconn_recv(newconnG, &buftcp)) == ERR_OK)
	{
		netbuf_data(buftcp, &rcvdata, &rcvlen);
		RcvForMenu = rcvdata;
		RcvForMenu1 = (*RcvForMenu-48);
	}else
	{
	   netconn_close(newconnG);
	   netconn_delete(newconnG);
	   RcvForMenu1=3;
	}
	return RcvForMenu1;
}

void
tcpecho_init(void)
{
	//static Menu_handle_t Handles;
	sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, 2000, 3);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
