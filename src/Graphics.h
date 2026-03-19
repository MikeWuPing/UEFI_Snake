#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include "Snake.h"

EFI_STATUS
InitializeGraphics (
    IN EFI_HANDLE ImageHandle,
    OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop,
    OUT UINT32 *ScreenWidth,
    OUT UINT32 *ScreenHeight
    );

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
    );

VOID
ClearScreen (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 Color
    );

VOID
PresentBuffer (
    IN UINT32 *BackBuffer,
    OUT UINT32 *FrameBuffer,
    IN UINT32 BufferSize
    );

VOID
DrawGridCell (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize,
    IN UINT32 Color
    );

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
    );

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
    );

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
    );

VOID
DrawObstacle (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 GridX,
    IN UINT32 GridY,
    IN UINT32 CellSize
    );

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
    );

VOID
DrawParticle (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN PARTICLE *Particle
    );

VOID
UpdateAndDrawParticles (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN OUT PARTICLE Particles[],
    IN UINT32 MaxParticles
    );

VOID
SpawnExplosion (
    IN OUT PARTICLE Particles[],
    IN UINT32 MaxParticles,
    IN UINT32 CenterX,
    IN UINT32 CenterY,
    IN UINT32 Color
    );

EFI_STATUS
SetGraphicsMode (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
    IN UINT32 DesiredWidth,
    IN UINT32 DesiredHeight
    );

#endif
