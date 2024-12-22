
#pragma once

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


#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_DELAY 0
#define MOUSEKEY_TIME_TO_MAX 60
#define MOUSEKEY_MAX_SPEED 7
#define MOUSEKEY_WHEEL_DELAY 0