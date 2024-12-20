// Copyright 2023 sekigon-gonnoc (@sekigon-gonnoc)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// override backing_store_lock/unlock to control core1
#pragma weak backing_store_lock
#pragma weak backing_store_unlock

#define NO_DEBUG
#define NO_PRINT
#define FORCE_NKRO
#define TAPPING_TERM 300

#ifdef CRT0_EXTRA_CORES_NUMBER
#undef CRT0_EXTRA_CORES_NUMBER
#endif
#define CRT0_EXTRA_CORES_NUMBER 1
