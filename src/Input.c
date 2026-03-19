#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include "Input.h"

EFI_STATUS
InitializeInput (
    VOID
    )
{
    if (gST->ConIn == NULL) {
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}

BOOLEAN
IsKeyPressed (
    VOID
    )
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    ZeroMem(&Key, sizeof(EFI_INPUT_KEY));

    if (gST->ConIn == NULL) {
        return FALSE;
    }

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    if (Status == EFI_NOT_READY) {
        return FALSE;
    }

    if (EFI_ERROR(Status)) {
        return FALSE;
    }

    return TRUE;
}

EFI_STATUS
ReadInput (
    OUT INPUT_STATE *InputState
    )
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    if (InputState == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    ZeroMem(&Key, sizeof(EFI_INPUT_KEY));
    ZeroMem(InputState, sizeof(INPUT_STATE));

    if (gST->ConIn == NULL) {
        return EFI_NOT_FOUND;
    }

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    while (!EFI_ERROR(Status)) {
        if (Key.ScanCode == SCAN_UP) {
            InputState->Up = TRUE;
        } else if (Key.ScanCode == SCAN_DOWN) {
            InputState->Down = TRUE;
        } else if (Key.ScanCode == SCAN_LEFT) {
            InputState->Left = TRUE;
        } else if (Key.ScanCode == SCAN_RIGHT) {
            InputState->Right = TRUE;
        } else if (Key.ScanCode == SCAN_ESC) {
            InputState->Escape = TRUE;
        }

        if (Key.UnicodeChar == ASCII_ESC) {
            InputState->Escape = TRUE;
        } else if (Key.UnicodeChar == ASCII_ENTER) {
            InputState->Enter = TRUE;
        } else if (Key.UnicodeChar == ASCII_SPACE) {
            InputState->Space = TRUE;
        } else if (Key.UnicodeChar == L'p' || Key.UnicodeChar == L'P') {
            InputState->KeyP = TRUE;
        } else if (Key.UnicodeChar == L'w' || Key.UnicodeChar == L'W') {
            InputState->Up = TRUE;
        } else if (Key.UnicodeChar == L's' || Key.UnicodeChar == L'S') {
            InputState->Down = TRUE;
        } else if (Key.UnicodeChar == L'a' || Key.UnicodeChar == L'A') {
            InputState->Left = TRUE;
        } else if (Key.UnicodeChar == L'd' || Key.UnicodeChar == L'D') {
            InputState->Right = TRUE;
        }

        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    }

    return EFI_SUCCESS;
}

VOID
ProcessInput (
    IN INPUT_STATE *InputState,
    IN OUT GAME_STATE *GameState
    )
{
    if (InputState == NULL || GameState == NULL) {
        return;
    }

    if (InputState->Up) {
        ChangeDirection(&GameState->Snake, DIR_UP);
    } else if (InputState->Down) {
        ChangeDirection(&GameState->Snake, DIR_DOWN);
    } else if (InputState->Left) {
        ChangeDirection(&GameState->Snake, DIR_LEFT);
    } else if (InputState->Right) {
        ChangeDirection(&GameState->Snake, DIR_RIGHT);
    }
}
