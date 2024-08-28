/**
 ******************************************************************************
 * @file      net.h
 * @brief     Ethernet link functions
 ******************************************************************************
 */

#ifndef INC_NET_H_
#define INC_NET_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "lwip.h"
#include "lwip/udp.h"
/* Exported variables ----------------------------------------------------------*/

/* Exported definitions ----------------------------------------------------------*/


/* Exported functions ----------------------------------------------------------*/
void InitIpAdrr(void);
void UDPReceiveCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
void UDPTransportInit();
void UDPSend(unsigned char*pTx,uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* INC_NET_H_ */
