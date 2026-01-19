#include <stdlib.h>
#include "game.h"

// Global state definitions
snekGameState gameState = { RIGHT, true, false, false };
Position fruit;
bool hasFruit = false;

uint8_t score = 0;
uint8_t highScore = 0;

Position snek[MAX_LEN];
uint8_t snekLength = 4; // Start with a snek length of 4

// game time
extern volatile uint32_t gameTimeMs;

// If the coordinate goes below 0, move it to GRID_SIZE - 1.
// If the coordinate reaches GRID_SIZE, move it back to 0.
// Notes:
//  - We use uint8_t; subtracting 1 from 0 would underflow to 255.
//    To avoid underflow side effects, we check bounds before increment/decrement.

void ResetGame(void)
{
    // Place snek centered, heading right
    snekLength = 4;
    uint8_t cx = GRID_SIZE / 2;
    uint8_t cy = GRID_SIZE / 2;
    // Head at [cx, cy], body to the left
    for (uint8_t i = 0; i < snekLength; ++i) {
        snek[i].x = (uint8_t)(cx - i);
        snek[i].y = cy;
    }
    gameState.currentDirection = RIGHT;
    gameState.isRunning = true;
    gameState.needsReset = false;
    gameState.snekIsKil = false;

    // reset game time
    gameTimeMs = 0;

    // Reset score and spawn first fruit
    score = 0; 
    SpawnFruit();
}


bool IsPositionOnSnek(uint8_t x, uint8_t y) {
    for (uint8_t i = 0; i < snekLength; ++i) {
        if (snek[i].x == x && snek[i].y == y) {
            return true;
        }
    }
    return false;
}

bool IsHeadOnSnek(void) {
    for (uint8_t i = 1; i < snekLength; ++i) {
        if ((snek[0].x == snek[i].x) && (snek[0].y == snek[i].y)) {
            return true;
        }
    }
    return false;
}

// Place fruit at random empty position
void SpawnFruit(void)
{
    uint8_t x, y;

    // Keep trying random positions until we find an empty cell
    do {
        x = rand() % GRID_SIZE;
        y = rand() % GRID_SIZE;
    } while (IsPositionOnSnek(x, y));

    fruit.x = x;
    fruit.y = y;
    hasFruit = true;
}

bool HasEatenFruit(void) {
    return ((fruit.x == snek[0].x) && (fruit.y == snek[0].y) && hasFruit);
}

void movesnek()
{
    // Shift body so each segment follows the previous one
    for (uint8_t i = snekLength; i > 0; i--) {
        snek[i] = snek[i - 1];
    }

    // if ((gameState.currentDirection == UP) && (gameState.prevDirection == DOWN)) gameState.currentDirection = gameState.prevDirection;
    // if ((gameState.currentDirection == DOWN) && (gameState.prevDirection == UP)) gameState.currentDirection = gameState.prevDirection;
    // if ((gameState.currentDirection == LEFT) && (gameState.prevDirection == RIGHT)) gameState.currentDirection = gameState.prevDirection;
    // if ((gameState.currentDirection == RIGHT) && (gameState.prevDirection == LEFT)) gameState.currentDirection = gameState.prevDirection;

    // Update head position based on direction with  wrap-around.
    switch (gameState.currentDirection) {
        case UP:
            // If at the top and moving up => wrap to bottom
            if (snek[0].y == 0) {
                snek[0].y = (uint8_t)(GRID_SIZE - 1); //Death code here
                // snek is kil
            } else {
                snek[0].y = (uint8_t)(snek[0].y - 1);
            }
            break;
        case DOWN:
            // If at the bottom and moving down => wrap to top
            if (snek[0].y == GRID_SIZE - 1) {
                snek[0].y = 0; //Death code here
                // snek is kil
            } else {
                snek[0].y = (uint8_t)(snek[0].y + 1);
            }
            break;
        case LEFT:
            if (snek[0].x == 0) {
                snek[0].x = (uint8_t)(GRID_SIZE - 1); //Death code here
                // snek is kil
            } else {
                snek[0].x = (uint8_t)(snek[0].x - 1);
            }
            break;
        case RIGHT:
            if (snek[0].x == GRID_SIZE - 1) {
                snek[0].x = 0; //Death code here
                // snek is kil
            } else {
                snek[0].x = (uint8_t)(snek[0].x + 1);
            }
            break;
    }

}
