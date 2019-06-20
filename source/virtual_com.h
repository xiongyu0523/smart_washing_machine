/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIRTUAL_COM_H
#define VIRTUAL_COM_H

#include <stdint.h>

void VCOM_Init(void);
void VCOM_Write(uint8_t *buf, uint32_t len);
bool VCOM_Read(uint8_t *buf, uint32_t len, uint32_t to);

#endif /* _USB_CDC_VCOM_H_ */
