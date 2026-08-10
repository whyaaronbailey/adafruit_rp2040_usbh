// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdbool.h>
#include <stdint.h>

void c1_main_task(void);
void c1_usbh(void);
void c1_start_timer(void);
void c1_before_flash_operation(void);
void c1_after_flash_operation(void);

// Core-1 liveness diagnostics: c1_ready = the flash-operation trap loop is
// running (safe to coordinate flash writes); c1_heartbeat increments on every
// core-1 main-loop pass, so a frozen value means core 1 is dead.
extern volatile bool     c1_ready;
extern volatile uint32_t c1_heartbeat;