#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"


// Hardware mapping for the buzzer (PF1 -> M0PWM1 / Generator 0)
#define BUZZER_PWM_BASE    PWM0_BASE
#define BUZZER_GEN         PWM_GEN_0
#define BUZZER_OUTNUM      PWM_OUT_1
#define BUZZER_OUTBIT      PWM_OUT_1_BIT
#define BUZZER_GPIO_BASE   GPIO_PORTF_BASE
#define BUZZER_GPIO_PIN    GPIO_PIN_1

// Buzzer event structure sent via queue
typedef struct {
    uint32_t freq_hz;  // Hz
    uint32_t duration_ms;   // milliseconds
} BuzzerEvent;

// Q variable
static QueueHandle_t gBuzzerQ;

// Public API functions
void Buzzer_Init(void);
void Buzzer_Post(uint32_t freq, uint32_t durationMs);

// Internal functions (implement these)
static void vBuzzerTask(void* pvParameters);
static void Buzzer_HWInit(void);
static void Buzzer_Start(uint32_t freq_hz);
static void Buzzer_Stop(void);
