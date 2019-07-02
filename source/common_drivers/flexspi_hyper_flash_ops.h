/* Copyright 2019 NXP */

#ifndef FLEXSPI_HYPER_FLASH_OPS_H
#define FLEXSPI_HYPER_FLASH_OPS_H

#include "fsl_flexspi.h"

void flexspi_hyper_flash_init(void);
status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t address, const uint32_t *src);

#endif /* APP_PREINCLUDE_H */
