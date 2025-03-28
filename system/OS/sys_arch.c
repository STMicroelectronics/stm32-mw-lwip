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

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"

#if !NO_SYS

#include "cmsis_os2.h"

#if (defined (__CC_ARM) || defined (__ARMCC_VERSION) || defined (__ICCARM__))
int errno;
#endif

#if    (__ARM_ARCH_7A__      == 1U)
/* CPSR mode bitmasks */
#define CPSR_MODE_USER            0x10U
#define CPSR_MODE_SYSTEM          0x1FU

#define IS_IRQ_MODE()             ((__get_mode() != CPSR_MODE_USER) && (__get_mode() != CPSR_MODE_SYSTEM))
#else
#define IS_IRQ_MODE()             (__get_IPSR() != 0U)
#endif

/* Enables multithreading check functions in this port.

   For this to work, in your lwipopts.h file, you need to set this flag to 1,
   define LWIP_ASSERT_CORE_LOCKED() and LWIP_MARK_TCPIP_THREAD() macros,
   point them to functions sys_check_core_locking() and sys_mark_tcpip_thread()
   respectively and add the functions prototypes.

   If you use LWIP_TCPIP_CORE_LOCKING, you also have to define LOCK_TCPIP_CORE()
   and UNLOCK_TCPIP_CORE() macros, point them to functions sys_lock_tcpip_core()
   and sys_unlock_tcpip_core() respectively and add the functions prototypes.
*/
#ifndef LWIP_CHECK_MULTITHREADING
#define LWIP_CHECK_MULTITHREADING           0
#endif /* LWIP_CHECK_MULTITHREADING */

#if LWIP_CHECK_MULTITHREADING
#if LWIP_TCPIP_CORE_LOCKING
/* Flag the core lock held. A counter for recursive locks. */
static u8_t lwip_core_lock_count;
/* Mark the current core lock holder. */
static osThreadId_t lwip_core_lock_holder_thread;
#endif /* LWIP_TCPIP_CORE_LOCKING */
/* Mark the tcpip thread. */
static osThreadId_t lwip_tcpip_thread;
#endif /* LWIP_CHECK_MULTITHREADING */

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  *mbox = osMessageQueueNew(size, sizeof(void *), NULL);

#if SYS_STATS
  ++lwip_stats.sys.mbox.used;
  if(lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used)
  {
    lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
  }
#endif /* SYS_STATS */
  if(*mbox == NULL)
    return ERR_MEM;

  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
  if(osMessageQueueGetCount(*mbox))
  {
    /* Line for breakpoint.  Should never break here! */
#if SYS_STATS
    lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */

  }
  osMessageQueueDelete(*mbox);
#if SYS_STATS
  --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
  while(osMessageQueuePut(*mbox, &data, 0, osWaitForever) != osOK);
}


/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
  err_t result;
  if(osMessageQueuePut(*mbox, &msg, 0, 0) == osOK)
  {
    result = ERR_OK;
  }
  else
  {
    // could not post, queue must be full
    result = ERR_MEM;

#if SYS_STATS
    lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
  }

  return result;
}


/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
  return sys_mbox_trypost(mbox, msg);
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
  osStatus_t status;
  uint32_t starttime = osKernelGetTickCount();

  if(timeout != 0)
  {
    status = osMessageQueueGet(*mbox, msg, 0, timeout);
    if (status == osOK)
    {
      return (osKernelGetTickCount() - starttime);
    }
    else
    {
      return SYS_ARCH_TIMEOUT;
    }
  }
  else
  {
    osMessageQueueGet(*mbox, msg, 0, osWaitForever );
    return (osKernelGetTickCount() - starttime);
  }
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
  if (osMessageQueueGet(*mbox, msg, 0, 0) == osOK)
  {
    return ERR_OK;
  }
  else
  {
    return SYS_MBOX_EMPTY;
  }
}
/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)
{
  if (*mbox == SYS_MBOX_NULL)
    return 0;
  else
    return 1;
}
/*-----------------------------------------------------------------------------------*/
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  *mbox = SYS_MBOX_NULL;
}

/*-----------------------------------------------------------------------------------*/
//  Creates a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  *sem = osSemaphoreNew(UINT16_MAX, count, NULL);

  if(*sem == NULL)
  {
#if SYS_STATS
    ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
    return ERR_MEM;
  }

  if(count == 0)	// Means it can't be taken
  {
    osSemaphoreAcquire(*sem, 0);
  }

#if SYS_STATS
  ++lwip_stats.sys.sem.used;
  if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
    lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
  }
#endif /* SYS_STATS */

  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  uint32_t starttime = osKernelGetTickCount();

  if(timeout != 0)
  {
    if(osSemaphoreAcquire(*sem, timeout) == osOK)
    {
        return (osKernelGetTickCount() - starttime);
    }
    else
    {
      return SYS_ARCH_TIMEOUT;
    }
  }
  else
  {
    while(osSemaphoreAcquire(*sem, osWaitForever) != osOK);
    return (osKernelGetTickCount() - starttime);
  }
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
  osSemaphoreRelease(*sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
  --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */

  osSemaphoreDelete(*sem);
}
/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)
{
  if (*sem == SYS_SEM_NULL)
    return 0;
  else
    return 1;
}

/*-----------------------------------------------------------------------------------*/
void sys_sem_set_invalid(sys_sem_t *sem)
{
  *sem = SYS_SEM_NULL;
}

/*-----------------------------------------------------------------------------------*/
/* Attributes to support recursive Mutex. */
const osMutexAttr_t Mutex_attributes = {
  .attr_bits = osMutexRecursive,
};

osMutexId_t lwip_sys_mutex;

// Initialize sys arch
void sys_init(void)
{
  lwip_sys_mutex = osMutexNew(&Mutex_attributes);
}
/*-----------------------------------------------------------------------------------*/
                                      /* Mutexes*/
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
#if LWIP_COMPAT_MUTEX == 0
/* Create a new mutex*/
err_t sys_mutex_new(sys_mutex_t *mutex) {

  *mutex = osMutexNew(&Mutex_attributes);

  if(*mutex == NULL)
  {
#if SYS_STATS
    ++lwip_stats.sys.mutex.err;
#endif /* SYS_STATS */
    return ERR_MEM;
  }

#if SYS_STATS
  ++lwip_stats.sys.mutex.used;
  if (lwip_stats.sys.mutex.max < lwip_stats.sys.mutex.used) {
    lwip_stats.sys.mutex.max = lwip_stats.sys.mutex.used;
  }
#endif /* SYS_STATS */
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* Deallocate a mutex*/
void sys_mutex_free(sys_mutex_t *mutex)
{
#if SYS_STATS
      --lwip_stats.sys.mutex.used;
#endif /* SYS_STATS */

  osMutexDelete(*mutex);
}
/*-----------------------------------------------------------------------------------*/
/* Lock a mutex*/
void sys_mutex_lock(sys_mutex_t *mutex)
{
  osMutexAcquire(*mutex, osWaitForever);
}

/*-----------------------------------------------------------------------------------*/
/* Unlock a mutex*/
void sys_mutex_unlock(sys_mutex_t *mutex)
{
  osMutexRelease(*mutex);
}
#endif /*LWIP_COMPAT_MUTEX*/
/*-----------------------------------------------------------------------------------*/
// TODO
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread , void *arg, int stacksize, int prio)
{
  const osThreadAttr_t attributes = {
                        .name = name,
                        .stack_size = stacksize,
                        .priority = (osPriority_t)prio,
                      };
  return osThreadNew(thread, arg, &attributes);
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.

  Note: This function is based on FreeRTOS API, because no equivalent CMSIS-RTOS
        API is available
*/
sys_prot_t sys_arch_protect(void)
{
  osMutexAcquire(lwip_sys_mutex, osWaitForever);

  return (sys_prot_t)1;
}


/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.

  Note: This function is based on FreeRTOS API, because no equivalent CMSIS-RTOS
        API is available
*/
void sys_arch_unprotect(sys_prot_t pval)
{
  ( void ) pval;
  osMutexRelease(lwip_sys_mutex);
}

/*-----------------------------------------------------------------------------------*/
#if LWIP_CHECK_MULTITHREADING
#if LWIP_TCPIP_CORE_LOCKING
/* Lock the lwip core, mark the current core lock holder and the lock count. */
void sys_lock_tcpip_core(void)
{
   sys_mutex_lock(&lock_tcpip_core);
   if (lwip_core_lock_count == 0) {
     lwip_core_lock_holder_thread = osThreadGetId();
   }
   lwip_core_lock_count++;
}

/* Unlock the lwip core. */
void sys_unlock_tcpip_core(void)
{
   lwip_core_lock_count--;
   if (lwip_core_lock_count == 0) {
       lwip_core_lock_holder_thread = 0;
   }
   sys_mutex_unlock(&lock_tcpip_core);
}
#endif /* LWIP_TCPIP_CORE_LOCKING */

/* Mark the lwip tcpip thread ID. */
void sys_mark_tcpip_thread(void)
{
  lwip_tcpip_thread = osThreadGetId();
}

/* Check if the function was accessed with core lock or not.
   Note : This function cannot be called from Interrupt Service Routines.
*/
void sys_check_core_locking(void)
{
  osThreadId_t current_thread;

  /* Check if this function is called from Interrupt Service Routines */
  if (IS_IRQ_MODE())
  {
    LWIP_PLATFORM_ASSERT("This function cannot be called from Interrupt Service Routines");
    while(1);
  }

  if (lwip_tcpip_thread != 0)
  {
    current_thread = osThreadGetId();

#if LWIP_TCPIP_CORE_LOCKING
    LWIP_ASSERT("Function called without core lock",
                current_thread == lwip_core_lock_holder_thread && lwip_core_lock_count > 0);
#else /* LWIP_TCPIP_CORE_LOCKING */
    LWIP_ASSERT("Function called from wrong thread", current_thread == lwip_tcpip_thread);
#endif /* LWIP_TCPIP_CORE_LOCKING */
  }
}
#endif /* LWIP_CHECK_MULTITHREADING */

#endif /* !NO_SYS */
