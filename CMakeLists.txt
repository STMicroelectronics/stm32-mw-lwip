# origin-pack: STMicroelectronics::lwip@2.1.0
# file-format: 1.0.0
project(STMicroelectronics_lwip_2_1_0)
cmake_minimum_required(VERSION 3.20)
add_library(STMicroelectronics_lwip_2_1_0 INTERFACE)

# List of all CMSIS properties that influence conditions for this package
if(NOT DEFINED CMSIS_Dname)
  set(CMSIS_Dname "CMSIS_Dname not set")
  message(DEBUG "CMSIS_Dname is not set to any value")
endif()

if(NOT DEFINED CMSIS_Dvendor)
  set(CMSIS_Dvendor "CMSIS_Dvendor not set")
  message(DEBUG "CMSIS_Dvendor is not set to any value")
endif()


# Device specific defined symbols








# Enable all components in this package
if(CMSIS_ENTIRE_STMicroelectronics_lwip_2_1_0)
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:Core#Cversion:2.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:SNTP#Cversion:2.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:MDNS#Cversion:2.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:LWIPERF#Cversion:2.0.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:Common port#Cvariant:STM32#Cversion:2.1.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:RTOS port#Cvariant:FreeRTOS#Cversion:2.1.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:STM32 ETH Interface FreeRTOS#Cversion:2.1.0")
  list(APPEND CMSIS_COMPONENTS_LIST "Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:STM32 Custom Interface#Cversion:2.1.0")
endif()

# All conditions used by this package

# condition: STMicroelectronics.lwip.2.1.0
# description: 
if((CMSIS_Dname MATCHES "STM32.*" AND CMSIS_Dvendor STREQUAL "STMicroelectronics:13"))
  set(STMicroelectronics.lwip.2.1.0 "1")
  message(DEBUG "CMSIS condition STMicroelectronics.lwip.2.1.0 enabled")
else()
  set(STMicroelectronics.lwip.2.1.0 0)
endif()


# condition: STMicroelectronics.lwip.2.1.0:RTOS condition
# description: RTOS condition
set(STMicroelectronics.lwip.2.1.0_RTOS_condition "$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:RTOS#.*Cgroup:FreeRTOS#.*Csub:Core(#.*|$)>,>>")
message(DEBUG "CMSIS condition STMicroelectronics.lwip.2.1.0_RTOS_condition enabled")


# condition: STMicroelectronics.lwip.2.1.0:lwip condition
# description: lwip condition
set(STMicroelectronics.lwip.2.1.0_lwip_condition "$<NOT:$<STREQUAL:$<FILTER:${CMSIS_COMPONENTS_LIST},INCLUDE,.*Cclass:Network#.*Cgroup:LwIP#.*Csub:Core(#.*|$)>,>>")
message(DEBUG "CMSIS condition STMicroelectronics.lwip.2.1.0_lwip_condition enabled")

# Files and components in this package
if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:Core#Cversion:2.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP Core component
  message(DEBUG "Using component Network_LwIP_Core_2_0_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "-DCMSIS_USE_Network_LwIP_Core_2_0_0=1")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/compat/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/compat/posix/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/compat/posix/arpa/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/compat/posix/net/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/compat/posix/sys/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/compat/stdc/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/lwip")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/lwip/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/lwip/apps/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/lwip/priv/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/lwip/prot/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/netif")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/include/netif/")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/altcp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/altcp_alloc.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/altcp_tcp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/def.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/dns.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/inet_chksum.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/init.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ip.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/acd.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/autoip.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/dhcp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/etharp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/icmp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/igmp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/ip4.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/ip4_addr.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv4/ip4_frag.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/dhcp6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/ethip6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/icmp6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/inet6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/ip6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/ip6_addr.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/ip6_frag.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/mld6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/ipv6/nd6.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/mem.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/memp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/netif.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/pbuf.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/raw.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/stats.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/sys.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/tcp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/tcp_in.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/tcp_out.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/timeouts.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/core/udp.c")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src/netif/ethernet.c")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/include/lwip>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/api_lib.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/api_msg.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/err.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/if_api.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/netbuf.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/netdb.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/netifapi.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/sockets.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/src/api/tcpip.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:SNTP#Cversion:2.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP SNTP component
  message(DEBUG "Using component Network_LwIP_SNTP_2_0_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:-DCMSIS_USE_Network_LwIP_SNTP_2_0_0=1>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/src/apps/sntp/sntp.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:MDNS#Cversion:2.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP MDNS component
  message(DEBUG "Using component Network_LwIP_MDNS_2_0_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:-DCMSIS_USE_Network_LwIP_MDNS_2_0_0=1>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/src/apps/mdns/mdns.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/src/apps/mdns/mdns_domain.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/src/apps/mdns/mdns_out.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:LWIPERF#Cversion:2.0.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP LWIPERF component
  message(DEBUG "Using component Network_LwIP_LWIPERF_2_0_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:-DCMSIS_USE_Network_LwIP_LWIPERF_2_0_0=1>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/src/apps/lwiperf/lwiperf.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:Common port#Cvariant:STM32#Cversion:2.1.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP STM32 port
  message(DEBUG "Using component Network_LwIP_Common_port_STM32_2_1_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "-DCMSIS_USE_Network_LwIP_Common_port_STM32_2_1_0=1")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ports/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ports/common/")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ports/common/arch")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/ports/common/arch/")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:RTOS port#Cvariant:FreeRTOS#Cversion:2.1.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP FreeRTOS port
  message(DEBUG "Using component Network_LwIP_RTOS_port_FreeRTOS_2_1_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:-DCMSIS_USE_Network_LwIP_RTOS_port_FreeRTOS_2_1_0=1>")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/ports/freertos/>")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/ports/freertos/arch/>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/ports/freertos/arch/sys_arch.c>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_RTOS_condition}:${CMAKE_CURRENT_LIST_DIR}/ports/freertos/arch/sys_arch.h>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:STM32 ETH Interface FreeRTOS#Cversion:2.1.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP STM32 ETH Interface FreeRTOS
  message(DEBUG "Using component Network_LwIP_STM32_ETH_Interface_FreeRTOS_2_1_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:-DCMSIS_USE_Network_LwIP_STM32_ETH_Interface_FreeRTOS_2_1_0=1>")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/interfaces/ethernet>")
  target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/interfaces/ethernet/eth_interface_freertos.c>")
endif()

if("Cvendor:STMicroelectronics#Cclass:Network#Cgroup:LwIP#Csub:STM32 Custom Interface#Cversion:2.1.0" IN_LIST CMSIS_COMPONENTS_LIST)  # LwIP STM32 Custom Interface
  message(DEBUG "Using component Network_LwIP_STM32_Custom_Interface_2_1_0")
  target_compile_definitions(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:-DCMSIS_USE_Network_LwIP_STM32_Custom_Interface_2_1_0=1>")
  target_include_directories(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/interfaces/custom>")
  if(EXISTS ${CMSIS_RTE_FOLDER}/Network/custom_interface.c)
    target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMSIS_RTE_FOLDER}/Network/custom_interface.c>")
  else()
    target_sources(STMicroelectronics_lwip_2_1_0 INTERFACE "$<${STMicroelectronics.lwip.2.1.0_lwip_condition}:${CMAKE_CURRENT_LIST_DIR}/interfaces/custom/custom_interface.c>")
  endif()
endif()

