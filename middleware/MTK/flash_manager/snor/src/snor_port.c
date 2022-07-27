/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */


#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
#include <assert.h>
#include "snor_port.h"
#include "hal_nvic.h"
#include "flash_drvflash.h"
#include "syslog.h"

#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#endif

extern NOR_FLASH_DRV_Data FlashDriveData;
extern FS_Driver NORFlashDriver;

int32_t SNOR_MountDevice(int32_t DeviceNumber, int32_t DeviceType, uint32_t Flags)
{
    NOR_FLASH_DISK_Data *D =(NOR_FLASH_DISK_Data *)(&FlashDriveData);
    return NOR_MountDevice_ext(D, 1, 2, 0);
}

int32_t SNOR_ReadSectors(uint32_t Sector, uint32_t Sectors, void * Buffer)
{
    NOR_FLASH_DISK_Data *D = (NOR_FLASH_DISK_Data *)(&FlashDriveData);

    return NOR_ReadSectors_ext(D, Sector, Sectors, Buffer);
}

int32_t SNOR_WriteSectors(uint32_t Sector, uint32_t Sectors, void * Buffer)
{
    NOR_FLASH_DISK_Data *D = (NOR_FLASH_DISK_Data *)(&FlashDriveData);

    return NOR_WriteSectors_ext(D, Sector, Sectors, Buffer);
}	

uint32_t SNOR_GetSectors(void)
{
    //get sectors number
    return FlashDriveData.TotalFATSectors;
}

uint32_t SNOR_GetBlockSize(void)
{
    //erase block size in unit of sector
    return (FlashDriveData.FlashInfo.BlockSize[0])/512;
}

#ifdef FREERTOS_ENABLE
// it will be used to synchronize read/write/program between Fota/raw disk/GC at same time
static SemaphoreHandle_t fdm_reclaim_mutex;
static EventGroupHandle_t nor_egid;

EventGroupHandle_t triggerEvent = NULL;
static SemaphoreHandle_t protectLock = NULL;
static SemaphoreHandle_t informLock = NULL;
static TaskHandle_t fdm_owner_id = NULL;
#ifdef LOGGING
static TaskHandle_t prev_owner_id = NULL;
#endif


bool INT_QueryExceptionStatus(void){
   if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER){
       return 0;
   } else {
       return 1;
   }
}

void FDM_LOCK_Init(void)
{
    if (fdm_reclaim_mutex==NULL) {
        fdm_reclaim_mutex = xSemaphoreCreateMutex();
    }
    if (fdm_reclaim_mutex==NULL) {
        assert(0);
    }    
}

void FDM_LOCK(void)
{
    if((SysInitStatus_Query() == 1) && (INT_QueryExceptionStatus() == 0))
    {
        if (xSemaphoreTake(fdm_reclaim_mutex, portMAX_DELAY) == pdFALSE) {
            assert(0);
        }
    }
}

void FDM_UNLOCK(void)
{
   if((SysInitStatus_Query() == 1) && (INT_QueryExceptionStatus() == 0))
   {
      if (xSemaphoreGive(fdm_reclaim_mutex) == pdFALSE) {
           assert(0);
      }
   } 
}

void FDM_Set_Reclaim_Event(uint32_t event)
{
    xEventGroupSetBits(triggerEvent, event);
    return;
}

/* synchronzation helper function */
void CreateFDMLock(void)
{
   if (protectLock == NULL) {
       protectLock = xSemaphoreCreateMutex();
       if (protectLock == NULL) {
          //LOG_I("FS","Crete protectLock error\r\n");
       }
   }
   if(informLock == NULL){
       informLock = xSemaphoreCreateMutex();
       if (informLock == NULL) {
          //LOG_W("FS","Crete informLock error\r\n");
       }
   }
   if(triggerEvent == NULL) {
       triggerEvent = xEventGroupCreate();
       if (triggerEvent == NULL) {
          //LOG_W("FS","Crete triggerEvent error\r\n");
       }
   }
}

/*******************************************************************//**
 * Get informLock and protectLock, and set fdm_owner_id to the calling task.
 *
 * @par Catagory:
 * NOR FDM 4.0 (Enhanced Single Bank)
 *
 * @remarks
 * This function only takes effect in normal mode (not in system initialization stage and exception handler).
 * Wait forever if informLock or protectLock can't be taken.
 *
 **********************************************************************/
void GetFDMLock(void)
{	
   if (SysInitStatus_Query() == 0 || HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0)
      return;

   xSemaphoreTake(informLock, portMAX_DELAY);
   	
   xSemaphoreTake(protectLock, portMAX_DELAY);

   NORFDM_ASSERT(fdm_owner_id == NULL);
   fdm_owner_id = xTaskGetCurrentTaskHandle();
}

/*******************************************************************//**
 * Give informLock and protectLock.
 *
 * @par Catagory:
 * NOR FDM 4.0 (Enhanced Single Bank)
 *
 * @remarks
 * This function only works in normal mode.
 * ASSERT if fdm_owner_id is not the same as calling task.
 *
 **********************************************************************/
void FreeFDMLock(void)
{
   if (SysInitStatus_Query() == 0 || HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0)
      return;

   NORFDM_ASSERT(fdm_owner_id == xTaskGetCurrentTaskHandle());

   fdm_owner_id = NULL;
   xSemaphoreGive(protectLock);
   
   xSemaphoreGive(informLock);
}

uint32_t FDM_query_reclaim_event(void)
{
    return uxQueueSpacesAvailable(informLock);
}

void *get_ctrl_buffer(uint32_t size)
{
    return pvPortMalloc(size);
}

void free_ctrl_buffer(void *pdata)
{
    vPortFree(pdata);
}


#else
bool INT_QueryExceptionStatus(void){
   return 1; 
}

void FDM_LOCK_Init(void)
{   
}

void FDM_LOCK(void)
{
}

void FDM_UNLOCK(void)
{
}

void FDM_Set_Reclaim_Event(uint32_t event)
{
    //(void*)(event);
}

void CreateFDMLock(void)
{
}

void GetFDMLock(void)
{
}

void FreeFDMLock(void)
{
}

uint32_t FDM_query_reclaim_event(void)
{
    return 0;
}

void *get_ctrl_buffer(uint32_t size)
{
    return NULL;
}

void free_ctrl_buffer(void *pdata){
   //(void *)(pdata);
}

#endif


#endif //MTK_FATFS_ON_SERIAL_NOR_FLASH

