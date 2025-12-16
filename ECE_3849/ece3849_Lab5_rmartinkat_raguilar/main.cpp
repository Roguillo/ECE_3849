#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "sysctl_pll.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
}

// Board drivers (provided in project includes)
#include "button.h"
#include "joystick.h"
#include "buzzer.h"

// App modules per lab structure
#include "app_objects.h"
#include "game.h"
#include "display.h"

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    GPIOPinWrite(GPIO_PORTN_BASE, RED_LED, 1);
    OverflowScreen();
    vTaskEndScheduler();
}

// Shared objects
tContext gContext;
uint32_t gSysClk;
SemaphoreHandle_t xMutexLCD;
SemaphoreHandle_t xSemJoystickInput = NULL;
QueueHandle_t xQueueDirections = NULL;

// Buttons used for pause/reset
static Button btnPause(S1);
static Button btnReset(S2);
// Joystick (axes + stick push). Pins from HAL pins.h (BOOSTERPACK1)
static Joystick gJoystick(JSX, JSY, JS1);

// Config
#define INPUT_TICK_MS 20U

// debug tomfoolery
bool debugMode = false;
TaskHandle_t hInput = NULL;
TaskHandle_t hSnake = NULL;
TaskHandle_t hRender = NULL;
TaskHandle_t hMonitor = NULL;

uint8_t gNumTasks = 0;

uint8_t snek_cpu;
uint8_t render_cpu;
uint8_t input_cpu;
uint8_t gCpuUtil;

uint8_t renderMailbox;

// Prototypes
static void configureSystemClock(void);
static void vInputTask(void *pvParameters);
static void vsnekTask(void *pvParameters);
static void vRenderTask(void *pvParameters);
static void MonitorTask(void *pvParameters);

void Timing_Init(void);
uint32_t micros(void);
void Timing_PeriodTick(void);
void Timing_ExecutionStart(void);
void Timing_ExecutionEnd(void);
void ChronoCallback(TimerHandle_t xTimer);

// Timing measurement state
volatile uint32_t last_us = 0;               // Last timestamp
volatile uint32_t min_period_us = 0xFFFFFFFF;
volatile uint32_t max_period_us = 0;
volatile uint64_t sum_period_us = 0;         // For averaging
volatile uint32_t period_count  = 0;
volatile int32_t  last_jitter_us = 0;        // Signed jitter

// NEW: Execution time measurement
volatile uint32_t last_exec_start_us = 0;
volatile uint32_t min_exec_us = 0xFFFFFFFF;
volatile uint32_t max_exec_us = 0;
volatile uint64_t sum_exec_us = 0;
volatile uint32_t exec_count = 0;

uint32_t expected_period_us = 38000;         // Set per task being measured

// Global shared variables for display
volatile uint32_t gAvgPeriodUs = 0;
volatile int32_t  gLastJitterSnapshot = 0;
volatile uint32_t gAvgExecUs = 0;
volatile uint32_t gMaxExecUs = 0;

// Add to your global variables
volatile uint32_t gFrameCount = 0;
volatile uint32_t gCurrentFPS = 0;

// Global timer objects
TimerHandle_t chronoTimer = NULL;
volatile uint32_t gameTimeMs = 0;


int main(void) {
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();
    configureSystemClock();


    // Init LED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, RED_LED);


    // Init buttons and joystick
    btnPause.begin();
    btnReset.begin();
    gJoystick.begin();
    Buzzer_Init();
    btnPause.setTickIntervalMs(INPUT_TICK_MS);
    btnReset.setTickIntervalMs(INPUT_TICK_MS);
    gJoystick.setTickIntervalMs(INPUT_TICK_MS);
    btnPause.setDebounceMs(30);
    btnReset.setDebounceMs(30);
    // Optional joystick tuning
    gJoystick.setDeadzone(0.15f);
    
    Timing_Init();
    IntMasterEnable();

    // Create tasks (priorities per lab suggestion)
    xTaskCreate(vInputTask,  "Input",  67, NULL, 4, &hInput);
    xTaskCreate(vsnekTask,  "Snek", 29, NULL, 2, &hSnake);
    xTaskCreate(vRenderTask, "Render", 350, NULL, 3, &hRender);
    xTaskCreate(MonitorTask, "Monitor", 73, NULL, 1, &hMonitor);
    
    // mutex to protect rendering
    xMutexLCD = xSemaphoreCreateMutex();
    if (xMutexLCD == NULL) {
        // Mutex creation failed - handle error
        while(1);  // Halt system
    }
    // Create counting semaphore (max count = MAX_DIRECTION_BUFFER, initial count = 0)
    xSemJoystickInput = xSemaphoreCreateCounting(MAX_DIRECTION_BUFFER, 0);
    if (xSemJoystickInput == NULL) {
        while(1);  // Creation failed - halt system
    }
    // Create queue to store Direction values
    xQueueDirections = xQueueCreate(MAX_DIRECTION_BUFFER, sizeof(Direction));
    if (xQueueDirections == NULL) {
        while(1);  // Creation failed - halt system
    }

    chronoTimer = xTimerCreate(
    "Chrono",                    // Timer name (for debugging)
    pdMS_TO_TICKS(10),          // Period: 10ms = 1 centisecond
    pdTRUE,                     // Auto-reload (periodic timer)
    (void*)0,                   // Timer ID (not used, can be NULL)
    ChronoCallback);            // Callback function to execute

    if (chronoTimer != NULL) {
        // Timer created successfully, start it
        if (xTimerStart(chronoTimer, 0) != pdPASS) {
            // Failed to start timer
            while(1);  // Error handling
        }
    } else {
        // Timer creation failed (insufficient heap memory)
        while(1);  // Error handling
    }

    vTaskStartScheduler();
    while (1);
}

//-------------------------------------------------------------------
void Timing_Init(void)
{
    // Enable Timer0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    // Configure as 32-bit periodic timer counting UP
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC_UP);

    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFFFFFF);
    // Start the timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}
uint32_t micros(void)
{
    uint32_t ticks = TimerValueGet(TIMER0_BASE, TIMER_A);
    // Convert ticks to microseconds: system clock is 120 MHz
    return ticks / (gSysClk / 1000000);
}
void Timing_PeriodTick(void)
{
    uint32_t now_us = micros();

    if (last_us != 0)  // Skip first measurement
    {
        // Calculate actual period
        uint32_t actual_us = now_us - last_us;

        // Calculate jitter (signed)
        last_jitter_us = (int32_t)actual_us - (int32_t)expected_period_us;

        // Update statistics
        if (actual_us < min_period_us) min_period_us = actual_us;
        if (actual_us > max_period_us) max_period_us = actual_us;
        sum_period_us += actual_us;
        period_count++;
    }

    last_us = now_us;
}

void Timing_ExecutionStart(void)
{
    last_exec_start_us = micros();
}

void Timing_ExecutionEnd(void)
{
    if (last_exec_start_us != 0)
    {
        uint32_t exec_time_us = micros() - last_exec_start_us;

        if (exec_time_us < min_exec_us) min_exec_us = exec_time_us;
        if (exec_time_us > max_exec_us) max_exec_us = exec_time_us;
        sum_exec_us += exec_time_us;
        exec_count++;
    }
}

static void configureSystemClock(void)
{
    gSysClk = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        120000000);
}

void ChronoCallback(TimerHandle_t xTimer)
{
    (void)xTimer;  // Unused parameter

    // Only count time when game is running
    if (gameState.isRunning) {
        gameTimeMs += 10;  // Timer fires every 10ms (1 centisecond)
    }
    // If game is paused, time doesn't advance
}

void FormatGameTime(char* buffer, size_t bufSize, uint32_t timeMs) {
    uint32_t totalCs      = timeMs / 10;
    uint32_t minutes      = totalCs / 6000;
    uint32_t remainingCs  = totalCs % 6000;
    uint32_t seconds      = remainingCs/100;
    uint32_t centiseconds = remainingCs % 100;

    snprintf(buffer, bufSize, "%02lu:%02lu:%02lu", minutes, seconds, centiseconds);
}

//Task Manager Functions
void CollectCPUUsage(void) {
    UBaseType_t   numTasks = uxTaskGetNumberOfTasks();
    TaskStatus_t *taskArray;
    
    uint32_t activeTime = 0;
    
    if (numTasks > MAX_TASKS) numTasks = MAX_TASKS;

    taskArray = (TaskStatus_t *) pvPortMalloc(numTasks * sizeof(TaskStatus_t));
    if (taskArray == NULL) return;

    uint32_t totalRuntime = 0;

    numTasks = uxTaskGetSystemState(taskArray, numTasks, &totalRuntime);

    if (totalRuntime == 0) totalRuntime = 1;   // avoid divide-by-zero

    gNumTasks = numTasks;

    for(UBaseType_t i = 0; i < numTasks; i++) {
        strncpy(gTaskCpuInfo[i].name, taskArray[i].pcTaskName, 15);
        gTaskCpuInfo[i].name[15] = '\0';

        gTaskCpuInfo[i].runtime = taskArray[i].ulRunTimeCounter;

        gTaskCpuInfo[i].cpuPercent =
            (uint8_t)((taskArray[i].ulRunTimeCounter * 100ULL) / totalRuntime);

        if(strcmp(gTaskCpuInfo[i].name, "IDLE") != 0) {
            activeTime += taskArray[i].ulRunTimeCounter;
        }
    }

    gCpuUtil = (uint8_t)((activeTime * 100ULL) / totalRuntime);

    vPortFree(taskArray);
}

void CollectStackUsage(void){
    // Must match the sizes you passed to xTaskCreate
    stackInput.allocated   = 67;
    stackSnake.allocated   = 29;
    stackRender.allocated  = 350;
    stackMonitor.allocated = 73;

    stackInput.highWater   = uxTaskGetStackHighWaterMark(hInput);
    stackSnake.highWater   = uxTaskGetStackHighWaterMark(hSnake);
    stackRender.highWater  = uxTaskGetStackHighWaterMark(hRender);
    stackMonitor.highWater = uxTaskGetStackHighWaterMark(hMonitor);

    stackInput.used   = stackInput.allocated   - stackInput.highWater;
    stackSnake.used   = stackSnake.allocated   - stackSnake.highWater;
    stackRender.used  = stackRender.allocated  - stackRender.highWater;
    stackMonitor.used = stackMonitor.allocated - stackMonitor.highWater;
}

// Reads joystick/buttons and updates gameState
static void vInputTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t lastDetectedDirection;
    uint8_t newDirection = RIGHT;
    
    for (;;) {
        // Hardware button + joystick polling
        btnPause.tick();
        btnReset.tick();
        gJoystick.tick();

        
        
        // Toggle pause on S1
        if (btnPause.wasPressed()) {
            Buzzer_Post(250,50);
            gameState.isRunning = !gameState.isRunning;
        }

        // Request reset on S2
        if (btnReset.wasPressed()) {
            Buzzer_Post(500,50);
            lastDetectedDirection = RIGHT;
            newDirection = RIGHT;
            gameState.needsReset = true;
        }

        // Joystick 8-way direction mapping to game directions
        switch (gJoystick.direction8()) {
            case JoystickDir::N:
            case JoystickDir::NE:
                // gameState.currentDirection = gameState.prevDirection;
                newDirection = UP;
                break;
            case JoystickDir::S:
            case JoystickDir::SW:
                // gameState.currentDirection = gameState.prevDirection;
                newDirection = DOWN;
                break;
            case JoystickDir::SE:
            case JoystickDir::E:
                newDirection = RIGHT;
                break;
            case JoystickDir::NW:
            case JoystickDir::W:
                newDirection = LEFT;
                break;
            case JoystickDir::Center:
            default:
                // keep last direction
                break;
        }
        
        if((newDirection != lastDetectedDirection) && (gameState.isRunning)) {
            if(
                ((newDirection == UP   ) && (lastDetectedDirection != DOWN )) ||
                ((newDirection == DOWN ) && (lastDetectedDirection != UP   )) ||
                ((newDirection == LEFT ) && (lastDetectedDirection != RIGHT)) ||
                ((newDirection == RIGHT) && (lastDetectedDirection != LEFT ))
            ) {
                if (xQueueSend(xQueueDirections, &newDirection, 0) == pdPASS) {
                    xSemaphoreGive(xSemJoystickInput);
                    lastDetectedDirection = newDirection;
                }
            }
        }
    
        if(gJoystick.wasPressed()) debugMode = !debugMode;

        vTaskDelay(pdMS_TO_TICKS(INPUT_TICK_MS));
    }

}

// Advances the snek periodically
static void vsnekTask(void *pvParameters) {
    (void)pvParameters;
    ResetGame();
    TickType_t last = xTaskGetTickCount();
    Direction bufferDirection = RIGHT;
    
    for(;;){
        //  Timing_PeriodTick();        // Measure period between task executions
        //  Timing_ExecutionStart();    // Start measuring execution times

        if(gameState.needsReset) {
            ResetGame();
        }

        while(xSemaphoreTake(xSemJoystickInput, 0) == pdPASS) {
            if (xQueueReceive(xQueueDirections, &bufferDirection, 0) == pdPASS) {
                gameState.currentDirection = bufferDirection;
        }
}
        if(gameState.isRunning) {
            movesnek();
            
            if(IsHeadOnSnek()) gameState.snekIsKil = true;

            if (HasEatenFruit()) {
                score++;
                snekLength++;
                Buzzer_Post(250, 15);
                Buzzer_Post(750, 30);
                Buzzer_Post(250, 15);
                SpawnFruit();//consume
            }

            if(score > highScore) highScore = score;  
            renderMailbox++;
        }

        if (gameState.snekIsKil) {
            gameState.isRunning = false;
        }

        vTaskDelayUntil(&last,pdMS_TO_TICKS(69));
    }
}

// Renders current frame to LCD (guarded by mutex)
static void vRenderTask(void *pvParameters)
{
    (void)pvParameters;
    LCD_Init();
    TickType_t last = xTaskGetTickCount();
    
    for(;;) {
        if (renderMailbox != 0){
            Timing_PeriodTick();        // Measure period between task executions
            Timing_ExecutionStart();    // Start measuring execution times

            // take mutex before drawing
            xSemaphoreTake(xMutexLCD, portMAX_DELAY);
            if (gameState.snekIsKil){
                DrawKilScren();
                
            } else{
                DrawGame(&gameState);
            }
            // let it roam free
            xSemaphoreGive(xMutexLCD);

            gFrameCount++;
            renderMailbox = 0;
            Timing_ExecutionEnd();      // End measuring execution time
        }

        vTaskDelayUntil(&last,pdMS_TO_TICKS(38));
    }
}

void MonitorTask(void *pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(2000);  // 1-second stats update
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&last, period);

        CollectCPUUsage  ();
        CollectStackUsage();

        gCurrentFPS = gFrameCount>>1;
        gFrameCount = 0;  // Reset counter for next second

        // Copy stats from measurement functions
        uint32_t p_count = period_count;
        uint64_t p_sum   = sum_period_us;
        int32_t  jit     = last_jitter_us;

        uint32_t e_count = exec_count;
        uint64_t e_sum   = sum_exec_us;
        uint32_t e_max   = max_exec_us;

        // Compute averages
        uint32_t avg_period_us = (p_count > 0u) ? (uint32_t)(p_sum / p_count) : 0u;
        uint32_t avg_exec_us   = (e_count > 0u) ? (uint32_t)(e_sum / e_count) : 0u;

        // Store snapshots for render task to display
        gAvgPeriodUs        = avg_period_us;
        gLastJitterSnapshot = jit;
        gAvgExecUs          = avg_exec_us;
        gMaxExecUs          = e_max;

        // Reset statistics window
        min_period_us = 0xFFFFFFFFu;
        max_period_us = 0u;
        sum_period_us = 0u;
        period_count  = 0u;

        min_exec_us = 0xFFFFFFFFu;
        max_exec_us = 0u;
        sum_exec_us = 0u;
        exec_count  = 0u;
        //renderMailbox++;
    }
}
