#pragma once

// Forward declaration for game state (defined in game.h).
struct snekGameState;

// Initialize LCD controller and the grlib drawing context.
void LCD_Init(void);

// Draw the full game frame: for now just background and snek.
void DrawGame(const snekGameState* state);

void OverflowScreen();
// kil screen
void DrawKilScren();
