#include "buzzer.h"
#include "app_objects.h"

extern "C" {
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
}

// TODO: Implement the buzzer task
// This task should:
// 1. Wait for events from the queue
// 2. Start PWM with correct frequency
// 3. Wait for specified duration
// 4. Stop PWM
static void vBuzzerTask(void* pvParameters)
{
    (void)pvParameters;

    BuzzerEvent cmd;
    

    for (;;)
    {   
        if (xQueueReceive(gBuzzerQ, &cmd, portMAX_DELAY) == pdPASS)
        {
            if (cmd.freq_hz > 0 && cmd.duration_ms > 0)
            {
                Buzzer_Start(cmd.freq_hz);
                vTaskDelay(pdMS_TO_TICKS(cmd.duration_ms));
                Buzzer_Stop();
            }
        }
    }
}

// TODO: Implement hardware initialization
// Youve done this before! Set up GPIO pin and PWM peripheral
static void Buzzer_HWInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    GPIOPinTypePWM(BUZZER_GPIO_BASE, BUZZER_GPIO_PIN);

    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64); //1.875 MHz
}

// TODO: Configure PWM to generate specific frequency
// Calculate period from frequency and system clock
static void Buzzer_Start(uint32_t freq_hz)
{
    if (freq_hz == 0) return;

    uint32_t pwmClock = 1875000;
    uint32_t period = pwmClock / freq_hz;

    PWMGenConfigure(BUZZER_PWM_BASE, BUZZER_GEN, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(BUZZER_PWM_BASE, BUZZER_GEN, period);
    PWMPulseWidthSet(BUZZER_PWM_BASE, BUZZER_OUTNUM, period / 2);
    PWMOutputState(BUZZER_PWM_BASE, BUZZER_OUTBIT, true);
    PWMGenEnable(BUZZER_PWM_BASE, BUZZER_GEN);
}

// TODO: Disable PWM output
static void Buzzer_Stop(void)
{
    PWMOutputState(BUZZER_PWM_BASE, BUZZER_OUTBIT, false);
}

// TODO: Initialize complete buzzer subsystem
// 1. Initialize hardware (GPIO + PWM)
// 2. Create queue for events
// 3. Create and start buzzer task
void Buzzer_Init(void)
{
    Buzzer_HWInit();
    gBuzzerQ = xQueueCreate(10, sizeof(BuzzerEvent));
    xTaskCreate(vBuzzerTask, "Buzzer", 256, NULL, 2, NULL);
}

// TODO: Post sound event to queue (non-blocking)
// Create event struct and send to queue
void Buzzer_Post(uint32_t freq, uint32_t durationMs)
{
    BuzzerEvent cmd = { freq, durationMs };
    
    if (xQueueSend(gBuzzerQ, &cmd, 0) == pdPASS) {

        (void)xQueueSend(gBuzzerQ, &cmd, 0);

    }
}
