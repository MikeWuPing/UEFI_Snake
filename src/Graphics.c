#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include "Graphics.h"

EFI_STATUS
InitializeGraphics (
    IN EFI_HANDLE ImageHandle,
    OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop,
    OUT UINT32 *ScreenWidth,
    OUT UINT32 *ScreenHeight
    )
{
    EFI_STATUS Status;

    if (Gop == NULL) {
        Print(L"[DEBUG] Gop is NULL\n");
        return EFI_INVALID_PARAMETER;
    }

    Print(L"[DEBUG] Locating GOP protocol...\n");
    Status = gBS->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        (VOID**)Gop
    );

    if (EFI_ERROR(Status)) {
        Print(L"[DEBUG] LocateProtocol failed: %r\n", Status);
        return Status;
    }
    Print(L"[DEBUG] GOP protocol found\n");

    if (ScreenWidth != NULL) {
        *ScreenWidth = (*Gop)->Mode->Info->HorizontalResolution;
    }

    if (ScreenHeight != NULL) {
        *ScreenHeight = (*Gop)->Mode->Info->VerticalResolution;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
SetGraphicsMode (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
    IN UINT32 DesiredWidth,
    IN UINT32 DesiredHeight
    )
{
    EFI_STATUS Status;
    UINT32 ModeIndex;
    UINT32 BestMode;
    UINTN BestDiff;
    UINTN Diff;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo;

    if (Gop == NULL) {
        Print(L"[DEBUG] SetGraphicsMode: Gop is NULL\n");
        return EFI_INVALID_PARAMETER;
    }

    Print(L"[DEBUG] SetGraphicsMode: Looking for %dx%d, MaxMode=%d\n",
          DesiredWidth, DesiredHeight, Gop->Mode->MaxMode);

    BestMode = 0;
    BestDiff = 0xFFFFFFFF;

    for (ModeIndex = 0; ModeIndex < Gop->Mode->MaxMode; ModeIndex++) {
        Status = Gop->QueryMode(
            Gop,
            ModeIndex,
            &Diff,
            &ModeInfo
        );

        if (EFI_ERROR(Status)) {
            Print(L"[DEBUG] QueryMode[%d] failed: %r\n", ModeIndex, Status);
            continue;
        }

        Print(L"[DEBUG] Mode[%d]: %dx%d\n", ModeIndex,
              ModeInfo->HorizontalResolution, ModeInfo->VerticalResolution);

        Diff = (ModeInfo->HorizontalResolution > DesiredWidth ?
                ModeInfo->HorizontalResolution - DesiredWidth : DesiredWidth - ModeInfo->HorizontalResolution) +
               (ModeInfo->VerticalResolution > DesiredHeight ?
                ModeInfo->VerticalResolution - DesiredHeight : DesiredHeight - ModeInfo->VerticalResolution);

        if (Diff < BestDiff) {
            BestDiff = Diff;
            BestMode = ModeIndex;
        }

        FreePool(ModeInfo);
    }

    Print(L"[DEBUG] BestMode=%d, BestDiff=%d\n", BestMode, BestDiff);
    Print(L"[DEBUG] Calling SetMode with mode %d...\n", BestMode);

    Status = Gop->SetMode(Gop, BestMode);

    Print(L"[DEBUG] SetMode returned: %r\n", Status);

    return Status;
}

VOID
DrawRect (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 X,
    IN UINT32 Y,
    IN UINT32 Width,
    IN UINT32 Height,
    IN UINT32 Color
    )
{
    UINT32 Row, Col;

    if (FrameBuffer == NULL) {
        return;
    }

    for (Row = 0; Row < Height; Row++) {
        for (Col = 0; Col < Width; Col++) {
            UINT32 PixelX = X + Col;
            UINT32 PixelY = Y + Row;

            if (PixelX < ScreenWidth && PixelY < ScreenHeight) {
                FrameBuffer[PixelY * ScreenWidth + PixelX] = Color;
            }
        }
    }
}

VOID
ClearScreen (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 Color
    )
{
    UINT32 i;
    UINT32 TotalPixels;

    if (FrameBuffer == NULL) {
        return;
    }

    TotalPixels = ScreenWidth * ScreenHeight;

    for (i = 0; i < TotalPixels; i++) {
        FrameBuffer[i] = Color;
    }
}

VOID
PresentBuffer (
    IN UINT32 *BackBuffer,
    OUT UINT32 *FrameBuffer,
    IN UINT32 BufferSize
    )
{
    if (BackBuffer == NULL || FrameBuffer == NULL) {
        return;
    }

    CopyMem((VOID*)FrameBuffer, (VOID*)BackBuffer, BufferSize);
}

VOID
DrawGridCell (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize,
    IN UINT32 Color
    )
{
    UINT32 X, Y;

    X = GridX * CellSize;
    Y = GridY * CellSize;

    DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
             X + 1, Y + 1, CellSize - 2, CellSize - 2, Color);
}

// Helper: Draw a filled circle
STATIC
VOID
DrawFilledCircle (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 CenterX,
    IN UINT32 CenterY,
    IN UINT32 Radius,
    IN UINT32 Color
    )
{
    INT32 x, y;
    INT32 RadiusSigned = (INT32)Radius;
    INT32 RadiusSq = RadiusSigned * RadiusSigned;

    for (y = -RadiusSigned; y <= RadiusSigned; y++) {
        for (x = -RadiusSigned; x <= RadiusSigned; x++) {
            if (x * x + y * y <= RadiusSq) {
                UINT32 PixelX = CenterX + x;
                UINT32 PixelY = CenterY + y;
                if (PixelX < ScreenWidth && PixelY < ScreenHeight) {
                    FrameBuffer[PixelY * ScreenWidth + PixelX] = Color;
                }
            }
        }
    }
}

// Helper: Lighten a color
STATIC
UINT32
LightenColor (
    IN UINT32 Color,
    IN UINT8 Amount
    )
{
    UINT32 B = (Color >> 16) & 0xFF;
    UINT32 G = (Color >> 8) & 0xFF;
    UINT32 R = Color & 0xFF;

    B = (B + Amount > 255) ? 255 : B + Amount;
    G = (G + Amount > 255) ? 255 : G + Amount;
    R = (R + Amount > 255) ? 255 : R + Amount;

    return 0xFF000000 | (B << 16) | (G << 8) | R;
}

// Helper: Darken a color
STATIC
UINT32
DarkenColor (
    IN UINT32 Color,
    IN UINT8 Amount
    )
{
    UINT32 B = (Color >> 16) & 0xFF;
    UINT32 G = (Color >> 8) & 0xFF;
    UINT32 R = Color & 0xFF;

    B = (B < Amount) ? 0 : B - Amount;
    G = (G < Amount) ? 0 : G - Amount;
    R = (R < Amount) ? 0 : R - Amount;

    return 0xFF000000 | (B << 16) | (G << 8) | R;
}

// Draw a rounded rectangle with 3D effect
VOID
DrawRoundedRect3D (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 X,
    IN UINT32 Y,
    IN UINT32 Width,
    IN UINT32 Height,
    IN UINT32 Radius,
    IN UINT32 BaseColor
    )
{
    INT32 x, y;
    INT32 RadiusSigned = (INT32)Radius;
    INT32 WidthSigned = (INT32)Width;
    INT32 HeightSigned = (INT32)Height;
    INT32 RadiusSq = RadiusSigned * RadiusSigned;
    UINT32 LightColor = LightenColor(BaseColor, 60);
    UINT32 DarkColor = DarkenColor(BaseColor, 60);
    UINT32 ShadowColor = 0xFF000000;

    // Draw shadow (offset by 2 pixels)
    for (y = 0; y < HeightSigned; y++) {
        for (x = 0; x < WidthSigned; x++) {
            UINT32 PixelX = X + x + 2;
            UINT32 PixelY = Y + y + 2;
            if (PixelX < ScreenWidth && PixelY < ScreenHeight) {
                // Only draw shadow for the shape
                BOOLEAN InCorner = FALSE;
                INT32 CornerX, CornerY;

                // Top-left corner
                if (x < RadiusSigned && y < RadiusSigned) {
                    CornerX = RadiusSigned - x;
                    CornerY = RadiusSigned - y;
                    if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
                }
                // Top-right corner
                else if (x >= WidthSigned - RadiusSigned && y < RadiusSigned) {
                    CornerX = x - (WidthSigned - RadiusSigned);
                    CornerY = RadiusSigned - y;
                    if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
                }
                // Bottom-left corner
                else if (x < RadiusSigned && y >= HeightSigned - RadiusSigned) {
                    CornerX = RadiusSigned - x;
                    CornerY = y - (HeightSigned - RadiusSigned);
                    if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
                }
                // Bottom-right corner
                else if (x >= WidthSigned - RadiusSigned && y >= HeightSigned - RadiusSigned) {
                    CornerX = x - (WidthSigned - RadiusSigned);
                    CornerY = y - (HeightSigned - RadiusSigned);
                    if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
                }

                if (!InCorner) {
                    FrameBuffer[PixelY * ScreenWidth + PixelX] = ShadowColor;
                }
            }
        }
    }

    // Draw main shape with 3D effect
    for (y = 0; y < HeightSigned; y++) {
        for (x = 0; x < WidthSigned; x++) {
            UINT32 PixelX = X + x;
            UINT32 PixelY = Y + y;
            if (PixelX >= ScreenWidth || PixelY >= ScreenHeight) continue;

            // Check if in corner
            BOOLEAN InCorner = FALSE;
            INT32 CornerX, CornerY;

            // Top-left corner
            if (x < RadiusSigned && y < RadiusSigned) {
                CornerX = RadiusSigned - x;
                CornerY = RadiusSigned - y;
                if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
            }
            // Top-right corner
            else if (x >= WidthSigned - RadiusSigned && y < RadiusSigned) {
                CornerX = x - (WidthSigned - RadiusSigned);
                CornerY = RadiusSigned - y;
                if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
            }
            // Bottom-left corner
            else if (x < RadiusSigned && y >= HeightSigned - RadiusSigned) {
                CornerX = RadiusSigned - x;
                CornerY = y - (HeightSigned - RadiusSigned);
                if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
            }
            // Bottom-right corner
            else if (x >= WidthSigned - RadiusSigned && y >= HeightSigned - RadiusSigned) {
                CornerX = x - (WidthSigned - RadiusSigned);
                CornerY = y - (HeightSigned - RadiusSigned);
                if (CornerX * CornerX + CornerY * CornerY > RadiusSq) InCorner = TRUE;
            }

            if (InCorner) continue;

            // Determine color based on position for 3D effect
            UINT32 PixelColor;
            if (y < 3 || x < 3) {
                PixelColor = LightColor;  // Top and left edges - light
            } else if (y >= HeightSigned - 3 || x >= WidthSigned - 3) {
                PixelColor = DarkColor;   // Bottom and right edges - dark
            } else {
                PixelColor = BaseColor;   // Center - base color
            }

            FrameBuffer[PixelY * ScreenWidth + PixelX] = PixelColor;
        }
    }
}

// Draw snake head with detailed eyes and animated tongue
VOID
DrawSnakeHead (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize,
    IN DIRECTION Dir,
    IN UINT32 HeadColor
    )
{
    UINT32 X = GridX * CellSize;
    UINT32 Y = GridY * CellSize;
    UINT32 CenterX = X + CellSize / 2;
    UINT32 CenterY = Y + CellSize / 2;
    UINT32 EyeColor = 0xFFFFFFFF;   // White
    UINT32 PupilColor = 0xFF000000; // Black
    UINT32 TongueColor = 0xFFFF0000; // Red
    UINT32 TongueTipColor = 0xFFFF6666; // Light red

    // Draw main head (rounded circle)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY, CellSize / 2 - 1, HeadColor);

    // Add gradient effect to head (highlight)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX - 2, CenterY - 2, CellSize / 3, LightenColor(HeadColor, 50));

    // Add shadow on bottom for 3D effect
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX + 2, CenterY + 2, CellSize / 4, DarkenColor(HeadColor, 40));

    // Calculate eye positions based on direction
    INT32 EyeOffset = CellSize / 3;
    INT32 Eye1X = CenterX, Eye1Y = CenterY;
    INT32 Eye2X = CenterX, Eye2Y = CenterY;
    INT32 TongueX = CenterX, TongueY = CenterY;
    INT32 TongueLen = CellSize / 2 + 2;

    switch (Dir) {
    case DIR_UP:
        Eye1X = CenterX - EyeOffset; Eye1Y = CenterY - EyeOffset / 2;
        Eye2X = CenterX + EyeOffset; Eye2Y = CenterY - EyeOffset / 2;
        TongueY = CenterY - CellSize / 2 - TongueLen / 2;
        break;
    case DIR_DOWN:
        Eye1X = CenterX - EyeOffset; Eye1Y = CenterY + EyeOffset / 2;
        Eye2X = CenterX + EyeOffset; Eye2Y = CenterY + EyeOffset / 2;
        TongueY = CenterY + CellSize / 2 + TongueLen / 2;
        break;
    case DIR_LEFT:
        Eye1X = CenterX - EyeOffset / 2; Eye1Y = CenterY - EyeOffset;
        Eye2X = CenterX - EyeOffset / 2; Eye2Y = CenterY + EyeOffset;
        TongueX = CenterX - CellSize / 2 - TongueLen / 2;
        break;
    case DIR_RIGHT:
        Eye1X = CenterX + EyeOffset / 2; Eye1Y = CenterY - EyeOffset;
        Eye2X = CenterX + EyeOffset / 2; Eye2Y = CenterY + EyeOffset;
        TongueX = CenterX + CellSize / 2 + TongueLen / 2;
        break;
    }

    // Draw eyes (larger white part)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     Eye1X, Eye1Y, CellSize / 5, EyeColor);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     Eye2X, Eye2Y, CellSize / 5, EyeColor);

    // Draw eye shine (small white highlight)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     Eye1X - 1, Eye1Y - 1, CellSize / 12, 0xFFFFFFFF);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     Eye2X - 1, Eye2Y - 1, CellSize / 12, 0xFFFFFFFF);

    // Draw pupils (looking in movement direction)
    INT32 PupilOffset = CellSize / 12;
    INT32 Pupil1X = Eye1X, Pupil1Y = Eye1Y;
    INT32 Pupil2X = Eye2X, Pupil2Y = Eye2Y;

    switch (Dir) {
    case DIR_UP:
        Pupil1Y -= PupilOffset; Pupil2Y -= PupilOffset;
        break;
    case DIR_DOWN:
        Pupil1Y += PupilOffset; Pupil2Y += PupilOffset;
        break;
    case DIR_LEFT:
        Pupil1X -= PupilOffset; Pupil2X -= PupilOffset;
        break;
    case DIR_RIGHT:
        Pupil1X += PupilOffset; Pupil2X += PupilOffset;
        break;
    }

    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     Pupil1X, Pupil1Y, CellSize / 8, PupilColor);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     Pupil2X, Pupil2Y, CellSize / 8, PupilColor);

    // Draw animated tongue (forked, wiggling)
    if (Dir == DIR_UP || Dir == DIR_DOWN) {
        // Main tongue body
        DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                 TongueX - 2, TongueY - TongueLen / 2, 4, TongueLen, TongueColor);
        // Lighter center
        DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                 TongueX - 1, TongueY - TongueLen / 2, 2, TongueLen, TongueTipColor);
        // Fork at end
        if (Dir == DIR_UP) {
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - 4, TongueY - TongueLen / 2 - 3, 3, 3, TongueColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + 1, TongueY - TongueLen / 2 - 3, 3, 3, TongueColor);
            // Tips
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - 5, TongueY - TongueLen / 2 - 4, 2, 2, TongueTipColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + 3, TongueY - TongueLen / 2 - 4, 2, 2, TongueTipColor);
        } else {
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - 4, TongueY + TongueLen / 2, 3, 3, TongueColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + 1, TongueY + TongueLen / 2, 3, 3, TongueColor);
            // Tips
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - 5, TongueY + TongueLen / 2 + 2, 2, 2, TongueTipColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + 3, TongueY + TongueLen / 2 + 2, 2, 2, TongueTipColor);
        }
    } else {
        // Main tongue body
        DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                 TongueX - TongueLen / 2, TongueY - 2, TongueLen, 4, TongueColor);
        // Lighter center
        DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                 TongueX - TongueLen / 2, TongueY - 1, TongueLen, 2, TongueTipColor);
        // Fork at end
        if (Dir == DIR_LEFT) {
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - TongueLen / 2 - 3, TongueY - 4, 3, 3, TongueColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - TongueLen / 2 - 3, TongueY + 1, 3, 3, TongueColor);
            // Tips
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - TongueLen / 2 - 4, TongueY - 5, 2, 2, TongueTipColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX - TongueLen / 2 - 4, TongueY + 3, 2, 2, TongueTipColor);
        } else {
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + TongueLen / 2, TongueY - 4, 3, 3, TongueColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + TongueLen / 2, TongueY + 1, 3, 3, TongueColor);
            // Tips
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + TongueLen / 2 + 2, TongueY - 5, 2, 2, TongueTipColor);
            DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
                     TongueX + TongueLen / 2 + 2, TongueY + 3, 2, 2, TongueTipColor);
        }
    }

    // Draw nostrils
    UINT32 NostrilColor = DarkenColor(HeadColor, 60);
    switch (Dir) {
    case DIR_UP:
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX - 2, CenterY + 2, 1, NostrilColor);
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX + 2, CenterY + 2, 1, NostrilColor);
        break;
    case DIR_DOWN:
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX - 2, CenterY - 2, 1, NostrilColor);
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX + 2, CenterY - 2, 1, NostrilColor);
        break;
    case DIR_LEFT:
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX + 2, CenterY - 2, 1, NostrilColor);
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX + 2, CenterY + 2, 1, NostrilColor);
        break;
    case DIR_RIGHT:
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX - 2, CenterY - 2, 1, NostrilColor);
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight, CenterX - 2, CenterY + 2, 1, NostrilColor);
        break;
    }
}

// Draw snake body segment with gradient and scale pattern
VOID
DrawSnakeBody (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize,
    IN UINT32 SegmentIndex,
    IN UINT32 TotalLength,
    IN UINT32 BaseColor
    )
{
    UINT32 X = GridX * CellSize;
    UINT32 Y = GridY * CellSize;
    UINT32 CenterX = X + CellSize / 2;
    UINT32 CenterY = Y + CellSize / 2;

    // Gradient from bright color (head) to darker color (tail)
    UINT32 Gradient = (SegmentIndex * 100) / TotalLength;
    // Darken the color based on segment index
    UINT32 BodyColor = BaseColor - ((Gradient * 0x04) << 8);

    // Draw main body circle for smoother appearance
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY, CellSize / 2 - 1, BodyColor);

    // Add 3D highlight effect
    UINT32 HighlightColor = LightenColor(BodyColor, 50);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX - 2, CenterY - 2, CellSize / 4, HighlightColor);

    // Add scale pattern - overlapping circles for texture
    UINT32 ScaleColor = DarkenColor(BodyColor, 30);
    UINT32 ScaleRadius = CellSize / 6;

    // Central scale
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY, ScaleRadius, ScaleColor);

    // Surrounding scales
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX - ScaleRadius, CenterY, ScaleRadius / 2, ScaleColor);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX + ScaleRadius, CenterY, ScaleRadius / 2, ScaleColor);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY - ScaleRadius, ScaleRadius / 2, ScaleColor);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY + ScaleRadius, ScaleRadius / 2, ScaleColor);
}

// Draw obstacle (stone block) with cracks and texture
VOID
DrawObstacle (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize
    )
{
    UINT32 X = GridX * CellSize;
    UINT32 Y = GridY * CellSize;
    UINT32 CenterX = X + CellSize / 2;
    UINT32 CenterY = Y + CellSize / 2;

    // Stone colors
    UINT32 StoneColor = 0xFF6B6B6B;      // Medium gray
    UINT32 DarkStone = 0xFF4A4A4A;       // Darker gray
    UINT32 LightStone = 0xFF8A8A8A;      // Lighter gray
    UINT32 CrackColor = 0xFF2A2A2A;      // Dark cracks

    // Draw base stone block with 3D effect
    DrawRoundedRect3D(FrameBuffer, ScreenWidth, ScreenHeight,
                      X + 1, Y + 1, CellSize - 2, CellSize - 2, 3, StoneColor);

    // Add stone texture - irregular patches
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     X + CellSize / 4, Y + CellSize / 4, CellSize / 6, LightStone);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     X + 3 * CellSize / 4, Y + 3 * CellSize / 4, CellSize / 5, DarkStone);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     X + CellSize / 3, Y + 2 * CellSize / 3, CellSize / 7, LightStone);

    // Draw cracks on the stone (V-shaped cracks)
    UINT32 i;
    // Vertical crack
    for (i = 2; i < CellSize - 2; i++) {
        if (i % 3 != 0) {  // Dotted line for crack
            FrameBuffer[(Y + i) * ScreenWidth + (CenterX)] = CrackColor;
        }
    }
    // Horizontal branch crack
    for (i = CenterX - CellSize / 4; i < CenterX + CellSize / 4; i++) {
        if (i % 2 == 0) {
            FrameBuffer[(CenterY + 2) * ScreenWidth + i] = CrackColor;
        }
    }

    // Add warning stripes on edges
    UINT32 WarningColor = 0xFFFFD700; // Gold/yellow
    // Corner markers
    DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
             X + 2, Y + 2, 4, 4, WarningColor);
    DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
             X + CellSize - 6, Y + 2, 4, 4, WarningColor);
    DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
             X + 2, Y + CellSize - 6, 4, 4, WarningColor);
    DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
             X + CellSize - 6, Y + CellSize - 6, 4, 4, WarningColor);
}

// Draw food with apple shape, stem, leaf and pulse effect
VOID
DrawFood (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize,
    IN UINT32 BaseColor,
    IN UINT32 PulseAmount
    )
{
    UINT32 X = GridX * CellSize;
    UINT32 Y = GridY * CellSize;
    UINT32 CenterX = X + CellSize / 2;
    UINT32 CenterY = Y + CellSize / 2;

    // Apple colors
    UINT32 StemColor = 0xFF8B4513;       // Brown stem
    UINT32 LeafColor = 0xFF228B22;       // Green leaf

    // Pulse the glow size
    INT32 PulseOffset = (INT32)(PulseAmount % 8) - 4;
    if (PulseOffset < 0) PulseOffset = -PulseOffset;

    // Draw glow (outer ring with decreasing intensity)
    UINT32 GlowColor = BaseColor;
    INT32 CellSizeSigned = (INT32)CellSize;
    for (INT32 r = CellSizeSigned / 2 + 5 + PulseOffset; r > CellSizeSigned / 2; r--) {
        DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                         CenterX, CenterY + 1, r, GlowColor);
        GlowColor = DarkenColor(GlowColor, 35);
    }

    // Draw main apple body (slightly squashed circle)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY + 1, CellSize / 2 - 2, BaseColor);

    // Add apple dimples at top and bottom
    UINT32 DimpleColor = DarkenColor(BaseColor, 30);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY - CellSize / 4, CellSize / 5, DimpleColor);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY + CellSize / 3, CellSize / 6, DimpleColor);

    // Draw highlight (shiny spot)
    UINT32 HighlightColor = LightenColor(BaseColor, 80);
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX - 2, CenterY - 1, CellSize / 4, HighlightColor);

    // Draw stem (small brown rectangle)
    DrawRect(FrameBuffer, ScreenWidth, ScreenHeight,
             CenterX - 1, Y + 1, 2, CellSize / 4, StemColor);

    // Draw leaf (small green ellipse)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX + CellSize / 3, Y + CellSize / 4, CellSize / 5, LeafColor);

    // Draw inner core (lighter center)
    DrawFilledCircle(FrameBuffer, ScreenWidth, ScreenHeight,
                     CenterX, CenterY + 2, CellSize / 6, LightenColor(BaseColor, 40));
}

// Draw a single particle
VOID
DrawParticle (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN PARTICLE *Particle
    )
{
    UINT32 X, Y;
    UINT32 Alpha;
    UINT32 Color;

    if (Particle == NULL || !Particle->Active) {
        return;
    }

    X = Particle->X;
    Y = Particle->Y;

    if (X >= ScreenWidth || Y >= ScreenHeight) {
        return;
    }

    // Fade out based on life
    Alpha = (Particle->Life * 255) / Particle->MaxLife;
    Color = Particle->Color;

    // Apply alpha to color (simplified)
    UINT32 R = ((Color >> 16) & 0xFF) * Alpha / 255;
    UINT32 G = ((Color >> 8) & 0xFF) * Alpha / 255;
    UINT32 B = (Color & 0xFF) * Alpha / 255;

    Color = 0xFF000000 | (R << 16) | (G << 8) | B;

    // Draw particle as a small square
    FrameBuffer[Y * ScreenWidth + X] = Color;
    if (X + 1 < ScreenWidth) FrameBuffer[Y * ScreenWidth + X + 1] = Color;
    if (Y + 1 < ScreenHeight) {
        FrameBuffer[(Y + 1) * ScreenWidth + X] = Color;
        if (X + 1 < ScreenWidth) FrameBuffer[(Y + 1) * ScreenWidth + X + 1] = Color;
    }
}

// Update and draw all particles
VOID
UpdateAndDrawParticles (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN OUT PARTICLE Particles[],
    IN UINT32 MaxParticles
    )
{
    UINT32 i;

    for (i = 0; i < MaxParticles; i++) {
        if (Particles[i].Active) {
            // Draw particle
            DrawParticle(FrameBuffer, ScreenWidth, ScreenHeight, &Particles[i]);

            // Update position
            Particles[i].X += Particles[i].VelX;
            Particles[i].Y += Particles[i].VelY;

            // Apply gravity
            Particles[i].VelY += 1;

            // Decrease life
            if (Particles[i].Life > 0) {
                Particles[i].Life--;
            } else {
                Particles[i].Active = FALSE;
            }

            // Check bounds
            if (Particles[i].X >= ScreenWidth || Particles[i].Y >= ScreenHeight) {
                Particles[i].Active = FALSE;
            }
        }
    }
}

// Spawn explosion effect at position
VOID
SpawnExplosion (
    IN OUT PARTICLE Particles[],
    IN UINT32 MaxParticles,
    IN UINT32 CenterX,
    IN UINT32 CenterY,
    IN UINT32 Color
    )
{
    UINT32 i;
    UINT32 Count = 0;
    // Pre-calculated direction vectors for 20 directions (normalized approx)
    // These give a circular explosion pattern without trig functions
    INT32 DirX[20] = {
         100,   95,   80,   58,   30,
           0,  -30,  -58,  -80,  -95,
        -100,  -95,  -80,  -58,  -30,
           0,   30,   58,   80,   95
    };
    INT32 DirY[20] = {
           0,   30,   58,   80,   95,
         100,   95,   80,   58,   30,
           0,  -30,  -58,  -80,  -95,
        -100,  -95,  -80,  -58,  -30
    };

    for (i = 0; i < MaxParticles && Count < 20; i++) {
        if (!Particles[i].Active) {
            // Initialize particle
            Particles[i].X = CenterX;
            Particles[i].Y = CenterY;

            // Use pre-calculated direction vectors
            // Speed varies from 2 to 5
            INT32 Speed = 2 + (i % 4);
            Particles[i].VelX = (DirX[Count] * Speed) / 100;
            Particles[i].VelY = (DirY[Count] * Speed) / 100;

            // Make sure velocity is non-zero
            if (Particles[i].VelX == 0) Particles[i].VelX = 1;
            if (Particles[i].VelY == 0) Particles[i].VelY = -1;

            Particles[i].Life = 30 + (i % 20);
            Particles[i].MaxLife = Particles[i].Life;
            Particles[i].Color = Color;
            Particles[i].Active = TRUE;

            Count++;
        }
    }
}
