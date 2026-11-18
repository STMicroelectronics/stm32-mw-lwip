
  @verbatim
  ******************************************************************************
  * @file    st_readme.txt 
  * @author  GPAM Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          LwIP for integration with STM32Cube solution.
  *          For more details on LwIP implementation on STM32Cube, please refer 
  *          to UM1713 "Developing applications on STM32Cube with LwIP TCP/IP stack"
  ******************************************************************************
  * @attention
  *
  * Portions Copyright (c) 2016-2025 STMicroelectronics, All rights reserved.
  * Copyright (c) 2001-2004 Swedish Institute of Computer Science, All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  @endverbatim

### 21-September-2026 ###
=========================
  + Fix: use recursive mutex for sys_arch_protect().
  + Add SW_Security_Level.md file at root directory.

### 28-August-2023 ###
========================
  + Add LICENSE.md file at the root directory.

### 15-March-2019 ###
========================
  + Upgrade to use LwIP V2.1.2 version
     - For more details about new features and bug fixes please refer to CHANGELOG.txt and UPGRADING files
  + sys_arch.c:
     - Add new API sys_mbox_trypost_fromisr to post preallocated messages from an ISR to the tcpip thread
     - Remove check on Flag LWIP_SOCKET_SET_ERRNO: this flag has been removed since LwIP 2.1.0

### 13-August-2018 ###
========================
  + Add support to CMSIS-RTOS V2 API
     - update the system/OS/sys_arch.c and system/arch/sys_arch.h file with CMSIS-RTOS v2 API

### 10-November-2017 ###
========================
  + Upgrade to use LwIP V2.0.3 version
    - For detailed list of new features and bug fixes please refer to CHANGELOG.txt
  + Updates done LwIP core
    - httpd.c: add include "lwip/sys.h"
    - lowpan6.c: fix MDK-ARM compilation errors.
    - fix variable "var" was set but never used warnings in many files
  + Updates on ST's port "/system/arch/cc.h" file:
    - define LWIP_TIMEVAL_PRIVATE to 0 add include sys/time.h for GNU C compiler
    - remove LWIP_PLATFORM_DIAG definition, added by lwIP in arch.h
    - redefine LWIP_PLATFORM_ASSERT
     
### 23-December-2016 ###
========================
  + Upgrade to use LwIP V2.0.0 version
    - For detailed list of new features and bug fixes please refer to CHANGELOG.txt
    - Additional modification done on V2.0.0 sources:
      - httpd.c: add include "lwip/sys.h"
      - snmp_netconn.c: add include "string.h"
      - snmp_msg.c: add implementation of  "strnlen()" function
      - fix statement unreachable warnings
      - fix variable "var" was set but never used warnings
      - cpu.h: add preprocessor condition to avoid redefinition of BYTE_ORDER macro (defined by default in the last GCC compiler version)
      - api_lib.c:add LWIP_IPV4 preprocessor condition to avoid compilation error when IPV4 is disabled
      - sys_arch.c: implementation updated to be CMSIS-RTOS compliant
      - snmp_msg.c: add implementation of  "strnlen()" function
      - cc.h: target debug macro to printf() 


### 22-April-2016 ###
=====================
  + Use updated version of LwIP v1.4.1 by integrating latest sources from LwIP Git Repository: 
    - Integrate latest commit dated of 2016-02-11, reference http://git.savannah.gnu.org/cgit/lwip.git/commit/?id=cddd3b552a52027a503a00982ceaaec9a6959819
    - Main new features:
      - IPv6 support.
      - Dual IPv4/IPv6 stack.
      - Optional use of IPv4. 
    - Many Bugs fixing an enhnacement, for more detailed please refer to CHANGELOG file
    - Updated architecture having impact on application based on previous version:
      - dhcp.c file is now available under \src\core\ipv4 directory
      - inet_chcksum.c is now available under \src\core directory
      - ip.c and ip_addr.c are renamed to ip4.c and ip4_addr.c respectively.
      - SNMP, SNTP and HTTP protocols implementation is available under \src\include\lwip\apps directory

 
### 19-June-2014 ###
====================
  + sys_arch.c file: fix implementation of sys_mutex_lock() function, by passing "mutex"
   instead of "*mutex" to sys_arch_sem_wait() function.  


### 18-February-2014 ###
========================
   + First customized version for STM32Cube solution.


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
