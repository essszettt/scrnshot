/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: scrnshot.h                                                         |
| project:  ZX Spectrum Next - SCRNSHOT                                        |
| author:   Stefan Zell                                                        |
| date:     08/18/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Screenshot tool for ZX Spectrum Next (dot command)                           |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 08/18/2025 STZ Engineering                                     |
|                                                                              |
| This software is provided  "as is",  without warranty of any kind, express   |
| or implied. In no event shall STZ or its contributors be held liable for any |
| direct, indirect, incidental, special or consequential damages arising out   |
| of the use of or inability to use this software.                             |
|                                                                              |
| Permission is granted to anyone  to use this  software for any purpose,      |
| including commercial applications,  and to alter it and redistribute it      |
| freely, subject to the following restrictions:                               |
|                                                                              |
| 1. Redistributions of source code must retain the above copyright            |
|    notice, definition, disclaimer, and this list of conditions.              |
|                                                                              |
| 2. Redistributions in binary form must reproduce the above copyright         |
|    notice, definition, disclaimer, and this list of conditions in            |
|    documentation and/or other materials provided with the distribution.      |
|                                                                          ;-) |
+-----------------------------------------------------------------------------*/

#if !defined(__SCRNSHOT_H__)
  #define __SCRNSHOT_H__

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
#define BMP_DPI_72 (2835)

/*============================================================================*/
/*                               Namespaces                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Konstanten                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Variablen                                    */
/*============================================================================*/

/*============================================================================*/
/*                               Strukturen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Typ-Definitionen                             */
/*============================================================================*/
/*!
Enumeration to describe all valid tasks of the application
*/
typedef enum _action
{
  ACTION_NONE = 0,
  ACTION_HELP,
  ACTION_INFO,
  ACTION_SHOT
} action_t;

/*!
Structure to describe the file header of a BMP file
*/
typedef struct _bmpfileheader
{
  uint16_t uiType;      /* "BM" = 0x4D42 (LE)       */
  uint32_t uiSize;      /* Dateigroesse             */
  uint32_t uiRes;       /* 0                        */
  uint32_t uiOffBits;   /* Offset zu den Pixeldaten */
} bmpfileheader_t;

/*!
Structure of the BMP info header
*/
typedef struct _bmpinfoheader
{
  uint32_t uiSize;          /* Size of header: 40                */
  int32_t  iWidth;          /* Breite in Pixeln                  */
  int32_t  iHeight;         /* Hoehe in Pixel (pos. = bottom-up) */
  uint16_t uiPlanes;        /* 1                                 */
  uint16_t uiBitCount;      /* 4 bei 16 Farben (Palette)         */
  uint32_t uiCompression;   /* 0 = BI_RGB (no compression)       */
  uint32_t uiSizeImage;     /* Groesse der Bilddaten             */ 
  int32_t  iXPelsPerMeter;  /* 2835 ≈ 72 DPI                     */
  int32_t  iYPelsPerMeter;  /* 2835 ≈ 72 DPI                     */
  uint32_t uiClrUsed;       /* Used palette entries: 16          */
  uint32_t uiClrImportant;  /* important colors: 0               */
} bmpinfoheader_t;

/*!
Structure to describe a palette entry of the BMP file
*/
typedef struct _bmppaletteentry
{
  uint8_t b;  /* blue  */
  uint8_t g;  /* green */
  uint8_t r;  /* red   */
  uint8_t a;  /* alpha */
} bmppaletteentry_t;

/*!
Structure to describe a region of RAM (i.e. pixel buffer, attribute buffer)
*/
typedef struct _memregion
{
  uint16_t  uiAddr;
  uint16_t  uiSize;
} memregion_t;

/*!
Structure to describe the properties of a videomode of Spectrum Next
*/
typedef struct _screenmode
{
  uint8_t   uiMode;
  uint16_t  uiResX;
  uint16_t  uiResY;
  uint16_t  uiColors;
  uint8_t   uiCelX;
  uint8_t   uiCelY;
  uint8_t   uiColorsPerCell;
  memregion_t tMemPixel;
  memregion_t tMemAttr;
} screenmode_t;

/*============================================================================*/
/*                               Prototypen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#endif /* __SCRNSHOT_H__ */
