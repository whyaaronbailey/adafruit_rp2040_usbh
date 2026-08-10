// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#include "c1.h"
#include "ch.h"
#include "hal.h"
#include "hardware/sync.h"
#include "timer.h"

// 1ms repeat timer for USB frame
extern void pio_usb_host_frame(void);

static virtual_timer_t vt;
static volatile bool   c1_stop_flag;
static volatile bool   c1_restart_flag;
static volatile bool   c1_pause_active;

// Liveness/readiness, read by core 0. c1_ready means the trap loop is running,
// i.e. core 1 can be parked safely before a flash operation cuts XIP.
volatile bool     c1_ready;
volatile uint32_t c1_heartbeat;

static uint32_t next_sof;

static void __no_inline_not_in_flash_func(timer_cb)(virtual_timer_t *_vt, void *_) {
    static bool sync;

    // Adjust frame interval to 1ms
    uint32_t enter_time = st_lld_get_counter();
    if (enter_time >= next_sof) {
        sync = false;
    } else {
        while (st_lld_get_counter() <= next_sof) {
            continue;
        }
    }

    // Start USB frame
    pio_usb_host_frame();

    // Schedule next frame
    if (!sync) {
        sync = true;
        next_sof = enter_time + 1010;
    } else {
        next_sof += 1000;
    }
}

// Called on core 0 before any wear-leveling flash operation (which disables
// XIP). Parks core 1 in a RAM spin first: if core 1 executes flash-resident
// code while XIP is off it hard-faults, taking the USB host stack with it.
void c1_before_flash_operation(void) {
    // Close the startup window: wait until core 1's trap loop is running.
    // Timeout so EEPROM still works if core 1 is absent or already dead.
    uint32_t t0 = timer_read32();
    while (!c1_ready && timer_elapsed32(t0) < 1000) {
        continue;
    }
    if (!c1_ready) {
        return;
    }
    c1_stop_flag = true;
    while (c1_stop_flag) {
        continue;
    }
    c1_pause_active = true;
}

void c1_after_flash_operation(void) {
    if (!c1_pause_active) {
        return;  // the pause was skipped (core 1 not up); nothing to resume
    }
    c1_restart_flag = true;
    while (c1_restart_flag) {
        continue;
    }
    c1_pause_active = false;
}

static void __no_inline_not_in_flash_func(c1_trap_for_flash_operation)(void) {
    c1_ready = true;
    c1_heartbeat++;
    if (c1_stop_flag) {
        chVTReset(&vt);  // flash code is still safe: core 0 waits for our ack
        // Core-1 interrupts must be off while XIP is disabled — ISR handlers
        // live in flash. Ack only after they are masked.
        uint32_t irq = save_and_disable_interrupts();
        c1_stop_flag  = false;
        while (!c1_restart_flag) {
            continue;
        }
        restore_interrupts(irq);
        c1_restart_flag = false;
        chVTSetContinuous(&vt, TIME_MS2I(1), timer_cb, NULL);
    }
}

// Main process for core1
static THD_WORKING_AREA(wa_c1_main_task_wrapper, 2048);
static THD_FUNCTION(c1_main_task_wrapper, arg) {
    while (1) {
        c1_main_task();
        c1_trap_for_flash_operation();
        chThdSleepMicroseconds(125);
    }
}

// Entry point of core1
void c1_main(void) {
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);
    chSysUnlock();

    // USB host stack uses PIO and DMA
    hal_lld_peripheral_unreset(RESETS_ALLREG_PIO0);
    hal_lld_peripheral_unreset(RESETS_ALLREG_PIO1);
    hal_lld_peripheral_unreset(RESETS_ALLREG_DMA);

    // Initialize USB host stack
    c1_usbh();

    // Start main task
    chThdCreateStatic(wa_c1_main_task_wrapper, sizeof(wa_c1_main_task_wrapper), NORMALPRIO + 1, c1_main_task_wrapper, NULL);
}

// Start 1ms timer
void c1_start_timer(void) {
    chVTObjectInit(&vt);
    chVTSetContinuous(&vt, TIME_MS2I(1), timer_cb, NULL);
    next_sof = st_lld_get_counter() + 1010;
}