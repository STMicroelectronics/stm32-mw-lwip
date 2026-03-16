/**
  ******************************************************************************
  * @file           : eth_interface_freertos.h
  * @brief          : LwIP STM32 FreeRTOS ethernet interface.
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

#ifndef LWIP_ETH_INTERFACE_FREERTOS_H
#define LWIP_ETH_INTERFACE_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "lwip/err.h"
#include "lwip/netif.h"
#include "stm32_hal.h"
#include "mx_lwip_phy.h"

#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Hardware abstraction for an Ethernet interface.
 *
 * This structure holds pointers to the hardware handle and the PHY link mode getter function.
 */
typedef struct
{
  hal_eth_handle_t *p_eth;                  /**< Pointer to the hardware ETH handle. */
  mx_phy_interface_t *p_phy;                /**< Pointer to the PHY interface. */
} lwip_eth_interface_hardware_t;

/**
 * @brief Context for a network interface.
 *
 * This structure maintains the context for a single LwIP network interface,
 * including VLAN, channel IDs, and hardware association.
 * It is intended to be store in field "state" of LwIP netif structure.
 */
typedef struct lwip_eth_interface_context_s
{
  struct netif *p_netif;                    /**< The LwIP network interface using this network device. */
  uint32_t vlan_id;                         /**< Virtual LAN number. */
  uint32_t tx_channel_id;                   /**< Transmit channel ID. */
  uint32_t rx_channel_id;                   /**< Receive channel ID. */
  lwip_eth_interface_hardware_t* p_hardware;/**< Pointer to associated hardware abstraction. */
} lwip_eth_interface_netif_context_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/**
 * @brief Initialize the Ethernet interface layer.
 *        Allocates resources and starts worker threads.
 * @return ERR_OK on success, error code otherwise.
 */
err_t lwip_eth_interface_init(void);

/**
 * @brief Deinitialize the Ethernet interface layer.
 *        Stops threads and releases resources if no interfaces remain.
 * @return ERR_OK on success, ERR_MEM if interfaces remain.
 */
err_t lwip_eth_interface_deinit(void);

/**
 * @brief Initialize the low-level hardware for a network interface.
 * @param[in] p_netif Pointer to the LwIP network interface.
 * @return ERR_OK on success, error code otherwise.
 */
err_t lwip_eth_interface_low_level_init(struct netif *p_netif);

/**
 * @brief Deinitialize the low-level hardware for a network interface.
 * @param[in] p_netif Pointer to the LwIP network interface.
 * @return ERR_OK on success, error code otherwise.
 */
err_t lwip_eth_interface_low_level_deinit(struct netif *p_netif);

/**
 * @brief Transmit an Ethernet frame (called by LwIP stack).
 * @param[in] netif Pointer to the LwIP network interface.
 * @param[in] p Pointer to the pbuf list containing the frame.
 * @return ERR_OK if the packet could be sent, ERR_BUF otherwise.
 */
err_t lwip_eth_interface_low_level_output(struct netif *netif, struct pbuf *p);
   
/**
 * @brief Notify the link monitor of a link event from an ISR context.
 *
 * This function should be called from an interrupt service routine (ISR) when a PHY link event
 * (such as link up or link down) is detected. It signals the link monitor thread to process the event.
 * This function is safe to call from ISR context.
 */
void lwip_eth_interface_link_monitor_event_notify_from_isr(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWIP_ETH_INTERFACE_FREERTOS_H */
