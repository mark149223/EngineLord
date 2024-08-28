/**
 ******************************************************************************
 * @file      net.c
 * @brief     Ethernet link functions
 ******************************************************************************
 */

#include "main.h"
#include "net.h"
#include "common.h"
#include "peripheral_manager.h"
//-----------------------------------------------
extern struct netif gnetif;
extern ip4_addr_t ipaddr;

extern BlindMessage * pbmessage;
extern Parser blind_parser;
extern uint8_t IsMessageArrived;

// убрать
extern BlindMessage bmessage;
// убрать
extern uint16_t driver_step;
// убрать
extern TIM_HandleTypeDef htim1;

struct udp_pcb *upcb;

uint8_t client_ip_addr_mass[4]={192,168,99,251};
ip4_addr_t server_ip_addr;
ip4_addr_t client_ip_addr;
u16_t server_port=65002;
u16_t client_port=60196;
//-----------------------------------------------




void InitIpAdrr(void)
{
	server_ip_addr = ipaddr;
	IP4_ADDR(&client_ip_addr,
			client_ip_addr_mass[0],
			client_ip_addr_mass[1],
			client_ip_addr_mass[2],
			client_ip_addr_mass[3]);
}

void UDPReceiveCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port)
{
	/*
	 * https://stackoverflow.com/questions/
	 * 16831605/strange-compiler-warning-c-warning-struct-declared-inside-parameter-list
	 */
	UNUSED(upcb);

	/*
	 * В дальнейшем сообщения будут парситься исключительно из кольцевого буфера
	 * Здесь будет происходить лишь добавление в него
	 */


	/*TODO: скачать или украсть аллокатор памяти для статического выделения*/
	char * payload = reinterpret_cast<char *>(p->payload);
	uint16_t len = p->len;


	blind_parser.parse(payload, len, pbmessage);
//
	IsMessageArrived=1;
	pbuf_free(p);

}

//void UDPTransportInit(uint32_t* addr,uint32_t* port)
void UDPTransportInit()
{

//  ip_addr_t Localaddr;	// just struct with u32_t field
  err_t err;
  netif_set_ipaddr(&gnetif,&server_ip_addr);
  udp_remove(upcb);
  // Инициализирует udp pcb. Pcb не активно пока не будет связано с адрессом удаленного устройства
  upcb=udp_new();
  if (upcb!=NULL)
  {
  	err =udp_bind(upcb,&server_ip_addr, server_port);

  	if (err == ERR_OK)
  	{
  		udp_recv(upcb, UDPReceiveCallback, NULL);
  	}

	udp_connect(upcb, &client_ip_addr,client_port);



  }
}


void UDPSend(unsigned char*pTx,uint16_t len)
{
  struct pbuf *p;
  p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM );
  if (p != NULL)
  {
	  //copy payload pTx into pbuf
    pbuf_take(p, (void *) pTx, len);
    udp_send(upcb, p);

    pbuf_free(p);
  }
}
