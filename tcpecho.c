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


#define GET_ARGS(args,type) *((type*)args)

uint32_t size = sizeof("1. Detener/Reproducir audio\n2. Seleccionar audio\n3. Desplegar estadisticas de la comunicacion");
uint32_t* Menu = (uint32_t*)"1. Detener/Reproducir audio\n2. Seleccionar audio\n3. Desplegar estadisticas de la comunicacion";

uint32_t size1 = sizeof("Ingresa un valor valido entre 1 y 3");
uint32_t* NoValue = (uint32_t*)"Ingresa un valor valido entre 1 y 3";

uint32_t size2 = sizeof("1. Detener\n2. Continuar");
uint32_t* NoValue2 = (uint32_t*)"1. Detener\n2. Continuar";

uint32_t size3 = sizeof("Audio\n1. Audio 1\n2. Audio 2");
uint32_t* NoValue3 = (uint32_t*)"Audio\n1. Audio 1\n2. Audio 2";

uint32_t size4 = sizeof("Paquete recibidos:\nPaquetes perdidos\n Calidad de COmunicación");
uint32_t* NoValue4 = (uint32_t*)"Audio\n1. Audio 1\n2. Audio 2";

//void* pVoid;
uint8_t *MenuValue;
uint8_t MenuValue1;
uint32_t MenuValue2;
struct netconn *newconnG;
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
  netconn_bind(conn, IP6_ADDR_ANY, 40000);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 40000);
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
      struct netbuf *buf;
      void *data;
      u16_t len;
      netconn_write(newconn, Menu, size, NETCONN_COPY);

      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buf, &data, &len);
             MenuValue = data;
             MenuValue1 = (*MenuValue-48);
             if ((MenuValue1) > 0 && (MenuValue1))
             {
            	 choose_function((MenuValue1-1));
             }else
             {
            	 netconn_write(newconn, NoValue, size1, NETCONN_COPY);
             }
             err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
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

void printf_menu3(void)
{
	netconn_write(newconnG, NoValue4, size4, NETCONN_COPY);
}

void
tcpecho_init(void)
{
	//static Menu_handle_t Handles;
	sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
