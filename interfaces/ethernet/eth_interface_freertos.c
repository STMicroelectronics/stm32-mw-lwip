/**
  ******************************************************************************
  * @file           : eth_interface_freertos.c
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

/* Includes ------------------------------------------------------------------*/
#include "eth_interface_freertos.h"
#include "lwip/netifapi.h"

#include "stm32_hal.h"
#include "mx_lwip.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "string.h"
/* Private defines -----------------------------------------------------------*/

/**
  * @brief  Get Rx channel index (starting from 0) from HAL ETH Rx channel ID
  */
#define GET_RX_CHANNEL_INDEX_FROM_HAL_ETH_RX_CHANNEL_ID(channel_rx) ((channel_rx / HAL_ETH_RX_CHANNEL_0) - 1)

/**
  * @brief  Get Tx channel index (starting from 0) from HAL ETH Tx channel ID
  */
#define GET_TX_CHANNEL_INDEX_FROM_HAL_ETH_TX_CHANNEL_ID(channel_tx) ((channel_tx / HAL_ETH_TX_CHANNEL_0) - 1)

/**
  * @brief Align allocated descriptor memory according to alignment constraint
  */
#define ALIGN_DESC_MEM(p_mem_start, align) (void*)((uint32_t)(p_mem_start)\
                                                   + (align) - ((uint32_t)(p_mem_start) % (align)))

/* Private typedef -----------------------------------------------------------*/

/**
  * @brief Message types for the data worker thread.
  */
typedef enum
{
  ERR_MSG,           /**< Error message. */
  DATA_HW_EVENT,     /**< Hardware data event (incoming RX). */
  MW_OUTPUT_BUFFER   /**< Middleware output buffer to be transmitted. */
} data_worker_msg_type_t;

/**
  * @brief Message structure for communication with the data worker thread.
  */
typedef struct
{
  data_worker_msg_type_t type;   /**< Type of the message. */
  uint32_t channels_set;         /**< Bitmask of channels associated with the event. */
  struct pbuf *pbuf;             /**< Pointer to the pbuf (for TX or RX). */
  struct netif *netif;           /**< Pointer to the LwIP network interface. */
  hal_eth_handle_t *p_eth;       /**< Pointer to the hardware ETH handle. */
} data_worker_message_t;

/**
  * @brief State structure for PHY (physical layer) management.
  *
  * Maintains the current PHY mode and a list of associated network interfaces.
  */
typedef struct
{
  lwip_eth_interface_hardware_t *p_hardware;     /**< Pointer to the associated hardware abstraction. */
  mx_phy_link_mode_t phy_mode_current;           /**< Current PHY link mode (speed, duplex, status). */
  struct netif *netifs[MX_LWIP_MAX_INTERFACE_NB];/**< Array of pointers to associated LwIP network interfaces. */
} phy_state_t;

/* Private variables ---------------------------------------------------------*/
/**
  * @brief Mutex for protecting access to the network interface context table.
  */
static SemaphoreHandle_t netif_context_table_mutex;

/**
  * @brief Mutex for protecting access to the PHY state table.
  */
static SemaphoreHandle_t phy_state_table_mutex;

/**
  * @brief Table of pointers to registered network interface contexts.
  *
  * Each entry corresponds to a registered LwIP network interface.
  */
static lwip_eth_interface_netif_context_t *netif_context_table[MX_LWIP_MAX_INTERFACE_NB] = {};

/**
  * @brief Table of PHY state structures.
  *
  * Each entry manages the PHY state and associated netifs for a hardware instance.
  */
static phy_state_t phy_state_table[MX_LWIP_MAX_INTERFACE_NB] = {};

/**
  * @brief Table of pointers to the first pbuf for each RX channel.
  *
  * Used to track the start of a received frame for each hardware RX channel.
  */
struct pbuf *first_pbuf_table[MX_LWIP_MAX_INTERFACE_NB] = {0};

/**
  * @brief Table of pointers storing allocated Rx descriptors pointers
  *
  */
void *rx_desc_pointer_table[MX_LWIP_MAX_INTERFACE_NB] = {0};

/**
  * @brief Table of pointers storing allocated Tx descriptors pointers
  *
  */
void *tx_desc_pointer_table[MX_LWIP_MAX_INTERFACE_NB] = {0};

/**
  * @brief Pointer to the array of Ethernet TX buffers.
  *
  * Used for transmitting frames via the hardware.
  */
static hal_eth_buffer_t *eth_tx_buffer = NULL;

/**
  * @brief Handle for the data worker FreeRTOS task.
  */
static TaskHandle_t data_worker_task = NULL;

/**
  * @brief Handle for the link monitor FreeRTOS task.
  */
static TaskHandle_t link_monitor_task = NULL;

/**
  * @brief Queue handle for messages to the data worker thread.
  */
static QueueHandle_t data_worker_queue = NULL;

/**
  * @brief Flag to control the running state of the data worker thread.
  */
static volatile BaseType_t RunDataWorkerThread = pdTRUE;

/**
  * @brief Flag to control the running state of the link monitor thread.
  */
static volatile BaseType_t RunLinkMonitorThread = pdTRUE;


/* Private function prototypes -----------------------------------------------*/
/**
  * @brief Add a network interface context to the internal table.
  * @param[in] p_netif_context Pointer to the network interface context.
  * @return ERR_OK on success, ERR_MEM if already exists or no space.
  */
static err_t netif_context_add_to_table(lwip_eth_interface_netif_context_t *p_netif_context);

/**
  * @brief Remove a network interface context from the internal table.
  * @param[in] p_netif_context Pointer to the network interface context.
  * @return true if successfully removed, false otherwise.
  */
static bool netif_context_remove_from_table(lwip_eth_interface_netif_context_t *p_netif_context);

/**
  * @brief Compare two network interface contexts for equality.
  * @param[in] p1 Pointer to the first network interface context.
  * @param[in] p2 Pointer to the second network interface context.
  * @return true if equal, false otherwise.
  */
static bool netif_context_equals(lwip_eth_interface_netif_context_t *p1, lwip_eth_interface_netif_context_t *p2);

/**
  * @brief Check if the network interface context table is empty.
  * @return true if empty, false otherwise.
  */
static bool netif_context_table_is_empty(void);

/**
  * @brief Add a network interface context to the PHY state table.
  * @param[in] p_netif_context Pointer to the network interface context.
  * @return ERR_OK on success, ERR_ALREADY if already present, ERR_MEM if no space.
  */
static err_t phy_state_add_to_table(lwip_eth_interface_netif_context_t *p_netif_context);

/**
  * @brief Remove a network interface context from the PHY state table.
  * @param[in] p_netif_context Pointer to the network interface context.
  */
static void phy_state_remove_from_table(lwip_eth_interface_netif_context_t *p_netif_context);

/**
  * @brief Add a netif pointer to a PHY state.
  * @param[in] p_phy_state Pointer to the PHY state.
  * @param[in] p_netif Pointer to the LwIP network interface.
  * @return ERR_OK on success, ERR_ALREADY if already present, ERR_MEM if no space.
  */
static err_t netif_add_to_phy_state(phy_state_t *p_phy_state, struct netif *p_netif);

/**
  * @brief Check if a hardware interface is used by any network interface.
  * @param[in] p_netif_context Pointer to the network interface context.
  * @return true if used, false otherwise.
  */
static bool hardware_interface_is_used(lwip_eth_interface_netif_context_t *p_netif_context);

/**
  * @brief Get the netif pointer from the table based on ETH instance, RX channel, and VLAN ID.
  * @param[in] p_eth Pointer to the ETH handle.
  * @param[in] rx_channel RX channel ID.
  * @param[in] rx_vlan_id VLAN ID.
  * @return Pointer to the LwIP network interface, or NULL if not found.
  */
static struct netif *get_netif_from_table(hal_eth_handle_t *p_eth, uint32_t rx_channel, uint32_t rx_vlan_id);

/**
  * @brief Notify the HAL ETH of a link update (speed/duplex).
  * @param[in] p_eth Pointer to the ETH handle.
  * @param[in] p_phy_link_mode Pointer to the PHY link mode.
  */
static void link_update_notify_hal_eth(hal_eth_handle_t *p_eth, mx_phy_link_mode_t *p_phy_link_mode);

/**
  * @brief Thread function for monitoring PHY link status.
  * @param[in] arg Unused.
  */
static void phy_link_monitor_thread(void *arg);

/**
  * @brief Data worker thread for handling data input/output events.
  * @param[in] arg Unused.
  */
static void data_worker_thread(void *arg);


/**
  * @brief Callback for completed Ethernet TX transfer.
  * @param[in] p_eth Pointer to the ETH handle.
  * @param[in] channel Channel ID.
  * @param[in] tx_pkt_addr Address of the transmitted packet.
  * @param[in] tx_pkt_data TX packet data structure.
  * @return HAL_OK on success.
  */
static hal_status_t tx_complete_cb(hal_eth_handle_t *p_eth, uint32_t channel,
                                   void *tx_pkt_addr, hal_eth_tx_cb_pkt_data_t tx_pkt_data);

/**
  * @brief Callback for completed Ethernet RX transfer.
  * @param[in] p_eth Pointer to the ETH handle.
  * @param[in] channel Channel ID.
  * @param[in] rx_pkt_address Address of the received packet.
  * @param[in] rx_pkt_size Size of the received packet.
  * @param[in] rx_pkt_data RX packet data structure.
  * @return HAL_OK on success, HAL_BUSY if input failed.
  */
static hal_status_t rx_complete_cb(hal_eth_handle_t *p_eth, uint32_t channel,
                                   void *rx_pkt_address, uint32_t rx_pkt_size,
                                   hal_eth_rx_cb_pkt_data_t rx_pkt_data);

/**
  * @brief Callback for hardware data events (from ISR).
  * @param[in] p_eth Pointer to the ETH handle.
  * @param[in] channel Channel mask (bitfield of channels with pending events).
  */
static void data_event_cb(hal_eth_handle_t *p_eth, uint32_t channel);

/**
  * @brief Callback to allocate a buffer for Ethernet reception.
  * @param[in] p_eth Pointer to the ETH handle.
  * @param[in] channel Channel ID.
  * @param[in] rx_buf_size Size of the RX buffer.
  * @param[out] p_rx_buffer Pointer to the allocated buffer.
  * @param[out] p_app_context Pointer to the application context (pbuf).
  */
static void rx_allocate_cb(hal_eth_handle_t *p_eth, uint32_t channel,
                           uint32_t rx_buf_size, void **p_rx_buffer,
                           void **p_app_context);
/* Private Functions Definitions ---------------------------------------------*/


static err_t netif_context_add_to_table(lwip_eth_interface_netif_context_t *p_netif_context)
{
  err_t status = ERR_OK;
  xSemaphoreTake(netif_context_table_mutex, portMAX_DELAY);
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    if (netif_context_equals(netif_context_table[i], p_netif_context) == true)
    {
      /* Already existing */
      status = ERR_MEM;
      break;
    }
  }

  if (status == ERR_OK)
  {
    for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
    {
      if (netif_context_table[i] == NULL)
      {
        /* Found empty slot */
        netif_context_table[i] = p_netif_context;
        status = ERR_OK;
        break;
      }
    }
  }
  xSemaphoreGive(netif_context_table_mutex);

  return status;
}

static bool netif_context_remove_from_table(lwip_eth_interface_netif_context_t *p_netif_context)
{
  bool removed = false;
  xSemaphoreTake(netif_context_table_mutex, portMAX_DELAY);
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    if (netif_context_equals(netif_context_table[i], p_netif_context) == true)
    {
      netif_context_table[i] = NULL;
      removed = true;
      break;
    }
  }
  xSemaphoreGive(netif_context_table_mutex);
  return removed;
}

static bool netif_context_table_is_empty(void)
{
  bool is_empty = true;
  xSemaphoreTake(netif_context_table_mutex, portMAX_DELAY);
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    if (netif_context_table[i] != NULL)
    {
      /* There is still an interface registered */
      is_empty = false;
      break;
    }
  }
  xSemaphoreGive(netif_context_table_mutex);
  return is_empty;
}

static err_t netif_add_to_phy_state(phy_state_t *p_phy_state, struct netif *p_netif)
{
  err_t status = ERR_OK;
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB; i++)
  {
    if (p_phy_state->netifs[i] == p_netif)
    {
      status = ERR_ALREADY;
      break;
    }
  }
  if (status == ERR_OK)
  {
    status = ERR_MEM;
    for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB; i++)
    {
      if (p_phy_state->netifs[i] == NULL)
      {
        p_phy_state->netifs[i] = p_netif;
        status = ERR_OK;
        break;
      }
    }
  }
  return status;
}

static err_t phy_state_add_to_table(lwip_eth_interface_netif_context_t *p_netif_context)
{
  err_t status = ERR_OK;
  xSemaphoreTake(phy_state_table_mutex, portMAX_DELAY);
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    if (phy_state_table[i].p_hardware != NULL)
    {
      if (phy_state_table[i].p_hardware->p_eth == p_netif_context->p_hardware->p_eth)
      {
        /* Already existing, add netif pointer to list to be notified */
        status = netif_add_to_phy_state(&phy_state_table[i], p_netif_context->p_netif);
        if (status == ERR_OK)
        {
          status = ERR_ALREADY;
        }
        else
        {
          status = ERR_MEM;
        }
        break;
      }
    }
  }

  if (status == ERR_OK)
  {
    status = ERR_MEM;
    for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
    {
      if (phy_state_table[i].p_hardware == NULL)
      {
        /* Found empty slot */
        phy_state_table[i].p_hardware = p_netif_context->p_hardware;
        MX_PHY_LINK_MODE_CLEAR(&(phy_state_table[i].phy_mode_current));
        status = netif_add_to_phy_state(&phy_state_table[i], p_netif_context->p_netif);
        if (status == ERR_ALREADY)
        {
          status = ERR_MEM;
        }
        break;
      }
    }
  }

  xSemaphoreGive(phy_state_table_mutex);
  return status;

}

static void phy_state_remove_from_table(lwip_eth_interface_netif_context_t *p_netif_context)
{
  err_t status = ERR_OK;
  xSemaphoreTake(phy_state_table_mutex, portMAX_DELAY);
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    if (phy_state_table[i].p_hardware == NULL)
    {
      continue;
    }
    if (phy_state_table[i].p_hardware->p_eth == p_netif_context->p_hardware->p_eth)
    {
      for (int j = 0 ; j < MX_LWIP_MAX_INTERFACE_NB; j++)
      {
        if (phy_state_table[i].netifs[j] == p_netif_context->p_netif)
        {
          phy_state_table[i].netifs[j] = NULL;
        }
        if (phy_state_table[i].netifs[j] != NULL)
        {
          status = ERR_ALREADY;
        }
      }
      if (status == ERR_OK)
      {
        memset(&phy_state_table[i], 0, sizeof(phy_state_t));
      }
      break;
    }
  }
  xSemaphoreGive(phy_state_table_mutex);
}

static bool hardware_interface_is_used(lwip_eth_interface_netif_context_t *p_netif_context)
{
  xSemaphoreTake(netif_context_table_mutex, portMAX_DELAY);
  bool hardware_interface_found = false;
  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    lwip_eth_interface_netif_context_t *p_i = netif_context_table[i];

    if (p_i == NULL)
    {
      continue;
    }

    if ((p_netif_context->p_hardware->p_eth == p_i->p_hardware->p_eth)
        && (p_netif_context->rx_channel_id == p_i->rx_channel_id)
        && (p_netif_context->tx_channel_id == p_i->tx_channel_id))
    {
      hardware_interface_found = true;
      break;
    }
  }
  xSemaphoreGive(netif_context_table_mutex);
  return hardware_interface_found;
}

/**
  * @brief  obtain netif pointer from ETH instance, RX channel id, vlan ID.
  * @param  heth: ETH handle
  * @retval struct netif *
  */
static struct netif *get_netif_from_table(hal_eth_handle_t *p_eth, uint32_t rx_channel, uint32_t rx_vlan_id)
{
  struct netif *p_netif = NULL;
  xSemaphoreTake(netif_context_table_mutex, portMAX_DELAY);

  for (int i = 0 ; i < MX_LWIP_MAX_INTERFACE_NB ; i++)
  {
    if (netif_context_table[i] != NULL)
    {
      if ((netif_context_table[i]->p_hardware->p_eth == p_eth) && (netif_context_table[i]->rx_channel_id == rx_channel)
          && (netif_context_table[i]->vlan_id == rx_vlan_id))
      {
        p_netif = netif_context_table[i]->p_netif;
        break;
      }
    }
  }
  xSemaphoreGive(netif_context_table_mutex);
  return p_netif;
}

static bool netif_context_equals(lwip_eth_interface_netif_context_t *p1, lwip_eth_interface_netif_context_t *p2)
{
  if ((p1 == NULL) || (p2 == NULL))
  {
    return false;
  }

  if ((p1->vlan_id == p2->vlan_id)
      && (p1->p_hardware->p_eth == p2->p_hardware->p_eth)
      && (p1->rx_channel_id == p2->rx_channel_id)
      && (p1->tx_channel_id == p2->tx_channel_id))
  {
    return true;
  }
  return false;
}

static void link_update_notify_hal_eth(hal_eth_handle_t *p_eth, mx_phy_link_mode_t *p_phy_link_mode)
{
  hal_eth_link_config_t eth_link_config;

  switch (p_phy_link_mode->speed)
  {
    case MX_PHY_LINK_SPEED_10:
      eth_link_config.speed = HAL_ETH_MAC_SPEED_10M;
      break;
    case MX_PHY_LINK_SPEED_100:
      eth_link_config.speed = HAL_ETH_MAC_SPEED_100M;
      break;
#if defined(MX_LWIP_ETH_GIGABIT_SUPPORT) && (MX_LWIP_ETH_GIGABIT_SUPPORT == 1)
    case MX_PHY_LINK_SPEED_1000:
      eth_link_config.speed = HAL_ETH_MAC_SPEED_1000M;
      break;
#endif /* if defined(MX_LWIP_ETH_GIGABIT_SUPPORT) */
    default:
      /* MAC speed not changed on purpose */
      break;
  }
  switch (p_phy_link_mode->duplex)
  {
    case MX_PHY_LINK_HALF_DUPLEX:
      eth_link_config.duplex_mode = HAL_ETH_MAC_HALF_DUPLEX_MODE;
      break;
    case MX_PHY_LINK_FULL_DUPLEX:
      eth_link_config.duplex_mode = HAL_ETH_MAC_FULL_DUPLEX_MODE;
      break;
    default:
      /* MAC duplex mode not changed on purpose */
      break;
  }
  HAL_ETH_UpdateConfigLink(p_eth, &eth_link_config);
}

void phy_link_monitor_thread(void *arg)
{
  RunLinkMonitorThread = pdTRUE;

  while (RunLinkMonitorThread == pdTRUE)
  {
    xSemaphoreTake(phy_state_table_mutex, portMAX_DELAY);
    for (int i_phy = 0 ; i_phy < MX_LWIP_MAX_INTERFACE_NB ; i_phy++)
    {
      if (phy_state_table[i_phy].p_hardware != NULL)
      {
        lwip_eth_interface_hardware_t *p_hardware = phy_state_table[i_phy].p_hardware;

        mx_phy_link_mode_t new_phy_link_mode;
        mx_phy_link_mode_t *current_phy_link_mode = &(phy_state_table[i_phy].phy_mode_current);

        p_hardware->p_phy->get_link_mode(&new_phy_link_mode);

        if ((new_phy_link_mode.status != current_phy_link_mode->status)
            || (new_phy_link_mode.speed != current_phy_link_mode->speed)
            || (new_phy_link_mode.duplex != current_phy_link_mode->duplex))
        {
          if ((new_phy_link_mode.status == MX_PHY_LINK_DOWN)
              && (new_phy_link_mode.status != current_phy_link_mode->status))
          {
            for (int i_netif = 0 ; i_netif < MX_LWIP_MAX_INTERFACE_NB ; i_netif++)
            {
              if (phy_state_table[i_phy].netifs[i_netif] != NULL)
              {
                netifapi_netif_set_link_down(phy_state_table[i_phy].netifs[i_netif]);
              }
            }
          }
          else if (new_phy_link_mode.status == MX_PHY_LINK_UP)
          {
            /* Notify HAL ETH that link mode has changed */
            link_update_notify_hal_eth(p_hardware->p_eth, &new_phy_link_mode);
            for (int i_netif = 0 ; i_netif < MX_LWIP_MAX_INTERFACE_NB ; i_netif++)
            {
              if (phy_state_table[i_phy].netifs[i_netif] != NULL)
              {
                netifapi_netif_set_link_up(phy_state_table[i_phy].netifs[i_netif]);
              }
            }
          }
        }
        /* Store new phy link mode */
        *current_phy_link_mode = new_phy_link_mode;
      }
    }
    xSemaphoreGive(phy_state_table_mutex);

    /* Wait for notification from ISR or timeout for periodic polling */
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(MX_LWIP_LINK_MONITOR_PERIOD_MS));
  }

  vTaskDelete(NULL);
}

void data_worker_thread(void *arg)
{
  uint32_t buffer_count;
  data_worker_message_t message;
  struct pbuf *pbuf = NULL;
  hal_status_t hal_status = HAL_OK;
  uint32_t output_channel_mask;
  hal_eth_tx_pkt_config_t tx_pkt_conf = {0};
  lwip_eth_interface_netif_context_t *p_netif_context;

  tx_pkt_conf.attributes = HAL_ETH_TX_PKT_CTRL_CSUM | HAL_ETH_TX_PKT_CTRL_CRCPAD;
  tx_pkt_conf.crc_pad_ctrl = HAL_ETH_TX_PKT_CRC_PAD_INSERT;
  tx_pkt_conf.csum_ctrl = HAL_ETH_TX_PKT_CSUM_PAYLOAD_HEADER_INSERT;

  RunDataWorkerThread = pdTRUE;

  while (RunDataWorkerThread == pdTRUE)
  {
    ulTaskNotifyTake(true, portMAX_DELAY);
    while (xQueueReceive(data_worker_queue, &message, (TickType_t) 0) == true)
    {
      if (message.type == DATA_HW_EVENT)
      {
        HAL_ETH_ExecDataHandler(message.p_eth, message.channels_set, &output_channel_mask);
      }
      else if (message.type == MW_OUTPUT_BUFFER)
      {
        p_netif_context = (lwip_eth_interface_netif_context_t *)message.netif->state;

        /* foreach pbuf in the linked list */
        for (pbuf = message.pbuf, buffer_count = 0;
             (pbuf != NULL) && (buffer_count < MX_LWIP_MAX_ETH_TX_BUFFERS);
             buffer_count++, pbuf = pbuf->next)
        {
          eth_tx_buffer[buffer_count].p_buffer = pbuf->payload;
          eth_tx_buffer[buffer_count].len_byte = pbuf->len;
        }
        /* record the first pbuf in tx_pkt_conf.p_data */
        /* it will be used to free the entire pbuf chain when last eth_tx_buffer transfer is complete */
        /* NOTE: this must be updated if the TX buffers notification strategy is changed */
        tx_pkt_conf.p_data = (void *)message.pbuf;
        if (p_netif_context->vlan_id != 0)
        {
          tx_pkt_conf.attributes |= HAL_ETH_TX_PKT_CTRL_VLANTAG;
          tx_pkt_conf.vlan_tag_id = p_netif_context->vlan_id;
          tx_pkt_conf.vlan_ctrl = HAL_ETH_TX_PKT_VLAN_INSERT;
        }

        do
        {
          hal_status = HAL_ETH_RequestTx(p_netif_context->p_hardware->p_eth,
            p_netif_context->tx_channel_id, eth_tx_buffer, buffer_count, &tx_pkt_conf);

          if (hal_status == HAL_BUSY)
          {
            HAL_ETH_ExecDataHandler(p_netif_context->p_hardware->p_eth, p_netif_context->tx_channel_id, &output_channel_mask);
          }
        } while (hal_status == HAL_BUSY);

      }
      else
      {
        /* other message type ? */
        LWIP_ASSERT("data_worker_thread message.type unknown\n", 0);
      }
    }
  }
  vTaskDelete(NULL);
}

void rx_allocate_cb(hal_eth_handle_t *p_eth, uint32_t channel,
                    uint32_t rx_buf_size, void **p_rx_buffer,
                    void **p_app_context)
{
  /* need to check the channel, app_context */
  /* there can be cases where different buffer pools are used depending from channel / netif */

  struct pbuf *p = pbuf_alloc(PBUF_RAW, rx_buf_size, PBUF_POOL);
  if (p)
  {
    /* Get a pointer to payload. */
    *p_rx_buffer = p->payload;
    *p_app_context = p;
  }
  else
  {
    *p_rx_buffer = NULL;
    LWIP_ASSERT("pbuf_alloc(PBUF_POOL) != NULL", (p != NULL));
  }
}

/**
  * @brief  Ethernet Tx Transfer completed callback.
  *         Called from HAL_ETH_ExecHandler() when an ethernet frame
  *         or part of frame is sent.
  *         An ethernet frame can be sent in several buffers.
  *         The last part has a specific status flag that indicates the full
  *         transfer is complete.
  * @param  heth: ETH handle
  * @retval None
  */
hal_status_t tx_complete_cb(hal_eth_handle_t *p_eth, uint32_t channel,
                            void *tx_pkt_addr, hal_eth_tx_cb_pkt_data_t tx_pkt_data)
{
  /* if it is the last buffer of full frame, free all the linked list of pbufs */
  /* pbuf_free(pbuf); */ /* one call to pbuf_free() on first pbuf frees the entire chain */

  /* handle the channel */

  /* handle the buffer */
  struct pbuf *pbuf = NULL;
  if (tx_pkt_data.errors == 0) /* to be checked */
  {
    if ((tx_pkt_data.status & HAL_ETH_TX_STATUS_LD) == HAL_ETH_TX_STATUS_LD)
    {
      /* if it is the last buffer of full frame, free all the linked list of pbufs */
      if (tx_pkt_data.p_data != NULL)
      {
        /* the first LwIP pbuf in the chain was recorded in tx_pkt_data.p_data. */
        pbuf = (struct pbuf *)tx_pkt_data.p_data;
        /* free the entire pbuf chain */
        pbuf_free(pbuf);
      }
      else
      {
        LWIP_ASSERT("tx_pkt_data.status == HAL_ETH_TX_STATUS_LD and tx_pkt_data.p_data != NULL",
                    (tx_pkt_data.p_data != NULL));
      }
    }
  }
  else
  {
    LWIP_ASSERT("tx_pkt_data.errors == 0", (tx_pkt_data.errors == 0));
  }
  return HAL_OK;
}

void data_event_cb(hal_eth_handle_t *p_eth, uint32_t channel)
{
  BaseType_t xHigherPriorityTaskWoken1 = false;
  BaseType_t xHigherPriorityTaskWoken2 = false;
  data_worker_message_t message;
  message.type = DATA_HW_EVENT;
  message.channels_set = channel;
  message.pbuf = NULL;
  message.netif = NULL;
  message.p_eth = p_eth;

  /* just send a message to the data worker thread that will do the actual processing */
  xQueueSendFromISR(data_worker_queue, &message, &xHigherPriorityTaskWoken1);
  vTaskNotifyGiveFromISR(data_worker_task, &xHigherPriorityTaskWoken2);
  portYIELD_FROM_ISR((xHigherPriorityTaskWoken1) || (xHigherPriorityTaskWoken2));
}

/**
  * @brief  Ethernet Rx Transfer completed callback.
  *         Called from HAL_ETH_ExecHandler() when some data was received on a channel.
  * @param  heth: ETH handle
  * @retval None
  */
hal_status_t rx_complete_cb(hal_eth_handle_t *p_eth, uint32_t channel,
                            void *rx_pkt_address, uint32_t rx_pkt_size,
                            hal_eth_rx_cb_pkt_data_t rx_pkt_data)
{
  struct pbuf *pbuf = (struct pbuf *)rx_pkt_data.p_data;
  const uint32_t channel_idx = GET_RX_CHANNEL_INDEX_FROM_HAL_ETH_RX_CHANNEL_ID(channel);
  struct pbuf *first_pbuf = NULL;
  struct netif *p_netif = NULL;
  hal_status_t hal_status = HAL_OK;
  err_t lwip_err = ERR_OK;

  if (rx_pkt_data.errors)
  {
    pbuf_free(pbuf);
  }
  else
  {
    if (channel_idx >= MX_LWIP_MAX_INTERFACE_NB)
    {
      LWIP_ASSERT("rx_complete_cb channel_idx < MX_LWIP_MAX_INTERFACE_NB", 0);
      return HAL_ERROR;
    }
    first_pbuf = first_pbuf_table[channel_idx];

    pbuf->len = rx_pkt_size;
    pbuf->tot_len = rx_pkt_size;

    if (rx_pkt_data.status & HAL_ETH_RX_STATUS_FD)
    {
      if (first_pbuf != NULL)
      {
        /* First pbuf has not been received because of abort Rx frame, free it */
        pbuf_free(first_pbuf);
      }
      first_pbuf = pbuf;
      /* Store first pbuf entry in table */
      first_pbuf_table[channel_idx] = first_pbuf;
    }
    else
    {
      /* Chain pbuf is not first chunk of received frame */
      pbuf_chain(first_pbuf, pbuf);
    }


    /* provide the received frame to IP stack if last chunk */
    if (rx_pkt_data.status & HAL_ETH_RX_STATUS_LD)
    {

      /* find netif from ETH IP instance, channel, VLAN */
      p_netif = get_netif_from_table(p_eth, channel, rx_pkt_data.vlan_tag_ids);
      if (p_netif != NULL)
      {
        lwip_err = p_netif->input(first_pbuf, p_netif);
        LWIP_ASSERT("rx_complete_cb() p_netif->input() lwip_err == ERR_OK", lwip_err == ERR_OK);
      }
      else
      {
        /* no netif found, free the pbuf chain */
        pbuf_free(first_pbuf);
        lwip_err = ERR_OK;
      }

      if (lwip_err != ERR_OK)
      {
        hal_status = HAL_BUSY;
      }
      else
      {
        /* Remove first pbuf entry in table */
        first_pbuf_table[channel_idx] = NULL;
      }
    }
  }

  return hal_status;
}

/* Public Functions Definitions ----------------------------------------------*/

err_t lwip_eth_interface_low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q = NULL;
  BaseType_t rtos_val = pdPASS;
  data_worker_message_t message;

  LWIP_ASSERT("pbuf_clen < MX_LWIP_MAX_ETH_TX_BUFFERS", pbuf_clen(p) < MX_LWIP_MAX_ETH_TX_BUFFERS);

  /* Increment the reference counter of the pbufs
     to avoid them being re-used immediately by LwIP.
     They will be freed with pbuf_free() in last TX complete callback */
  for (q = p; q != NULL; q = q->next)
  {
    pbuf_ref(q);
  }

  message.type = MW_OUTPUT_BUFFER;
  message.netif = netif;
  message.pbuf = p;

  rtos_val = xQueueSend(data_worker_queue, &message, pdMS_TO_TICKS(MX_LWIP_DATA_WORKER_QUEUE_TX_TIMEOUT_MS));

  LWIP_ASSERT("xQueueSend() == pdPASS", rtos_val == pdPASS);
  if (rtos_val == pdPASS)
  {
    xTaskNotifyGive(data_worker_task);
    return ERR_OK;
  }
  else
  {
    return ERR_MEM;
  }
}

err_t lwip_eth_interface_low_level_init(struct netif *p_netif)
{
  err_t err = ERR_OK;
  hal_status_t hal_status;
  lwip_eth_interface_netif_context_t *p_netif_context = NULL;
  hal_eth_channel_state_t channel_rx_state;
  hal_eth_channel_state_t channel_tx_state;
  bool hw_interface_is_used;
  uint32_t rx_channel_idx;
  uint32_t tx_channel_idx;
  void *rx_channel_desc_memory;
  void *tx_channel_desc_memory;
  uint32_t align;

  if (p_netif == NULL || (p_netif->state == NULL))
  {
    return ERR_VAL;
  }

  p_netif_context = (lwip_eth_interface_netif_context_t *)p_netif->state;

  if ((p_netif_context->p_hardware == NULL) || (p_netif_context->p_hardware->p_eth == NULL)
      || (p_netif_context->p_hardware->p_phy == NULL))
  {
    return ERR_VAL;
  }

  rx_channel_idx = GET_RX_CHANNEL_INDEX_FROM_HAL_ETH_RX_CHANNEL_ID(p_netif_context->rx_channel_id);
  if (rx_channel_idx >= MX_LWIP_MAX_INTERFACE_NB)
  {
    LWIP_ASSERT("lwip_eth_interface_low_level_init rx_channel_idx < MX_LWIP_MAX_INTERFACE_NB", 0);
    return ERR_VAL;
  }

  tx_channel_idx = GET_TX_CHANNEL_INDEX_FROM_HAL_ETH_TX_CHANNEL_ID(p_netif_context->tx_channel_id);
  if (tx_channel_idx >= MX_LWIP_MAX_INTERFACE_NB)
  {
    LWIP_ASSERT("lwip_eth_interface_low_level_init tx_channel_idx < MX_LWIP_MAX_INTERFACE_NB", 0);
    return ERR_VAL;
  }

  /* Start Rx Channel */
  channel_rx_state = HAL_ETH_GetChannelState(p_netif_context->p_hardware->p_eth, p_netif_context->rx_channel_id);
  if ((channel_rx_state != HAL_ETH_CHANNEL_STATE_ACTIVE) && (channel_rx_state != HAL_ETH_CHANNEL_STATE_SUSPENDED))
  {
    /* register allocate callback called by HAL_ETH_ExecDataHandler() */
    hal_status = HAL_ETH_RegisterChannelRxAllocateCallback(p_netif_context->p_hardware->p_eth,
                                                           p_netif_context->rx_channel_id, rx_allocate_cb);

    HAL_ETH_RegisterChannelRxCptCallback(p_netif_context->p_hardware->p_eth, p_netif_context->rx_channel_id,
                                         rx_complete_cb);

    hal_eth_channel_alloc_needs_t rx_eth_alloc_req = {0};
    HAL_ETH_GetChannelAllocNeeds(p_netif_context->p_hardware->p_eth, p_netif_context->rx_channel_id, &rx_eth_alloc_req);

    align = rx_eth_alloc_req.mem_addr_align_byte;
    rx_desc_pointer_table[rx_channel_idx] = pvPortMalloc(rx_eth_alloc_req.mem_size_byte + align);
    if (rx_desc_pointer_table[rx_channel_idx] == NULL)
    {
      LWIP_ASSERT("alloc channel memory RX", 0);
      err = ERR_MEM;
    }
    else
    {
      rx_channel_desc_memory = ALIGN_DESC_MEM(rx_desc_pointer_table[rx_channel_idx], align);
      LWIP_ASSERT("Align desc Rx", (uint32_t)rx_channel_desc_memory % rx_eth_alloc_req.mem_addr_align_byte == 0);

      hal_status = HAL_ETH_StartChannel(p_netif_context->p_hardware->p_eth, p_netif_context->rx_channel_id,
                                        rx_channel_desc_memory, rx_eth_alloc_req.mem_size_byte);
      if (hal_status != HAL_OK)
      {
        vPortFree(rx_desc_pointer_table[rx_channel_idx]);
        LWIP_ASSERT("HAL_ETH_StartChannel RX hal_status == HAL_OK", 0);
        err = ERR_IF;
      }
    }
  }

  /* Start Tx Channel */
  if (err == ERR_OK)
  {
    channel_tx_state = HAL_ETH_GetChannelState(p_netif_context->p_hardware->p_eth, p_netif_context->tx_channel_id);
    if ((channel_tx_state != HAL_ETH_CHANNEL_STATE_ACTIVE) && (channel_tx_state != HAL_ETH_CHANNEL_STATE_SUSPENDED))
    {
      HAL_ETH_RegisterChannelTxCptCallback(p_netif_context->p_hardware->p_eth, p_netif_context->tx_channel_id,
                                           tx_complete_cb);

      hal_eth_channel_alloc_needs_t tx_eth_alloc_req = {0};
      HAL_ETH_GetChannelAllocNeeds(p_netif_context->p_hardware->p_eth, p_netif_context->tx_channel_id, &tx_eth_alloc_req);

      align = tx_eth_alloc_req.mem_addr_align_byte;
      tx_desc_pointer_table[tx_channel_idx] = pvPortMalloc(tx_eth_alloc_req.mem_size_byte + align);
      if (tx_desc_pointer_table[tx_channel_idx] == NULL)
      {
        LWIP_ASSERT("alloc channel memory TX", 0);
        err = ERR_MEM;
      }
      else
      {
        tx_channel_desc_memory = ALIGN_DESC_MEM(tx_desc_pointer_table[tx_channel_idx], align);
        LWIP_ASSERT("Align desc Tx", (uint32_t)tx_channel_desc_memory % align == 0);

        hal_status = HAL_ETH_StartChannel(p_netif_context->p_hardware->p_eth, p_netif_context->tx_channel_id,
                                          tx_channel_desc_memory, tx_eth_alloc_req.mem_size_byte);
        if (hal_status != HAL_OK)
        {
          vPortFree(tx_desc_pointer_table[tx_channel_idx]);
          LWIP_ASSERT("HAL_ETH_StartChannel TX hal_status == HAL_OK", 0);
          err = ERR_IF;
        }
      }
    }
  }

  if (err == ERR_OK)
  {
    /* Check if hardware interface is used by another network interface and register data callback if not */
    hw_interface_is_used = hardware_interface_is_used(p_netif_context);
    if (hw_interface_is_used == false)
    {
      hal_status = HAL_ETH_RegisterDataCallback(p_netif_context->p_hardware->p_eth, data_event_cb);
      if (hal_status != HAL_OK)
      {
        LWIP_ASSERT("HAL_ETH_RegisterDataCallback hal_status == HAL_OK", 0);
        err = ERR_IF;
      }
    }
  }

  /* Fill netif context table */
  if (err == ERR_OK)
  {
    err = netif_context_add_to_table(p_netif_context);
  }

  /* Fill phy state table */
  if (err == ERR_OK)
  {
    err = phy_state_add_to_table(p_netif_context);
    if (err == ERR_ALREADY)
    {
      err = ERR_OK;
    }
  }


  if (err == ERR_OK)
  {
    /* set MAC hardware address in LwIP netif */
    p_netif->hwaddr_len = ETH_HWADDR_LEN;
    hal_eth_config_t hal_eth_config;
    HAL_ETH_GetConfig(p_netif_context->p_hardware->p_eth, &hal_eth_config);
    memcpy(p_netif->hwaddr, &hal_eth_config.mac_addr[0], sizeof(hal_eth_config.mac_addr));
  }


  if (err != ERR_OK)
  {
    lwip_eth_interface_low_level_deinit(p_netif);
  }

  return err;
}


err_t lwip_eth_interface_low_level_deinit(struct netif *p_netif)
{
  hal_status_t hal_status;
  lwip_eth_interface_netif_context_t *p_netif_context = NULL;
  bool hw_interface_is_used;
  bool netif_removed;

  uint32_t rx_channel_idx;
  uint32_t tx_channel_idx;

  if (p_netif == NULL || (p_netif->state == NULL))
  {
    return ERR_VAL;
  }

  p_netif_context = (lwip_eth_interface_netif_context_t *)p_netif->state;

  if ((p_netif_context->p_hardware == NULL) || (p_netif_context->p_hardware->p_eth == NULL)
      || (p_netif_context->p_hardware->p_phy == NULL))
  {
    return ERR_VAL;
  }

  netif_removed = netif_context_remove_from_table(p_netif_context);
  if (netif_removed == true)
  {
    phy_state_remove_from_table(p_netif_context);

    /* Check if hardware interface is used by another network interface */
    hw_interface_is_used = hardware_interface_is_used(p_netif_context);
    if (hw_interface_is_used == false)
    {
      hal_status = HAL_ETH_StopChannel(p_netif_context->p_hardware->p_eth, p_netif_context->rx_channel_id);
      LWIP_ASSERT("HAL_ETH_StopChannel RX hal_status == HAL_OK", hal_status == HAL_OK);

      rx_channel_idx = GET_RX_CHANNEL_INDEX_FROM_HAL_ETH_RX_CHANNEL_ID(p_netif_context->rx_channel_id);
      if (rx_channel_idx < MX_LWIP_MAX_INTERFACE_NB)
      {
        if (rx_desc_pointer_table[rx_channel_idx] != NULL)
        {
          vPortFree(rx_desc_pointer_table[rx_channel_idx]);
          rx_desc_pointer_table[rx_channel_idx] = NULL;
        }
        else
        {
          LWIP_ASSERT("lwip_eth_interface_low_level_deinit rx_desc_pointer_table[rx_channel_idx] == NULL", 0);
        }
      }
      else
      {
        LWIP_ASSERT("lwip_eth_interface_low_level_deinit rx_channel_idx < MX_LWIP_MAX_INTERFACE_NB", 0);
      }

      hal_status = HAL_ETH_StopChannel(p_netif_context->p_hardware->p_eth, p_netif_context->tx_channel_id);
      LWIP_ASSERT("HAL_ETH_StopChannel TX hal_status == HAL_OK", hal_status == HAL_OK);

      tx_channel_idx = GET_TX_CHANNEL_INDEX_FROM_HAL_ETH_TX_CHANNEL_ID(p_netif_context->tx_channel_id);
      if (tx_channel_idx < MX_LWIP_MAX_INTERFACE_NB)
      {
        if (tx_desc_pointer_table[tx_channel_idx] != NULL)
        {
          vPortFree(tx_desc_pointer_table[tx_channel_idx]);
          tx_desc_pointer_table[tx_channel_idx] = NULL;
        }
        else
        {
          LWIP_ASSERT("lwip_eth_interface_low_level_deinit tx_desc_pointer_table[tx_channel_idx] == NULL", 0);
        }
      }
      else
      {
        LWIP_ASSERT("lwip_eth_interface_low_level_deinit tx_channel_idx < MX_LWIP_MAX_INTERFACE_NB", 0);
      }
    }
  }
  return ERR_OK;
}

err_t lwip_eth_interface_init(void)
{
  err_t err = ERR_OK;

  if ((err == ERR_OK) && (eth_tx_buffer == NULL))
  {
    eth_tx_buffer = pvPortMalloc(MX_LWIP_MAX_ETH_TX_BUFFERS * sizeof(hal_eth_buffer_t *));
    if (eth_tx_buffer == NULL)
    {
      LWIP_ASSERT("eth_tx_buffer creation failed", eth_tx_buffer);
      err =  ERR_MEM;
    }
  }
  else
  {
    err =  ERR_MEM;
  }

  if ((err == ERR_OK) && (netif_context_table_mutex == NULL))
  {
    netif_context_table_mutex = xSemaphoreCreateRecursiveMutex();
  }
  else
  {
    err =  ERR_MEM;
  }

  if ((err == ERR_OK) && (phy_state_table_mutex == NULL))
  {
    phy_state_table_mutex = xSemaphoreCreateRecursiveMutex();
  }
  else
  {
    err =  ERR_MEM;
  }


  if ((err == ERR_OK) && (data_worker_queue == NULL))
  {
    data_worker_queue = xQueueCreate(MX_LWIP_DATA_WORKER_QUEUE_SIZE, sizeof(data_worker_message_t));
    if (data_worker_queue == NULL)
    {
      LWIP_ASSERT("Message queue creation failed", data_worker_queue);
      err =  ERR_MEM;
    }
  }
  else
  {
    err =  ERR_MEM;
  }

  if ((err == ERR_OK) && (data_worker_task == NULL))
  {
    if (xTaskCreate(
          data_worker_thread,
          "EthDataWorker",
          MX_LWIP_DATA_WORKER_STACK_SIZE / sizeof(StackType_t), (void *)NULL, MX_LWIP_DATA_WORKER_PRIORITY,
          &data_worker_task) != pdPASS)
    {
      LWIP_ASSERT("data worker task creation failed", false);
      err = ERR_MEM;
    }
  }
  else
  {
    err =  ERR_MEM;
  }

  if ((err == ERR_OK) && (link_monitor_task == NULL))
  {
    if (xTaskCreate(phy_link_monitor_thread,
                    "EthLinkMonitor",
                    MX_LWIP_LINK_MONITOR_STACK_SIZE / sizeof(StackType_t), (void *)NULL, MX_LWIP_LINK_MONITOR_PRIORITY,
                    &link_monitor_task) != pdPASS)
    {
      LWIP_ASSERT("link monitor task creation failed", false);
      err = ERR_MEM;
    }
  }
  else
  {
    err =  ERR_MEM;
  }

  if (err != ERR_OK)
  {
    (void) lwip_eth_interface_deinit();
  }

  return err;
}

err_t lwip_eth_interface_deinit(void)
{
  err_t err = ERR_MEM;

  bool empty = netif_context_table_is_empty();

  if (empty == true)
  {
    err = ERR_OK;

    RunDataWorkerThread = pdFALSE;
    RunLinkMonitorThread = pdFALSE;

    if (data_worker_task != NULL)
    {
      xTaskNotifyGive(data_worker_task);
      data_worker_task = NULL;
    }
    link_monitor_task = NULL;
    if (eth_tx_buffer != NULL)
    {
      vPortFree(eth_tx_buffer);
      eth_tx_buffer = NULL;
    }
    if (data_worker_queue != NULL)
    {
      vQueueDelete(data_worker_queue);
      data_worker_queue = NULL;
    }
    if (netif_context_table_mutex != NULL)
    {
      vSemaphoreDelete(netif_context_table_mutex);
      netif_context_table_mutex = NULL;
    }
    if (phy_state_table_mutex != NULL)
    {
      vSemaphoreDelete(phy_state_table_mutex);
      phy_state_table_mutex = NULL;
    }
  }

  return err;
}


void lwip_eth_interface_link_monitor_event_notify_from_isr(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (link_monitor_task != NULL)
  {
    vTaskNotifyGiveFromISR(link_monitor_task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}
