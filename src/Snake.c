#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include "Snake.h"
#include "Graphics.h"
#include "Input.h"
#include "Font.h"

// Wrapper for compiler-generated memset calls
VOID *
memset (
    VOID    *Buffer,
    INT32   Value,
    UINTN   Length
    )
{
    SetMem(Buffer, Length, (UINT8)Value);
    return Buffer;
}

// Simple pseudo-random number generator
STATIC UINT32 gRandSeed = 12345;

UINT32
SimpleRand (
    VOID
    )
{
    gRandSeed = gRandSeed * 1103515245 + 12345;
    return (gRandSeed / 65536) % 32768;
}

VOID
SeedRand (
    IN UINT32 Seed
    )
{
    gRandSeed = Seed;
}

UINT32
GetFoodColor (
    IN FOOD_TYPE Type
    )
{
    switch (Type) {
    case FOOD_BONUS:
        return COLOR_ORANGE;
    case FOOD_SPEED:
        return COLOR_CYAN;
    case FOOD_GROWTH:
        return COLOR_PURPLE;  // Purple for growth bean
    case FOOD_NORMAL:
    default:
        return COLOR_RED;
    }
}

UINT32
GetFoodScore (
    IN FOOD_TYPE Type
    )
{
    switch (Type) {
    case FOOD_BONUS:
        return 30;
    case FOOD_SPEED:
        return 20;
    case FOOD_GROWTH:
        return 15;  // Growth bean gives 15 points
    case FOOD_NORMAL:
    default:
        return 10;
    }
}

VOID
DrawInfoPanel (
    IN OUT GAME_STATE *GameState,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight
    )
{
    UINT32 PanelX = GAME_WIDTH + 10;
    UINT32 PanelY = 20;
    UINT32 LineHeight = FONT_HEIGHT + 4;
    UINT32 SectionGap = 15;
    UINT32 i;

    if (GameState == NULL || GameState->BackBuffer == NULL) {
        return;
    }

    // Draw title "Snake" (贪吃蛇)
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x80\x81\x82", COLOR_GREEN);
    PanelY += LineHeight + SectionGap;

    // Draw separator line
    for (i = 0; i < INFO_PANEL_WIDTH - 20; i++) {
        if (PanelX + i < ScreenWidth) {
            GameState->BackBuffer[(PanelY - SectionGap/2) * ScreenWidth + PanelX + i] = COLOR_GRAY;
        }
    }

    // SCORE section
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x83\x84", COLOR_YELLOW);
    PanelY += LineHeight;
    DrawNumber(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX + 100, PanelY, GameState->Score, COLOR_WHITE);
    PanelY += LineHeight + SectionGap;

    // HIGH SCORE section
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x85\x86", COLOR_ORANGE);
    PanelY += LineHeight;
    DrawNumber(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX + 100, PanelY, GameState->HighScore, COLOR_WHITE);
    PanelY += LineHeight + SectionGap;

    // LEVEL section
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x87\x88", COLOR_CYAN);
    PanelY += LineHeight;
    DrawNumber(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX + 100, PanelY, GameState->Level, COLOR_WHITE);
    PanelY += LineHeight + SectionGap;

    // SPEED section
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x89\x8A", COLOR_PURPLE);
    PanelY += LineHeight;
    DrawNumber(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX + 100, PanelY, (150000 - GameState->Speed) / 5000 + 1, COLOR_WHITE);
    PanelY += LineHeight + SectionGap;

    // LENGTH section
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x8B\x8A", COLOR_BLUE);
    PanelY += LineHeight;
    DrawNumber(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX + 100, PanelY, GameState->Snake.Length, COLOR_WHITE);
    PanelY += LineHeight + SectionGap * 2;

    // CONTROLS section
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x92\x93", COLOR_WHITE);
    PanelY += LineHeight + 5;

    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x94\x95\x96-\x97\x98", COLOR_LIGHTGRAY);
    PanelY += LineHeight;
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "WASD-\x97\x98", COLOR_LIGHTGRAY);
    PanelY += LineHeight + 5;

    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "P-\x8C\x8D", COLOR_LIGHTGRAY);
    PanelY += LineHeight;

    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "ESC-\xA3\xA4", COLOR_LIGHTGRAY);
    PanelY += LineHeight + 5;

    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "\x99\x9A\x96", COLOR_LIGHTGRAY);
    PanelY += LineHeight;
    DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
               PanelX, PanelY, "-\x9E\x9F", COLOR_LIGHTGRAY);
    PanelY += LineHeight + SectionGap;

    // STATUS section
    if (GameState->Paused) {
        PanelY += 20;
        DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   PanelX, PanelY, "\x8C\x8D\x9F", COLOR_YELLOW);
    } else if (GameState->GameOver) {
        PanelY += 20;
        DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   PanelX, PanelY, "\x8E\x8F", COLOR_RED);
        PanelY += LineHeight;
        DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   PanelX, PanelY, "\x90\x91", COLOR_RED);
        PanelY += LineHeight + 5;
        DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   PanelX, PanelY, "\xA0\x99\x9A\x96", COLOR_WHITE);
        PanelY += LineHeight;
        DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   PanelX, PanelY, "\x9E\x9F", COLOR_WHITE);
    }

    // VERSION - Display at bottom right of info panel
    {
        UINT32 VersionY = ScreenHeight - FONT_HEIGHT - 10;
        UINT32 VersionX = PanelX + 10;
        // Draw "版本" text using Chinese characters (0xA2=版, 0xA3=本)
        DrawString(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   VersionX, VersionY, "\xA2\xA3", COLOR_GRAY);
        // Draw version number
        DrawNumber(GameState->BackBuffer, ScreenWidth, ScreenHeight,
                   VersionX + 2 * (FONT_CN_WIDTH + 3) + 5, VersionY, SNAKE_VERSION, COLOR_GRAY);
    }
}

EFI_STATUS
InitializeGame (
    OUT GAME_STATE **GameState,
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
    )
{
    GAME_STATE *Game;
    UINTN BufferSize;
    UINT32 i;

    if (GameState == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Game = AllocateZeroPool(sizeof(GAME_STATE));
    if (Game == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    BufferSize = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(UINT32);
    Game->BackBuffer = AllocatePool(BufferSize);
    if (Game->BackBuffer == NULL) {
        FreePool(Game);
        return EFI_OUT_OF_RESOURCES;
    }

    Game->Speed = 150000;
    Game->GameOver = FALSE;
    Game->Paused = FALSE;
    Game->Score = 0;
    Game->HighScore = 0;
    Game->Level = 1;
    Game->ObstacleCount = 0;
    Game->FrameCounter = 0;

    // Initialize all particles as inactive
    for (i = 0; i < MAX_PARTICLES; i++) {
        Game->Particles[i].Active = FALSE;
    }

    ResetGame(Game);
    SpawnFood(Game);
    InitAiSnakes(Game);

    *GameState = Game;
    return EFI_SUCCESS;
}

VOID
CleanupGame (
    IN GAME_STATE *GameState
    )
{
    if (GameState != NULL) {
        if (GameState->BackBuffer != NULL) {
            FreePool(GameState->BackBuffer);
        }
        FreePool(GameState);
    }
}

VOID
ResetGame (
    IN OUT GAME_STATE *GameState
    )
{
    UINT32 i;

    if (GameState == NULL) {
        return;
    }

    GameState->Snake.Length = 3;
    GameState->Snake.Dir = DIR_RIGHT;
    GameState->Snake.Grow = FALSE;

    for (i = 0; i < GameState->Snake.Length; i++) {
        GameState->Snake.Body[i].X = GRID_WIDTH / 2 - i;
        GameState->Snake.Body[i].Y = GRID_HEIGHT / 2;
    }

    GameState->GameOver = FALSE;
    GameState->Paused = FALSE;
    GameState->Speed = 150000;
    GameState->Level = 1;
    GameState->ObstacleCount = 0;

    // Clear obstacles
    for (i = 0; i < MAX_OBSTACLES; i++) {
        GameState->Obstacles[i].X = 0;
        GameState->Obstacles[i].Y = 0;
    }

    // Generate initial obstacles
    GenerateObstacles(GameState, 5);

    // Initialize AI snakes
    InitAiSnakes(GameState);
}

VOID
SpawnFood (
    IN OUT GAME_STATE *GameState
    )
{
    UINT32 X, Y;
    UINT32 i, j;
    BOOLEAN ValidPosition;
    UINT32 RandValue;

    if (GameState == NULL) {
        return;
    }

    // Update seed based on current snake position and score for variety
    gRandSeed = (UINT32)(GameState->Score + GameState->Snake.Body[0].X * 7 + GameState->Snake.Body[0].Y * 13);

    do {
        ValidPosition = TRUE;

        X = SimpleRand() % GRID_WIDTH;
        Y = SimpleRand() % GRID_HEIGHT;

        // Check collision with snake
        for (i = 0; i < GameState->Snake.Length; i++) {
            if (GameState->Snake.Body[i].X == X &&
                GameState->Snake.Body[i].Y == Y) {
                ValidPosition = FALSE;
                gRandSeed = gRandSeed * 1103515245 + 12345;
                break;
            }
        }

    // Check collision with AI snakes
    if (ValidPosition) {
        for (i = 0; i < MAX_AI_SNAKES; i++) {
            if (GameState->AiSnakes[i].Active) {
                for (j = 0; j < GameState->AiSnakes[i].Length; j++) {
                    if (GameState->AiSnakes[i].Body[j].X == X &&
                        GameState->AiSnakes[i].Body[j].Y == Y) {
                        ValidPosition = FALSE;
                        gRandSeed = gRandSeed * 1103515245 + 12345;
                        break;
                    }
                }
                if (!ValidPosition) break;
            }
        }
    }

    } while (!ValidPosition);

    // Determine food type based on random value
    // 50% normal, 20% speed, 20% bonus, 10% growth
    RandValue = SimpleRand() % 100;
    if (RandValue < 50) {
        GameState->Food.Type = FOOD_NORMAL;
    } else if (RandValue < 70) {
        GameState->Food.Type = FOOD_SPEED;
    } else if (RandValue < 90) {
        GameState->Food.Type = FOOD_BONUS;
    } else {
        GameState->Food.Type = FOOD_GROWTH;  // 10% chance for growth bean
    }

    GameState->Food.X = X;
    GameState->Food.Y = Y;
    GameState->Food.Active = TRUE;
    GameState->Food.Timer = 0;
}

VOID
GenerateObstacles (
    IN OUT GAME_STATE *GameState,
    IN UINT32 Count
    )
{
    UINT32 i, j;
    UINT32 X, Y;
    BOOLEAN ValidPosition;

    if (GameState == NULL || Count > MAX_OBSTACLES) {
        return;
    }

    for (i = 0; i < Count; i++) {
        do {
            ValidPosition = TRUE;

            X = SimpleRand() % GRID_WIDTH;
            Y = SimpleRand() % GRID_HEIGHT;

            // Don't spawn on snake
            for (j = 0; j < GameState->Snake.Length; j++) {
                if (GameState->Snake.Body[j].X == X &&
                    GameState->Snake.Body[j].Y == Y) {
                    ValidPosition = FALSE;
                    break;
                }
            }

            // Don't spawn on food
            if (ValidPosition && GameState->Food.Active &&
                GameState->Food.X == X && GameState->Food.Y == Y) {
                ValidPosition = FALSE;
            }

            // Don't spawn on existing obstacles
            if (ValidPosition) {
                for (j = 0; j < GameState->ObstacleCount; j++) {
                    if (GameState->Obstacles[j].X == X &&
                        GameState->Obstacles[j].Y == Y) {
                        ValidPosition = FALSE;
                        break;
                    }
                }
            }

            // Keep clear area around snake head for initial movement
            if (ValidPosition) {
                INT32 HeadX = (INT32)GameState->Snake.Body[0].X;
                INT32 HeadY = (INT32)GameState->Snake.Body[0].Y;
                if (X >= (UINT32)(HeadX - 3) && X <= (UINT32)(HeadX + 3) &&
                    Y >= (UINT32)(HeadY - 3) && Y <= (UINT32)(HeadY + 3)) {
                    ValidPosition = FALSE;
                }
            }
        } while (!ValidPosition);

        GameState->Obstacles[GameState->ObstacleCount].X = X;
        GameState->Obstacles[GameState->ObstacleCount].Y = Y;
        GameState->ObstacleCount++;
    }
}

BOOLEAN
CheckObstacleCollision (
    IN GAME_STATE *GameState,
    IN UINT32 X,
    IN UINT32 Y
    )
{
    UINT32 i;

    if (GameState == NULL) {
        return FALSE;
    }

    for (i = 0; i < GameState->ObstacleCount; i++) {
        if (GameState->Obstacles[i].X == X && GameState->Obstacles[i].Y == Y) {
            return TRUE;
        }
    }

    return FALSE;
}

VOID
TogglePause (
    IN OUT GAME_STATE *GameState
    )
{
    if (GameState == NULL || GameState->GameOver) {
        return;
    }

    GameState->Paused = !GameState->Paused;
}

VOID
ChangeDirection (
    IN OUT SNAKE *Snake,
    IN DIRECTION NewDir
    )
{
    if (Snake == NULL) {
        return;
    }

    if ((Snake->Dir == DIR_UP && NewDir == DIR_DOWN) ||
        (Snake->Dir == DIR_DOWN && NewDir == DIR_UP) ||
        (Snake->Dir == DIR_LEFT && NewDir == DIR_RIGHT) ||
        (Snake->Dir == DIR_RIGHT && NewDir == DIR_LEFT)) {
        return;
    }

    Snake->Dir = NewDir;
}

VOID
UpdateGame (
    IN OUT GAME_STATE *GameState
    )
{
    SNAKE *Snake;
    UINT32 i;
    UINT32 NewHeadX, NewHeadY;
    UINT32 FoodScore;

    if (GameState == NULL || GameState->GameOver || GameState->Paused) {
        return;
    }

    Snake = &GameState->Snake;

    NewHeadX = Snake->Body[0].X;
    NewHeadY = Snake->Body[0].Y;

    switch (Snake->Dir) {
    case DIR_UP:
        NewHeadY--;
        break;
    case DIR_DOWN:
        NewHeadY++;
        break;
    case DIR_LEFT:
        NewHeadX--;
        break;
    case DIR_RIGHT:
        NewHeadX++;
        break;
    default:
        break;
    }

    // Check wall collision
    if (NewHeadX >= GRID_WIDTH || NewHeadY >= GRID_HEIGHT) {
        GameState->GameOver = TRUE;
        return;
    }

    // Check self collision
    for (i = 0; i < Snake->Length; i++) {
        if (Snake->Body[i].X == NewHeadX && Snake->Body[i].Y == NewHeadY) {
            GameState->GameOver = TRUE;
            return;
        }
    }

    // Check obstacle collision
    if (CheckObstacleCollision(GameState, NewHeadX, NewHeadY)) {
        GameState->GameOver = TRUE;
        return;
    }

    // Check AI snake collision
    if (CheckAiSnakeCollision(GameState, NewHeadX, NewHeadY)) {
        GameState->GameOver = TRUE;
        return;
    }

    // Check if eating food
    if (NewHeadX == GameState->Food.X && NewHeadY == GameState->Food.Y) {
        // Determine growth amount based on food type
        if (GameState->Food.Type == FOOD_GROWTH) {
            Snake->Grow = TRUE;
            // Growth bean adds 3 segments
            Snake->Length += 2;  // +1 more later, total +3
        } else if (GameState->Food.Type == FOOD_BONUS) {
            Snake->Grow = TRUE;
            Snake->Length += 1;  // Bonus food adds 2 segments
        } else {
            Snake->Grow = TRUE;  // Normal food adds 1 segment
        }
        FoodScore = GetFoodScore(GameState->Food.Type);
        GameState->Score += FoodScore;

        // Update high score
        if (GameState->Score > GameState->HighScore) {
            GameState->HighScore = GameState->Score;
        }

        // Spawn particle explosion at food position
        {
            UINT32 FoodPixelX = GameState->Food.X * CELL_SIZE + CELL_SIZE / 2;
            UINT32 FoodPixelY = GameState->Food.Y * CELL_SIZE + CELL_SIZE / 2;
            UINT32 ExplosionColor = GetFoodColor(GameState->Food.Type);
            SpawnExplosion(GameState->Particles, MAX_PARTICLES, FoodPixelX, FoodPixelY, ExplosionColor);
        }

        // Update speed based on food type
        if (GameState->Food.Type == FOOD_SPEED) {
            // Speed food makes snake faster temporarily
            if (GameState->Speed > 30000) {
                GameState->Speed -= 20000;
            }
        } else {
            // Normal speed increase
            if (GameState->Speed > 50000) {
                GameState->Speed -= 3000;
            }
        }

        // Level up every 50 points
        GameState->Level = (GameState->Score / 50) + 1;

        // Add more obstacles at higher levels
        if (GameState->Score % 50 == 0 && GameState->ObstacleCount < MAX_OBSTACLES) {
            GenerateObstacles(GameState, 3);
        }
    }

    // Move body
    if (Snake->Grow) {
        // Body already grown, just move body positions
        for (i = Snake->Length - 1; i > 0; i--) {
            Snake->Body[i].X = Snake->Body[i - 1].X;
            Snake->Body[i].Y = Snake->Body[i - 1].Y;
        }
        Snake->Grow = FALSE;
        SpawnFood(GameState);
    } else {
        for (i = Snake->Length - 1; i > 0; i--) {
            Snake->Body[i].X = Snake->Body[i - 1].X;
            Snake->Body[i].Y = Snake->Body[i - 1].Y;
        }
    }

    Snake->Body[0].X = NewHeadX;
    Snake->Body[0].Y = NewHeadY;
}

VOID
DrawGame (
    IN GAME_STATE *GameState,
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
    )
{
    UINT32 i;
    UINT32 X, Y;
    UINT32 Color;

    if (GameState == NULL || GameState->BackBuffer == NULL || Gop == NULL) {
        return;
    }

    // Clear entire screen
    ClearScreen(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BLACK);

    // Draw game area background with subtle gradient
    for (Y = 0; Y < GAME_HEIGHT; Y++) {
        UINT32 RowColor = 0xFF101520 + (Y * 2 / GAME_HEIGHT) * 0x000505;
        for (X = 0; X < GAME_WIDTH; X++) {
            GameState->BackBuffer[Y * SCREEN_WIDTH + X] = RowColor;
        }
    }

    // Draw game area border with gradient effect
    UINT32 BorderColor1 = 0xFF00FFFF;  // Cyan
    UINT32 BorderColor2 = 0xFF0080FF;  // Blue
    for (Y = 0; Y < GAME_HEIGHT; Y++) {
        UINT32 BorderColor = (Y % 20 < 10) ? BorderColor1 : BorderColor2;
        // Left border of game area
        GameState->BackBuffer[Y * SCREEN_WIDTH + 0] = BorderColor;
        GameState->BackBuffer[Y * SCREEN_WIDTH + 1] = BorderColor;
        // Right border of game area (before info panel)
        GameState->BackBuffer[Y * SCREEN_WIDTH + GAME_WIDTH - 1] = BorderColor;
        GameState->BackBuffer[Y * SCREEN_WIDTH + GAME_WIDTH - 2] = BorderColor;
    }
    for (X = 0; X < GAME_WIDTH; X++) {
        UINT32 BorderColor = (X % 20 < 10) ? BorderColor1 : BorderColor2;
        // Top border
        GameState->BackBuffer[0 * SCREEN_WIDTH + X] = BorderColor;
        GameState->BackBuffer[1 * SCREEN_WIDTH + X] = BorderColor;
        // Bottom border
        GameState->BackBuffer[(GAME_HEIGHT - 1) * SCREEN_WIDTH + X] = BorderColor;
        GameState->BackBuffer[(GAME_HEIGHT - 2) * SCREEN_WIDTH + X] = BorderColor;
    }

    // Draw grid lines (subtle)
    UINT32 GridLineColor = 0xFF202830;
    for (X = 0; X <= GRID_WIDTH; X++) {
        UINT32 LineX = X * CELL_SIZE;
        if (LineX < GAME_WIDTH) {
            for (Y = 2; Y < GAME_HEIGHT - 2; Y++) {
                GameState->BackBuffer[Y * SCREEN_WIDTH + LineX] = GridLineColor;
            }
        }
    }
    for (Y = 0; Y <= GRID_HEIGHT; Y++) {
        UINT32 LineY = Y * CELL_SIZE;
        if (LineY < GAME_HEIGHT) {
            for (X = 2; X < GAME_WIDTH - 2; X++) {
                GameState->BackBuffer[LineY * SCREEN_WIDTH + X] = GridLineColor;
            }
        }
    }

    // Draw obstacles with new 3D brick effect
    for (i = 0; i < GameState->ObstacleCount; i++) {
        DrawObstacle(
            GameState->BackBuffer,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            GameState->Obstacles[i].X,
            GameState->Obstacles[i].Y,
            CELL_SIZE
        );
    }

    // Draw food with type-based color, glow effect and pulse animation
    if (GameState->Food.Active) {
        Color = GetFoodColor(GameState->Food.Type);
        DrawFood(
            GameState->BackBuffer,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            GameState->Food.X, GameState->Food.Y,
            CELL_SIZE, Color,
            GameState->FrameCounter
        );
    }

    // Draw AI snakes
    DrawAiSnakes(GameState);

    // Draw snake with new graphics
    for (i = 0; i < GameState->Snake.Length; i++) {
        if (i == 0) {
            // Draw head with eyes and tongue
            DrawSnakeHead(
                GameState->BackBuffer,
                SCREEN_WIDTH, SCREEN_HEIGHT,
                GameState->Snake.Body[i].X,
                GameState->Snake.Body[i].Y,
                CELL_SIZE,
                GameState->Snake.Dir,
                0xFF00D400  // Player snake head color - bright green
            );
        } else {
            // Draw body segment with gradient
            DrawSnakeBody(
                GameState->BackBuffer,
                SCREEN_WIDTH, SCREEN_HEIGHT,
                GameState->Snake.Body[i].X,
                GameState->Snake.Body[i].Y,
                CELL_SIZE,
                i,
                GameState->Snake.Length,
                0xFF00D000  // Player snake body base color - green
            );
        }
    }

    // Draw particles (effects)
    UpdateAndDrawParticles(
        GameState->BackBuffer,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        GameState->Particles,
        MAX_PARTICLES
    );

    // Draw info panel separator line
    for (Y = 0; Y < SCREEN_HEIGHT; Y++) {
        GameState->BackBuffer[Y * SCREEN_WIDTH + GAME_WIDTH] = COLOR_WHITE;
        GameState->BackBuffer[Y * SCREEN_WIDTH + GAME_WIDTH + 1] = COLOR_GRAY;
    }

    // Draw info panel background
    for (Y = 0; Y < SCREEN_HEIGHT; Y++) {
        for (X = GAME_WIDTH + 2; X < SCREEN_WIDTH; X++) {
            GameState->BackBuffer[Y * SCREEN_WIDTH + X] = 0xFF1E1E1E;
        }
    }

    // Draw info panel content
    DrawInfoPanel(GameState, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Draw pause overlay
    if (GameState->Paused) {
        UINT32 OverlayX = GAME_WIDTH / 4;
        UINT32 OverlayY = GAME_HEIGHT / 3;
        UINT32 OverlayW = GAME_WIDTH / 2;
        UINT32 OverlayH = GAME_HEIGHT / 3;

        // Draw semi-transparent background
        for (Y = OverlayY; Y < OverlayY + OverlayH; Y++) {
            for (X = OverlayX; X < OverlayX + OverlayW; X++) {
                GameState->BackBuffer[Y * SCREEN_WIDTH + X] = 0xDD000000;
            }
        }

        // Draw border
        for (X = OverlayX; X < OverlayX + OverlayW; X++) {
            GameState->BackBuffer[OverlayY * SCREEN_WIDTH + X] = COLOR_WHITE;
            GameState->BackBuffer[(OverlayY + OverlayH - 1) * SCREEN_WIDTH + X] = COLOR_WHITE;
        }
        for (Y = OverlayY; Y < OverlayY + OverlayH; Y++) {
            GameState->BackBuffer[Y * SCREEN_WIDTH + OverlayX] = COLOR_WHITE;
            GameState->BackBuffer[Y * SCREEN_WIDTH + OverlayX + OverlayW - 1] = COLOR_WHITE;
        }

        // Draw "PAUSED" text (center in overlay)
        UINT32 TextX = OverlayX + (OverlayW - 2 * (FONT_CN_WIDTH + 1)) / 2;
        UINT32 TextY = OverlayY + (OverlayH - FONT_CN_HEIGHT) / 2 - 10;
        DrawString(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, TextX, TextY, "\x8C\x8D", COLOR_YELLOW);

        // Draw instruction "Press P to continue"
        UINT32 InstX = OverlayX + (OverlayW - 3 * (FONT_CN_WIDTH + 1) - FONT_WIDTH - 1) / 2;
        UINT32 InstY = TextY + FONT_CN_HEIGHT + 10;
        DrawString(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, InstX, InstY, "\xA0 P \xA1\xA2", COLOR_WHITE);
    }

    // Draw game over overlay
    if (GameState->GameOver) {
        UINT32 OverlayX = GAME_WIDTH / 4;
        UINT32 OverlayY = GAME_HEIGHT / 3;
        UINT32 OverlayW = GAME_WIDTH / 2;
        UINT32 OverlayH = GAME_HEIGHT / 3;

        // Draw semi-transparent overlay box
        for (Y = OverlayY; Y < OverlayY + OverlayH; Y++) {
            for (X = OverlayX; X < OverlayX + OverlayW; X++) {
                GameState->BackBuffer[Y * SCREEN_WIDTH + X] = 0xDD000000;
            }
        }

        // Draw border
        for (X = OverlayX; X < OverlayX + OverlayW; X++) {
            GameState->BackBuffer[OverlayY * SCREEN_WIDTH + X] = COLOR_WHITE;
            GameState->BackBuffer[(OverlayY + OverlayH - 1) * SCREEN_WIDTH + X] = COLOR_WHITE;
        }
        for (Y = OverlayY; Y < OverlayY + OverlayH; Y++) {
            GameState->BackBuffer[Y * SCREEN_WIDTH + OverlayX] = COLOR_WHITE;
            GameState->BackBuffer[Y * SCREEN_WIDTH + OverlayX + OverlayW - 1] = COLOR_WHITE;
        }

        // Draw "GAME OVER" text
        UINT32 GameOverX = OverlayX + (OverlayW - 4 * (FONT_CN_WIDTH + 1)) / 2;
        UINT32 GameOverY = OverlayY + 20;
        DrawString(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, GameOverX, GameOverY, "\x8E\x8F\x90\x91", COLOR_RED);

        // Draw final score "SCORE"
        UINT32 ScoreLabelX = OverlayX + (OverlayW - 2 * (FONT_CN_WIDTH + 1)) / 2;
        UINT32 ScoreLabelY = GameOverY + FONT_CN_HEIGHT + 15;
        DrawString(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, ScoreLabelX, ScoreLabelY, "\x83\x84", COLOR_WHITE);

        // Draw score number
        UINT32 ScoreX = OverlayX + OverlayW / 2;
        UINT32 ScoreY = ScoreLabelY + FONT_CN_HEIGHT + 8;
        DrawNumber(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, ScoreX + 20, ScoreY, GameState->Score, COLOR_YELLOW);

        // Draw restart instruction "Press SPACE to restart"
        UINT32 RestartX = OverlayX + (OverlayW - 7 * (FONT_CN_WIDTH + 3)) / 2;
        UINT32 RestartY = OverlayY + OverlayH - FONT_CN_HEIGHT - 15;
        DrawString(GameState->BackBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, RestartX, RestartY, "\x9A\x9B\x9C\xA0\xA1\x9E\x9F", COLOR_GREEN);
    }

    PresentBuffer(
        GameState->BackBuffer,
        (UINT32*)Gop->Mode->FrameBufferBase,
        SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(UINT32)
    );
}

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
{
    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    GAME_STATE *GameState;
    INPUT_STATE InputState;
    BOOLEAN Running;

    Print(L"[DEBUG] Starting SnakeApp...\n");

    Print(L"[DEBUG] Calling InitializeGraphics...\n");
    Status = InitializeGraphics(ImageHandle, &Gop, NULL, NULL);
    if (EFI_ERROR(Status)) {
        Print(L"TU XING CHU SHI HUA SHI BAI: %r\n", Status);
        return Status;
    }
    Print(L"[DEBUG] InitializeGraphics OK\n");

    Print(L"[DEBUG] Calling SetGraphicsMode...\n");
    Status = SetGraphicsMode(Gop, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (EFI_ERROR(Status)) {
        Print(L"JING GAO: WU FA SHE ZHI JING QUE FEN BIAN LV\n");
    } else {
        Print(L"[DEBUG] SetGraphicsMode OK\n");
    }

    Print(L"[DEBUG] Calling InitializeInput...\n");
    Status = InitializeInput();
    if (EFI_ERROR(Status)) {
        Print(L"SHU RU CHU SHI HUA SHI BAI: %r\n", Status);
        return Status;
    }
    Print(L"[DEBUG] InitializeInput OK\n");

    ZeroMem(&InputState, sizeof(INPUT_STATE));

    Print(L"[DEBUG] Calling InitializeGame...\n");
    Status = InitializeGame(&GameState, Gop);
    if (EFI_ERROR(Status)) {
        Print(L"YOU XI CHU SHI HUA SHI BAI: %r\n", Status);
        return Status;
    }
    Print(L"[DEBUG] InitializeGame OK\n");

    Running = TRUE;

    while (Running) {
        ReadInput(&InputState);

        if (InputState.Escape) {
            Running = FALSE;
            break;
        }

        // Pause toggle - process before movement
        if (InputState.KeyP) {
            TogglePause(GameState);
            // Clear the key to prevent multiple toggles
            InputState.KeyP = FALSE;
            // Small delay to prevent rapid toggling
            gBS->Stall(200000);
        }

        // Process movement only when not paused
        if (!GameState->Paused) {
            if (InputState.Up) {
                ChangeDirection(&GameState->Snake, DIR_UP);
            } else if (InputState.Down) {
                ChangeDirection(&GameState->Snake, DIR_DOWN);
            } else if (InputState.Left) {
                ChangeDirection(&GameState->Snake, DIR_LEFT);
            } else if (InputState.Right) {
                ChangeDirection(&GameState->Snake, DIR_RIGHT);
            }
        }

        if (GameState->GameOver) {
            if (InputState.Space || InputState.Enter) {
                ResetGame(GameState);
                SpawnFood(GameState);
            }
        } else {
            UpdateGame(GameState);
            UpdateAiSnakes(GameState);
        }

        DrawGame(GameState, Gop);

        // Increment frame counter for animations
        GameState->FrameCounter++;

        gBS->Stall(GameState->Speed);
    }

    CleanupGame(GameState);

    return EFI_SUCCESS;
}

// AI Snake Functions

VOID
InitAiSnakes (
    IN OUT GAME_STATE *GameState
    )
{
    UINT32 i, j;

    if (GameState == NULL) {
        return;
    }

    for (i = 0; i < MAX_AI_SNAKES; i++) {
        GameState->AiSnakes[i].Active = TRUE;
        GameState->AiSnakes[i].Length = 3;
        GameState->AiSnakes[i].Grow = FALSE;
        GameState->AiSnakes[i].MoveCounter = 0;
        GameState->AiSnakes[i].Color = (i == 0) ? AI_SNAKE_COLOR_1 : AI_SNAKE_COLOR_2;

        // Set different starting positions based on AI snake index (avoid overlapping with player)
        if (i == 0) {
            GameState->AiSnakes[i].Dir = DIR_LEFT;
            for (j = 0; j < GameState->AiSnakes[i].Length; j++) {
                GameState->AiSnakes[i].Body[j].X = GRID_WIDTH / 4 + j;
                GameState->AiSnakes[i].Body[j].Y = GRID_HEIGHT / 3;
            }
        } else {
            GameState->AiSnakes[i].Dir = DIR_RIGHT;
            for (j = 0; j < GameState->AiSnakes[i].Length; j++) {
                GameState->AiSnakes[i].Body[j].X = GRID_WIDTH / 4 * 3 - j;
                GameState->AiSnakes[i].Body[j].Y = GRID_HEIGHT / 3 * 2;
            }
        }
    }
}

BOOLEAN
CheckAiSnakeCollision (
    IN GAME_STATE *GameState,
    IN UINT32 X,
    IN UINT32 Y
    )
{
    UINT32 i, j;

    if (GameState == NULL) {
        return FALSE;
    }

    for (i = 0; i < MAX_AI_SNAKES; i++) {
        if (!GameState->AiSnakes[i].Active) {
            continue;
        }

        // Check collision with AI snake body
        for (j = 0; j < GameState->AiSnakes[i].Length; j++) {
            if (GameState->AiSnakes[i].Body[j].X == X &&
                GameState->AiSnakes[i].Body[j].Y == Y) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

STATIC BOOLEAN
IsValidAiMove (
    IN GAME_STATE *GameState,
    IN UINT32 X,
    IN UINT32 Y,
    IN UINT32 AiSnakeIndex
    )
{
    UINT32 i;

    // Check boundaries
    if (X >= GRID_WIDTH || Y >= GRID_HEIGHT) {
        return FALSE;
    }

    // Check obstacles
    if (CheckObstacleCollision(GameState, X, Y)) {
        return FALSE;
    }

    // Check collision with player snake
    for (i = 0; i < GameState->Snake.Length; i++) {
        if (GameState->Snake.Body[i].X == X && GameState->Snake.Body[i].Y == Y) {
            return FALSE;
        }
    }

    // Check collision with other AI snakes
    for (i = 0; i < MAX_AI_SNAKES; i++) {
        if (i == AiSnakeIndex || !GameState->AiSnakes[i].Active) {
            continue;
        }

        UINT32 j;
        for (j = 0; j < GameState->AiSnakes[i].Length; j++) {
            if (GameState->AiSnakes[i].Body[j].X == X &&
                GameState->AiSnakes[i].Body[j].Y == Y) {
                return FALSE;
            }
        }
    }

    // Check collision with itself (except head)
    for (i = 1; i < GameState->AiSnakes[AiSnakeIndex].Length; i++) {
        if (GameState->AiSnakes[AiSnakeIndex].Body[i].X == X &&
            GameState->AiSnakes[AiSnakeIndex].Body[i].Y == Y) {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
UpdateAiSnakes (
    IN OUT GAME_STATE *GameState
    )
{
    UINT32 i, j;
    AI_SNAKE *AiSnake;
    UINT32 NewHeadX, NewHeadY;
    UINT32 FoodX, FoodY;
    DIRECTION PossibleDirs[4];
    UINT32 NumValidDirs;
    DIRECTION BestDir;
    INT32 MinDistance;
    INT32 Distance;

    if (GameState == NULL || GameState->Paused || GameState->GameOver) {
        return;
    }

    FoodX = GameState->Food.X;
    FoodY = GameState->Food.Y;

    for (i = 0; i < MAX_AI_SNAKES; i++) {
        AiSnake = &GameState->AiSnakes[i];

        if (!AiSnake->Active) {
            continue;
        }

        // AI snake moves slower than player (every 2 frames)
        AiSnake->MoveCounter++;
        if (AiSnake->MoveCounter < 2) {
            continue;
        }
        AiSnake->MoveCounter = 0;

        // Calculate possible movement directions
        NumValidDirs = 0;
        BestDir = AiSnake->Dir;
        MinDistance = 0x7FFFFFFF;

        // Try four directions
        for (j = 0; j < 4; j++) {
            DIRECTION TestDir = (DIRECTION)j;

            // Don't allow reverse direction
            if ((AiSnake->Dir == DIR_UP && TestDir == DIR_DOWN) ||
                (AiSnake->Dir == DIR_DOWN && TestDir == DIR_UP) ||
                (AiSnake->Dir == DIR_LEFT && TestDir == DIR_RIGHT) ||
                (AiSnake->Dir == DIR_RIGHT && TestDir == DIR_LEFT)) {
                continue;
            }

            NewHeadX = AiSnake->Body[0].X;
            NewHeadY = AiSnake->Body[0].Y;

            switch (TestDir) {
            case DIR_UP:
                NewHeadY--;
                break;
            case DIR_DOWN:
                NewHeadY++;
                break;
            case DIR_LEFT:
                NewHeadX--;
                break;
            case DIR_RIGHT:
                NewHeadX++;
                break;
            default:
                break;
            }

            // Check if this direction is valid
            if (IsValidAiMove(GameState, NewHeadX, NewHeadY, i)) {
                PossibleDirs[NumValidDirs++] = TestDir;

                // Calculate distance to food
                INT32 Dx = (INT32)NewHeadX - (INT32)FoodX;
                INT32 Dy = (INT32)NewHeadY - (INT32)FoodY;
                Distance = Dx * Dx + Dy * Dy;

                // Choose direction closest to food
                if (Distance < MinDistance) {
                    MinDistance = Distance;
                    BestDir = TestDir;
                }
            }
        }

        // If no valid direction, AI snake dies
        if (NumValidDirs == 0) {
            AiSnake->Active = FALSE;
            continue;
        }

        // 70% chance to choose best direction (toward food), 30% random
        UINT32 RandVal = SimpleRand() % 100;
        if (RandVal < 70) {
            AiSnake->Dir = BestDir;
        } else if (NumValidDirs > 1) {
            // Randomly select a valid direction (but not the best one)
            UINT32 RandDirIdx = SimpleRand() % NumValidDirs;
            AiSnake->Dir = PossibleDirs[RandDirIdx];
        } else {
            AiSnake->Dir = BestDir;
        }

        // Calculate new head position
        NewHeadX = AiSnake->Body[0].X;
        NewHeadY = AiSnake->Body[0].Y;

        switch (AiSnake->Dir) {
        case DIR_UP:
            NewHeadY--;
            break;
        case DIR_DOWN:
            NewHeadY++;
            break;
        case DIR_LEFT:
            NewHeadX--;
            break;
        case DIR_RIGHT:
            NewHeadX++;
            break;
        default:
            break;
        }

        // Check if eating food
        if (NewHeadX == FoodX && NewHeadY == FoodY && GameState->Food.Active) {
            AiSnake->Grow = TRUE;
            AiSnake->Length++;

            // Spawn explosion effect
            UINT32 FoodPixelX = FoodX * CELL_SIZE + CELL_SIZE / 2;
            UINT32 FoodPixelY = FoodY * CELL_SIZE + CELL_SIZE / 2;
            SpawnExplosion(GameState->Particles, MAX_PARTICLES, FoodPixelX, FoodPixelY, AiSnake->Color);

            // Respawn food
            SpawnFood(GameState);
        }

        // Move body
        if (AiSnake->Grow) {
            for (j = AiSnake->Length - 1; j > 0; j--) {
                AiSnake->Body[j].X = AiSnake->Body[j - 1].X;
                AiSnake->Body[j].Y = AiSnake->Body[j - 1].Y;
            }
            AiSnake->Grow = FALSE;
        } else {
            for (j = AiSnake->Length - 1; j > 0; j--) {
                AiSnake->Body[j].X = AiSnake->Body[j - 1].X;
                AiSnake->Body[j].Y = AiSnake->Body[j - 1].Y;
            }
        }

        AiSnake->Body[0].X = NewHeadX;
        AiSnake->Body[0].Y = NewHeadY;
    }
}

VOID
DrawAiSnakes (
    IN GAME_STATE *GameState
    )
{
    UINT32 i, j;
    AI_SNAKE *AiSnake;
    UINT32 X, Y;
    UINT32 Color;

    if (GameState == NULL || GameState->BackBuffer == NULL) {
        return;
    }

    for (i = 0; i < MAX_AI_SNAKES; i++) {
        AiSnake = &GameState->AiSnakes[i];

        if (!AiSnake->Active) {
            continue;
        }

        // AI snakes use their respective colors
        Color = AiSnake->Color;

        for (j = 0; j < AiSnake->Length; j++) {
            X = AiSnake->Body[j].X;
            Y = AiSnake->Body[j].Y;

            if (j == 0) {
                // Draw AI snake head with the same style as player snake
                DrawSnakeHead(
                    GameState->BackBuffer,
                    SCREEN_WIDTH, SCREEN_HEIGHT,
                    X, Y,
                    CELL_SIZE,
                    AiSnake->Dir,
                    Color  // AI snake head color
                );
            } else {
                // Draw AI snake body with the same style as player snake
                DrawSnakeBody(
                    GameState->BackBuffer,
                    SCREEN_WIDTH, SCREEN_HEIGHT,
                    X, Y,
                    CELL_SIZE,
                    j,
                    AiSnake->Length,
                    Color  // AI snake body base color
                );
            }
        }
    }
}
