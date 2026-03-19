#ifndef __INPUT_H__
#define __INPUT_H__

#include <Uefi.h>
#include <Protocol/SimpleTextIn.h>
#include "Snake.h"

#define KEY_NONE        0x00
#define KEY_UP          SCAN_UP
#define KEY_DOWN        SCAN_DOWN
#define KEY_LEFT        SCAN_LEFT
#define KEY_RIGHT       SCAN_RIGHT
#define KEY_ESC         SCAN_ESC
#define KEY_ENTER       SCAN_NULL
#define ASCII_ESC       0x1B
#define ASCII_ENTER     0x0D
#define ASCII_SPACE     0x20

typedef struct {
    BOOLEAN Up;
    BOOLEAN Down;
    BOOLEAN Left;
    BOOLEAN Right;
    BOOLEAN Escape;
    BOOLEAN Enter;
    BOOLEAN Space;
    BOOLEAN KeyP;
} INPUT_STATE;

EFI_STATUS
InitializeInput (
    VOID
    );

EFI_STATUS
ReadInput (
    OUT INPUT_STATE *InputState
    );

BOOLEAN
IsKeyPressed (
    VOID
    );

VOID
ProcessInput (
    IN INPUT_STATE *InputState,
    IN OUT GAME_STATE *GameState
    );

#endif
