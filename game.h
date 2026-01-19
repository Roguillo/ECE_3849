#pragma once

#include <stdbool.h>
#include <stdint.h>

// Direction for snek movement
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

// High-level game state flags
typedef struct snekGameState {
    uint8_t currentDirection;
    bool isRunning;
    bool needsReset;
    bool snekIsKil;
} snekGameState;

extern snekGameState gameState;

// Grid configuration (128x128 display, 8x8 cells)
#define GRID_SIZE 16
#define CELL_SIZE 8
#define MAX_LEN   256  // Maximum snek length (16x16 grid)

// Position on grid
typedef struct { uint8_t x, y; } Position;

// snek representation
extern Position snek[MAX_LEN];
extern uint8_t snekLength;
extern uint8_t score;
extern uint8_t highScore;


// fruit variables
extern Position fruit;
extern bool hasFruit;  // Track if fruit is currently placed


// fruit method
void SpawnFruit(void);
bool HasEatenFruit(void);

// Basic API
void ResetGame(void);
void movesnek(void);

bool IsHeadOnSnek(void);
bool IsPositionOnSnek(uint8_t x, uint8_t y);
