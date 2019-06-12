/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "kv_api.h"

#include "kv_conf.h"
#include "kv_adapt.h"
     
#include "flexspi_hyper_flash_ops.h"
#include "fsl_cache.h"

#include "FreeRTOS.h"
#include "task.h"
#include "wrappers_defs.h"

#include "fsl_debug_console.h"
     
#define KV_FLASH_BASE               (0x03C00000)    // 60MB - 64MB
#define KV_FLASH_SECTOR_SIZE        (1 << KV_CONFIG_BLOCK_SIZE_BITS)
#define FLASH_PAGE_SIZE_BYTES       512

void *HAL_MutexCreate(void);
void  HAL_MutexDestroy(void *mutex);
void  HAL_MutexLock(void *mutex);
void  HAL_MutexUnlock(void *mutex);
void *HAL_SemaphoreCreate(void);
void  HAL_SemaphoreDestroy(void *sem);
void  HAL_SemaphorePost(void *sem);
int   HAL_SemaphoreWait(void *sem, uint32_t timeout_ms);
void *HAL_Malloc(uint32_t size);
void  HAL_Free(void *ptr);

static TaskHandle_t kv_task_handle;

int32_t kv_flash_read(uint32_t offset, void *buf, uint32_t nbytes)
{
    uint32_t cpu_addr;
    
   // PRINTF("R - kv_flash_read, offset = %d, nbytes = %d\r\n", offset, nbytes);
    
    cpu_addr = FlexSPI_AMBA_BASE + KV_FLASH_BASE + offset;
    memcpy(buf, (void *)cpu_addr, nbytes);
    
    return 0;
}

int32_t kv_flash_write(uint32_t offset, void *buf, uint32_t nbytes)
{
    /* Must be 4-byte aligned. */
    SDK_ALIGN(static uint8_t intBuffer[FLASH_PAGE_SIZE_BYTES], 4);

    status_t status;
    uint32_t phy_address;
    uint32_t page_offset;
    uint32_t phyAddrAlign;
    uint32_t memcpylen;
    uint32_t sizeLeft = nbytes;
    const uint8_t *buffer = (const uint8_t *)buf;
    
   // PRINTF("P - kv_flash_write, offset = %d, nbytes = %d, [0]=%d, [1]=%d, [2]=%d, [3]=%d\r\n", offset, nbytes, buffer[0], buffer[1], buffer[2], buffer[3]);
    
    uint32_t old_primask = DisableGlobalIRQ();
    
    phy_address = KV_FLASH_BASE + offset;
    
   /* Check if the startaddress is the page size aligned */
    if ((phy_address % FLASH_PAGE_SIZE_BYTES) != 0)
    {
        page_offset = phy_address % FLASH_PAGE_SIZE_BYTES;
        phyAddrAlign = (phy_address / FLASH_PAGE_SIZE_BYTES) * FLASH_PAGE_SIZE_BYTES;
        memcpylen = nbytes;

        /* Check if the area across pages. */
        if (page_offset + nbytes > FLASH_PAGE_SIZE_BYTES)
        {
            memcpylen = FLASH_PAGE_SIZE_BYTES - page_offset;
        }

        /* Need to read the page first*/
        memcpy(intBuffer, (void*)(phyAddrAlign + FlexSPI_AMBA_BASE), FLASH_PAGE_SIZE_BYTES);

        /* Change the data required to be changed. */
        memcpy((void*)(((uint32_t)intBuffer) + page_offset), buffer, memcpylen);
        
        status = flexspi_nor_flash_page_program(FLEXSPI, phyAddrAlign, (uint32_t const *)intBuffer);
        if (status != kStatus_Success)
        {
            EnableGlobalIRQ(old_primask);
            return status;
        }
        
        DCACHE_InvalidateByRange(FlexSPI_AMBA_BASE + phyAddrAlign, FLASH_PAGE_SIZE_BYTES);

        /* Update the address and size */
        phy_address = phyAddrAlign + FLASH_PAGE_SIZE_BYTES;
        sizeLeft -= memcpylen;
        buffer += memcpylen;
    }
    
    /* Now the startAddr is page size aligned. */
    while (sizeLeft >= FLASH_PAGE_SIZE_BYTES)
    {
        /* Make sure 4 byte align. */
        if (0 != ((uint32_t)buffer & 0x03))
        {
            memcpy(intBuffer, buffer, FLASH_PAGE_SIZE_BYTES);
            
            status = flexspi_nor_flash_page_program(FLEXSPI, phy_address, (uint32_t const *)intBuffer);
        }
        else
        {
            status = flexspi_nor_flash_page_program(FLEXSPI, phy_address, (uint32_t const *)buffer);
        }

        if (status != kStatus_Success)
        {
            EnableGlobalIRQ(old_primask);
            return status;
        }
        
        DCACHE_InvalidateByRange(FlexSPI_AMBA_BASE + phy_address, FLASH_PAGE_SIZE_BYTES);

        phy_address += FLASH_PAGE_SIZE_BYTES;
        sizeLeft -= FLASH_PAGE_SIZE_BYTES;
        buffer += FLASH_PAGE_SIZE_BYTES;
    }
    
    if (sizeLeft)
    {
        memcpy(intBuffer, buffer, sizeLeft);
        memcpy(intBuffer + sizeLeft, (void*)(phy_address + FlexSPI_AMBA_BASE + sizeLeft), FLASH_PAGE_SIZE_BYTES - sizeLeft);

        status = flexspi_nor_flash_page_program(FLEXSPI, phy_address, (uint32_t const *)intBuffer);
        if (status != kStatus_Success)
        {
            EnableGlobalIRQ(old_primask);
            return status;
        }
        
        DCACHE_InvalidateByRange(FlexSPI_AMBA_BASE + phy_address, FLASH_PAGE_SIZE_BYTES);
    }
    
    return 0;
}

int32_t kv_flash_erase(uint32_t offset, uint32_t size)
{
    /* 
        offset always start at the boundary of sector
        size is always 256KB
    */
    PRINTF("E - kv_flash_erase, offset = %d, size = %d\r\n", offset, size);
    
    uint32_t old_primask = DisableGlobalIRQ();
    
    if (flexspi_nor_flash_erase_sector(FLEXSPI, KV_FLASH_BASE + offset) != kStatus_Success) {
        
        EnableGlobalIRQ(old_primask);
        return -1;
    }
    
    EnableGlobalIRQ(old_primask);
    DCACHE_InvalidateByRange(FlexSPI_AMBA_BASE + KV_FLASH_BASE + offset, KV_FLASH_SECTOR_SIZE);
    return 0;
}

void *kv_lock_create(void)
{
    return HAL_MutexCreate();
}

int32_t kv_lock_free(void *lock)
{
    HAL_MutexDestroy(lock);
    return 0;
}

int32_t kv_lock(void *lock)
{
    HAL_MutexLock(lock);
    return 0;
}

int32_t kv_unlock(void *lock)
{
    HAL_MutexUnlock(lock);
    return 0;
}

void *kv_sem_create(void)
{
    return HAL_SemaphoreCreate();
}

int32_t kv_sem_free(void *sem)
{
    HAL_SemaphoreDestroy(sem);
    return 0;
}

int32_t kv_sem_wait(void *sem)
{
    return HAL_SemaphoreWait(sem, portMAX_DELAY);
}

int32_t kv_sem_post_all(void *sem)
{
    HAL_SemaphorePost(sem);
    return 0;
}

int32_t kv_start_task(const char *name, void (*fn)(void *), void *arg,
                      uint32_t stack)
{
	(void)xTaskCreate((TaskFunction_t)fn,
                      name,
                      stack,
                      arg,
                      4,
                      &kv_task_handle);
    return 0;
}

void kv_delete_task(void)
{
    vTaskDelete(kv_task_handle);
}

void *kv_malloc(uint32_t size)
{
    return HAL_Malloc(size);
}

void kv_free(void *ptr)
{
    HAL_Free(ptr);
}
