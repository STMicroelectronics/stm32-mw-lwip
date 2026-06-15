/**
  ******************************************************************************
  * @file           : custom_interface.h
  * @brief          : LwIP STM32 custom interface.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LWIP_CUSTOM_INTERFACE_H
#define LWIP_CUSTOM_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Exported functions ------------------------------------------------------- */
err_t custom_ethernetif_init(struct netif *netif);
void custom_ethernetif_input(struct netif *netif);

#endif /* LWIP_CUSTOM_INTERFACE_H */