#ifndef __FONT_H__
#define __FONT_H__

#include <Uefi.h>

// Simple 8x8 bitmap font for digits and uppercase letters
// Each character is 8 bytes, each byte represents a row (1 = pixel on)

extern CONST UINT8 Font8x8[][8];

// 16x16 Chinese character font
// Each character is 32 bytes (16 rows x 2 bytes per row)
extern CONST UINT8 FontChinese[][32];

#define FONT_WIDTH      8
#define FONT_HEIGHT     8
#define FONT_CN_WIDTH   16
#define FONT_CN_HEIGHT  16

VOID
DrawChar (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 X,
    IN UINT32 Y,
    IN CHAR8 Char,
    IN UINT32 Color
    );

VOID
DrawString (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 X,
    IN UINT32 Y,
    IN CONST CHAR8 *String,
    IN UINT32 Color
    );

VOID
DrawNumber (
    IN OUT UINT32 *FrameBuffer,
    IN UINT32 ScreenWidth,
    IN UINT32 ScreenHeight,
    IN UINT32 X,
    IN UINT32 Y,
    IN UINT32 Number,
    IN UINT32 Color
    );

#endif
