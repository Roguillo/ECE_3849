// Shared application-level objects used across modules.
// Keep this header minimal to avoid tight coupling.

#pragma once

extern "C" {
    #include "FreeRTOS.h"
    #include "semphr.h"
    #include "grlib/grlib.h"
    #include "timers.h"
}

#include <stdint.h>

extern SemaphoreHandle_t xSemJoystickInput;
extern QueueHandle_t     xQueueDirections;

#define MAX_DIRECTION_BUFFER 1  // Buffer up to 5 direction changes

#define MAX_TASKS 10


// Global graphics context for the LCD driver
extern tContext gContext;

// System clock frequency (Hz) set during startup
extern uint32_t gSysClk;

// Synchronization primitives shared by tasks
extern SemaphoreHandle_t xMutexLCD;   // Guards LCD access

extern TimerHandle_t chronoTimer;
extern volatile uint32_t gameTimeMs;

//Task Manager Globals
typedef struct {
    char     name[16];      // Task name (truncated)
    uint32_t runtime;       // Raw runtime counter
    uint8_t  cpuPercent;    // Computed CPU %
} TaskCpuInfo;

TaskCpuInfo gTaskCpuInfo[MAX_TASKS];

typedef struct {
    uint32_t allocated;  // words given to xTaskCreate
    uint32_t highWater;  // minimum free stack seen
    uint32_t used;       // derived: allocated - highWater
} StackInfo;

StackInfo stackInput;
StackInfo stackSnake;
StackInfo stackRender;
StackInfo stackMonitor;
