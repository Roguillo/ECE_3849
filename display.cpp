#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"

}

#include "app_objects.h"
#include "game.h"

// External timing measurement variables from main.cpp
extern volatile uint32_t gAvgPeriodUs;
extern volatile int32_t  gLastJitterSnapshot;
extern volatile uint32_t gAvgExecUs;
extern volatile uint32_t gMaxExecUs;
extern volatile uint32_t gCurrentFPS;

// game time
extern volatile uint32_t gameTimeMs;

// debug tomfoolery
extern bool debugMode;

// cpu percentage buffers
extern uint8_t snek_cpu;
extern uint8_t render_cpu;
extern uint8_t input_cpu;
extern uint8_t gCpuUtil;
extern uint8_t gNumTasks;

// Declare the formatting function
extern void FormatGameTime(char* buffer, size_t bufSize, uint32_t timeMs);

void LCD_Init(void)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    GrContextInit(&gContext, &g_sCrystalfontz128x128);
    GrContextFontSet(&gContext, &g_sFontFixed6x8);

    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);
#ifdef GrFlush
    GrFlush(&gContext);
#endif
}

static inline void drawCell(uint8_t gx, uint8_t gy, uint32_t color)
{
    // Convert grid cell to pixel rectangle
    int16_t x0 = (int16_t)(gx * CELL_SIZE);
    int16_t y0 = (int16_t)(gy * CELL_SIZE);
    tRectangle r = { x0, y0, (int16_t)(x0 + CELL_SIZE - 1), (int16_t)(y0 + CELL_SIZE - 1) };
    GrContextForegroundSet(&gContext, color);
    GrRectFill(&gContext, &r);
}

void DrawGame(const snekGameState* state)
{
    (void)state; // not used for minimal version yet

    // Clear background
    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);

    // Draw snek
    for (uint8_t i = 0; i < snekLength; ++i) {
        drawCell(snek[i].x, snek[i].y, i == 0 ? ClrGreen : ClrYellow);
    }

    // Draw fruit (if exists)
    if (hasFruit) {
        drawCell(fruit.x, fruit.y, ClrRed);
    }

    // Draw score
    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "Score: %u", score);
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDraw(&gContext, scoreText, -1, 2, 2, false);
    snprintf(scoreText, sizeof(scoreText), "High Score: %u", highScore);
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDraw(&gContext, scoreText, -1, 2, 12, false);

    //     Top timer bar
    char timeString[12];
    FormatGameTime(timeString, sizeof(timeString), gameTimeMs);
    tRectangle chronoArea = {78, 0, 127, 10};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &chronoArea);
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDraw(&gContext, timeString, -1, 80, 2, false);

    if(debugMode){
        //All Task Manager Display here
        char fpsText[20];
        snprintf(fpsText, sizeof(fpsText), "FPS:%lu", gCurrentFPS);
        //
        // char execText[30];
        // snprintf(execText, sizeof(execText), "Ex:%lu/%lu us", gAvgExecUs, gMaxExecUs);

        // // Display both metrics
        // tRectangle hud = {0, 112, 127, 127};
        // GrContextForegroundSet(&gContext, ClrBlack);
        // GrRectFill(&gContext, &hud);

        GrContextForegroundSet(&gContext, ClrWhite);
        GrStringDraw(&gContext, fpsText, -1, 2, 112, false);        // FPS on first line
        // GrStringDraw(&gContext, execText, -1, 2, 120, false);       // Execution time on second line

        char cpuText[7][20];
        char cpuTotal[20];
        char tasks[10];
        snprintf(cpuTotal, sizeof(cpuTotal), "Total CPU Util: %lu%%", gCpuUtil);
        snprintf(tasks, sizeof(tasks), "Tasks: %lu", gNumTasks-1);

        GrStringDraw(&gContext, cpuTotal, -1, 2, 40, false);
        GrStringDraw(&gContext, tasks, -1, 2, 48, false);


        for (uint8_t i = 0; i < 7; i++){
            snprintf(cpuText[i], sizeof(cpuText[i]), "%s %lu%%", gTaskCpuInfo[i].name, gTaskCpuInfo[i].cpuPercent);

            if (gTaskCpuInfo[i].cpuPercent > 80){
                GrContextForegroundSet(&gContext, ClrRed);
            } else if(gTaskCpuInfo[i].cpuPercent > 50){
                GrContextForegroundSet(&gContext, ClrYellow);
            } else {
                GrContextForegroundSet(&gContext, ClrWhite);
            }

            GrStringDraw(&gContext, cpuText[i], -1, 2, 56 + 8*i, false);
        }

        /* 0 :   Monitor
         * 1 :   IDLE 
         * 2 :   Render / tmr svc
         * 3 :   Input  / tmr svc
         * 4 :   Render / Input
         * 5 :   Snek
         * 6 :   Buzzer
         */

        char stkText[30];
        snprintf(stkText, sizeof(stkText), "STK: S:%lu R:%lu I:%lu", stackSnake.used, stackRender.used, stackInput.used);

        GrStringDraw(&gContext, stkText, -1, 2, 120, false);
    }
#ifdef GrFlush
    GrFlush(&gContext);
#endif
}

void DrawKilScren() {
    // Clear background
    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);

    // Draw score
    char scoreText[32];

    GrContextFontSet(&gContext, &g_sFontCm30);
    GrContextForegroundSet(&gContext, ClrRed);
    snprintf(scoreText, sizeof(scoreText), "SNEK");
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 25, false);
    snprintf(scoreText, sizeof(scoreText), "IS");
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 50, false);
    snprintf(scoreText, sizeof(scoreText), "KIL");
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 75, false);

    GrContextFontSet(&gContext, &g_sFontFixed6x8);
    snprintf(scoreText, sizeof(scoreText), "High Score: %u", highScore);
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 100, false);
    snprintf(scoreText, sizeof(scoreText), "Score: %u", score);
    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 110, false);


#ifdef GrFlush
    GrFlush(&gContext);
#endif
}

void OverflowScreen() {
    // Clear background
    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlue);
    GrRectFill(&gContext, &full);

    // Draw score
    char scoreText[32];

    GrContextFontSet(&gContext, &g_sFontCm16);
    GrContextForegroundSet(&gContext, ClrWhite);
    snprintf(scoreText, sizeof(scoreText), "ERROR:");
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 25, false);
    snprintf(scoreText, sizeof(scoreText), "STACK");
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 40, false);
    snprintf(scoreText, sizeof(scoreText), "OVERFLOW");
    GrStringDrawCentered(&gContext, scoreText, -1, 64, 55, false);

#ifdef GrFlush
    GrFlush(&gContext);
#endif
}
