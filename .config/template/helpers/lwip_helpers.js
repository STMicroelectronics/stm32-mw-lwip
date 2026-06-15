/**
  * @file Helpers functions used for LwIP SW component
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
**/

/**
 * Normalize interface configuration into an iterable list.
 * Supports legacy array format and object-based formats.
 * @param {any} interfaceConfig interface context value
 * @returns {Array}
 */
function normalizeInterfaceList(interfaceConfig) {
  if (Array.isArray(interfaceConfig)) {
    return interfaceConfig;
  }

  if (interfaceConfig && typeof interfaceConfig === "object") {
    if (Array.isArray(interfaceConfig.interfaces)) {
      return interfaceConfig.interfaces;
    }

    if (Array.isArray(interfaceConfig["interface list"])) {
      return interfaceConfig["interface list"];
    }
  }

  return [];
}

/**
 * Return true if at least one configured interface is Ethernet.
 * @param {any} interfaceConfig interface context value
 * @returns {boolean}
 */
function helper_lwip_has_ethernet_interface(interfaceConfig) {
  try {
    var interfaces = normalizeInterfaceList(interfaceConfig);
    return interfaces.some(function (itf) {
      return itf && itf.logical_config && itf.logical_config.interface_type === "ETH";
    });
  } catch (e) {
    console.error("[ERROR] helper_lwip_has_ethernet_interface: " + e);
    return false;
  }
}
/**
 * Return true if at least one configured interface is custom.
 * @param {any} interfaceConfig interface context value
 * @returns {boolean}
 */
function helper_lwip_has_custom_interface(interfaceConfig) {
  try {
    var interfaces = normalizeInterfaceList(interfaceConfig);
    return interfaces.some(function (itf) {
      return itf && itf.logical_config && itf.logical_config.interface_type === "CUSTOM";
    });
  } catch (e) {
    console.error("[ERROR] helper_lwip_has_custom_interface: " + e);
    return false;
  }
}

/**
 * Return true if the selected Ethernet media interface supports 1000 Mbps.
 * @param {object} peripheralsResourceManagerAPI PeripheralsResourceManagerAPI context object
 * @returns {boolean}
 */
function helper_lwip_ethernet_supports_gigabit(peripheralsResourceManagerAPI) {
  try {
    if (!peripheralsResourceManagerAPI || typeof peripheralsResourceManagerAPI.getHardwareIpDescription !== "function") {
      return false;
    }
    var peripheralName = "ETH1";
    var ethHardwareDescription = peripheralsResourceManagerAPI.getHardwareIpDescription(peripheralName);

    var ethFeatures = ethHardwareDescription.features;
    return ((ethFeatures?.ETH_RGMII_EN === 1) || (ethFeatures?.ETH_GMII_EN === 1));
  } catch (e) {
    console.error("[ERROR] helper_lwip_ethernet_supports_gigabit: " + e);
    return false;
  }
}

/**
 * Return the number of Ethernet TX channels.
 * @param {object} peripheralsResourceManagerAPI PeripheralsResourceManagerAPI context object
 * @returns {integer}
 */
function helper_lwip_ethernet_tx_channels_nb(peripheralsResourceManagerAPI) {
  try {
    if (!peripheralsResourceManagerAPI || typeof peripheralsResourceManagerAPI.getHardwareIpDescription !== "function") {
      return false;
    }
    var peripheralName = "ETH1";
    var ethHardwareDescription = peripheralsResourceManagerAPI.getHardwareIpDescription(peripheralName);

    var ethFeatures = ethHardwareDescription.features;
    return (ethFeatures.ETH_NUM_TXQ);
  } catch (e) {
    console.error("[ERROR] helper_lwip_ethernet_tx_channels_nb: " + e);
    return false;
  }
}

/**
 * Return the number of Ethernet RX channels.
 * @param {object} peripheralsResourceManagerAPI PeripheralsResourceManagerAPI context object
 * @returns {integer}
 */
function helper_lwip_ethernet_rx_channels_nb(peripheralsResourceManagerAPI) {
  try {
    if (!peripheralsResourceManagerAPI || typeof peripheralsResourceManagerAPI.getHardwareIpDescription !== "function") {
      return false;
    }
    var peripheralName = "ETH1";
    var ethHardwareDescription = peripheralsResourceManagerAPI.getHardwareIpDescription(peripheralName);

    var ethFeatures = ethHardwareDescription.features;
    return (ethFeatures.ETH_NUM_RXQ);
  } catch (e) {
    console.error("[ERROR] helper_lwip_ethernet_rx_channels_nb: " + e);
    return false;
  }
}

module.exports = {
  helper_lwip_has_ethernet_interface,
  helper_lwip_ethernet_supports_gigabit,
  helper_lwip_ethernet_tx_channels_nb,
  helper_lwip_ethernet_rx_channels_nb,
  helper_lwip_has_custom_interface
};

