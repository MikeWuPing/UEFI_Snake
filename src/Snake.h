#ifndef __SNAKE_H__
#define __SNAKE_H__

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>

// Version information
#define SNAKE_VERSION   6

//
// Screen layout definition (1024x768) - Standard VGA resolution for QEMU compatibility
// Game area: 768x768 (left side)
// Info panel: 256x768 (right side)
//
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768
#define INFO_PANEL_WIDTH 256
#define GAME_WIDTH      (SCREEN_WIDTH - INFO_PANEL_WIDTH)
#define GAME_HEIGHT     SCREEN_HEIGHT

// Game grid definition
#define CELL_SIZE       16
#define GRID_WIDTH      (GAME_WIDTH / CELL_SIZE)
#define GRID_HEIGHT     (GAME_HEIGHT / CELL_SIZE)
#define SNAKE_MAX_LEN   (GRID_WIDTH * GRID_HEIGHT)

// Maximum number of obstacles
#define MAX_OBSTACLES   50

// Maximum number of particles
#define MAX_PARTICLES   100

// Particle structure for effects
typedef struct {
    UINT32 X;
    UINT32 Y;
    INT32 VelX;
    INT32 VelY;
    UINT32 Life;
    UINT32 MaxLife;
    UINT32 Color;
    BOOLEAN Active;
} PARTICLE;

// Color definitions
#define COLOR_BLACK     0xFF000000
#define COLOR_DARKGRAY  0xFF2D2D2D
#define COLOR_GRAY      0xFF808080
#define COLOR_LIGHTGRAY 0xFFC0C0C0
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_GREEN     0xFF00C800
#define COLOR_DARKGREEN 0xFF008000
#define COLOR_RED       0xFFFF0000
#define COLOR_ORANGE    0xFFFF8000
#define COLOR_YELLOW    0xFFFFFF00
#define COLOR_BLUE      0xFF0080FF
#define COLOR_CYAN      0xFF00FFFF
#define COLOR_PURPLE    0xFF8000FF

// Food types
typedef enum {
    FOOD_NORMAL = 0,
    FOOD_BONUS,
    FOOD_SPEED,
    FOOD_GROWTH  // Growth bean - adds more length
} FOOD_TYPE;

typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} DIRECTION;

typedef struct {
    UINT32 X;
    UINT32 Y;
} SNAKE_NODE;

typedef struct {
    SNAKE_NODE Body[SNAKE_MAX_LEN];
    UINT32 Length;
    DIRECTION Dir;
    BOOLEAN Grow;
} SNAKE;

typedef struct {
    UINT32 X;
    UINT32 Y;
    BOOLEAN Active;
    FOOD_TYPE Type;
    UINT32 Timer;
} FOOD;

typedef struct {
    UINT32 X;
    UINT32 Y;
} OBSTACLE;

// AI snake structure
typedef struct {
    SNAKE_NODE Body[SNAKE_MAX_LEN];
    UINT32 Length;
    DIRECTION Dir;
    BOOLEAN Grow;
    BOOLEAN Active;  // Whether AI snake is active
    UINT32 Color;    // AI snake color
    UINT32 MoveCounter;  // Movement counter (controls AI speed)
} AI_SNAKE;

// AI snake count
#define MAX_AI_SNAKES 2
#define AI_SNAKE_COLOR_1 0xFFFF00FF  // Purple
#define AI_SNAKE_COLOR_2 0xFF00FFFF  // Cyan

typedef struct {
    SNAKE Snake;
    AI_SNAKE AiSnakes[MAX_AI_SNAKES];  // AI snake array
    FOOD Food;
    OBSTACLE Obstacles[MAX_OBSTACLES];
    UINT32 ObstacleCount;
    BOOLEAN GameOver;
    BOOLEAN Paused;
    UINT32 Score;
    UINT32 HighScore;
    UINT32 Speed;
    UINT32 Level;
    UINT32 *BackBuffer;
    UINT32 FrameCounter;           // For animations
    PARTICLE Particles[MAX_PARTICLES];  // Particle effects
} GAME_STATE;

EFI_STATUS
InitializeGame (
    OUT GAME_STATE **GameState,
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
    );

VOID
CleanupGame (
    IN GAME_STATE *GameState
    );

VOID
UpdateGame (
    IN OUT GAME_STATE *GameState
    );

VOID
DrawGame (
    IN GAME_STATE *GameState,
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
    );

VOID
ChangeDirection (
    IN OUT SNAKE *Snake,
    IN DIRECTION NewDir
    );

VOID
SpawnFood (
    IN OUT GAME_STATE *GameState
    );

VOID
ResetGame (
    IN OUT GAME_STATE *GameState
    );

VOID
GenerateObstacles (
    IN OUT GAME_STATE *GameState,
    IN UINT32 Count
    );

BOOLEAN
CheckObstacleCollision (
    IN GAME_STATE *GameState,
    IN UINT32 X,
    IN UINT32 Y
    );

VOID
TogglePause (
    IN OUT GAME_STATE *GameState
    );

UINT32
GetFoodColor (
    IN FOOD_TYPE Type
    );

UINT32
GetFoodScore (
    IN FOOD_TYPE Type
    );

// AI snake related functions
VOID
InitAiSnakes (
    IN OUT GAME_STATE *GameState
    );

VOID
UpdateAiSnakes (
    IN OUT GAME_STATE *GameState
    );

BOOLEAN
CheckAiSnakeCollision (
    IN GAME_STATE *GameState,
    IN UINT32 X,
    IN UINT32 Y
    );

VOID
DrawAiSnakes (
    IN GAME_STATE *GameState
    );

#endif
