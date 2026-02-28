/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: capture_L1.c                                                       |
| project:  zxn::scrnshot                                                      |
| author:   S. Zell                                                            |
| date:     02/28/2026                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Image capture functions for LAYER1                                           |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 02/28/2026 STZ Engineering                                     |
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

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <z80.h>
#include <intrinsic.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "scrnshot.h"
#include "layer1.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Namespaces                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Constants                                    */
/*============================================================================*/

/*============================================================================*/
/*                               Variables                                    */
/*============================================================================*/
/*!
In dieser Struktur werden alle globalen Daten der Anwendung gespeichert.
*/
extern appstate_t g_tState;

/*!
BMP color palette including all spectrum layer 0 colors.
*/
extern const bmppaletteentry_t g_tColorPalL0[];

/*============================================================================*/
/*                               Structures                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Type-Definitions                             */
/*============================================================================*/

/*============================================================================*/
/*                               Prototypes                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Classes                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementation                               */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* makeScreenshot_L10()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L10(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    bool     bRadastan = zxn_radastan_mode();
    uint16_t uiLineLen = (bRadastan ? pInfo->uiResX >> 1 : pInfo->uiResX);
    uint16_t uiPalSize = sizeof(bmppaletteentry_t) * (bRadastan ? pInfo->uiColors >> 4 : pInfo->uiColors);
    uint32_t uiPxlSize = ((uint32_t) pInfo->uiResY) * ((uint32_t) uiLineLen);

    /* Create BMP header */
    if (EOK == iReturn)
    {
      /* Create file header */
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      /* Create info header */
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;              /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;              /* image height    */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = pInfo->uiResY * uiLineLen;  /* image size      */

      if (bRadastan)
      {
        g_tState.bmpfile.tInfoHdr.uiBitCount = 4;                         /* bits per pixel  */
        g_tState.bmpfile.tInfoHdr.uiClrUsed  = pInfo->uiColors >> 4;      /* palette entries */
      }
      else
      {
        g_tState.bmpfile.tInfoHdr.uiBitCount = 8;                         /* bits per pixel  */
        g_tState.bmpfile.tInfoHdr.uiClrUsed  = pInfo->uiColors;           /* palette entries */
      }

      iReturn = saveImageHeader();
    }

    /* Save color palette ... */
    if (EOK == iReturn)
    {
      iReturn = saveColourPalette(pInfo);
    }

    /* Write pixel data ... */
    if (EOK == iReturn)
    {
      const uint8_t* pPixelData0 = (const uint8_t*) zxn_memmap(pInfo->tMemPixel.uiAddr);
      const uint8_t* pPixelData1 = (const uint8_t*) zxn_memmap(pInfo->tMemAttr.uiAddr);
      uint8_t* pBmpLine = 0;
      uint16_t uiPixelOffset;
      uint8_t  uiPixelByte;

      if (0 == (pBmpLine = malloc(uiLineLen)))
      {
        iReturn = ENOMEM;
      }
      else
      {
        memset(pBmpLine, 0, uiLineLen);

        for (uint8_t uiY = pInfo->uiResY - 1; uiY != 0xFF; --uiY)
        {
          intrinsic_di();

          if (bRadastan)
          {
            for (uint8_t uiX = 0; uiX < pInfo->uiResX; uiX += 2)
            {
              uiPixelOffset = ((uint16_t) uiY) * pInfo->uiResX + uiX;
              uiPixelByte = *(pPixelData0 + (uiPixelOffset >> 1));

              pBmpLine[uiX >> 1] = uiPixelByte;
            }
          }
          else
          {
            for (uint8_t uiX = 0; uiX < pInfo->uiResX; ++uiX)
            {
              uiPixelOffset = ((uint16_t) uiY) * pInfo->uiResX + uiX;
              uiPixelByte = (pInfo->tMemPixel.uiSize > uiPixelOffset ? 
                            *(pPixelData0 + uiPixelOffset) :
                            *(pPixelData1 + uiPixelOffset - pInfo->tMemPixel.uiSize));

              pBmpLine[uiX] = uiPixelByte;
            }
          }

          intrinsic_ei();

          if (uiLineLen != esx_f_write(g_tState.bmpfile.hFile, pBmpLine, uiLineLen))
          {
            iReturn = EBADF;
            goto EXIT_NESTED_LOOPS;
          }
        }

      EXIT_NESTED_LOOPS:

        free(pBmpLine);
        pBmpLine = 0;
      }
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L11()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L11(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    uint16_t uiPalSize = pInfo->uiColors * sizeof(bmppaletteentry_t);
    uint8_t  uiLineLen = pInfo->uiResX >> 1;   /* 32bit aligned */
    uint32_t uiPxlSize = ((uint32_t) pInfo->uiResY) * ((uint32_t) uiLineLen);

    /* Create BMP header */
    if (EOK == iReturn)
    {
      /* file header */
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      /* info header */
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;                         /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;                         /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 4;                                     /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = pInfo->uiResY * uiLineLen;             /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */

      iReturn = saveImageHeader();
    }

    /* Save color palette ... */
    if (EOK == iReturn)
    {
      iReturn = saveColourPalette(pInfo);
    }

    /* Write pixel data ... */
    if (EOK == iReturn)
    {
      const uint8_t* pPixelData = (const uint8_t*) zxn_memmap(pInfo->tMemPixel.uiAddr);
      const uint8_t* pAttrData  = (const uint8_t*) zxn_memmap(pInfo->tMemAttr.uiAddr);
      const uint8_t* pPixelRow  = 0;
      const uint8_t* pAttrRow   = 0;
      uint8_t  uiPixelByte;
      uint8_t  uiAttrByte;
      uint8_t* pBmpLine = 0;
      uint8_t  uiBmpIdx;
      uint8_t  uiBmpPixel;

      if (0 == (pBmpLine = malloc(uiLineLen)))
      {
        iReturn = ENOMEM;
      }
      else
      {
        memset(pBmpLine, 0, uiLineLen);

        for (uint16_t uiY = pInfo->uiResY - 1; uiY != 0xFFFF; --uiY)
        {
          pPixelRow = zxn_pixelad(0, (uint8_t) uiY);  /* Pixeladresse    */
          pAttrRow  = pAttrData + ((uiY >> 3) << 5);  /* Attributadresse */

          for (uint16_t uiX = 0; uiX < pInfo->uiResX; uiX += 8)
          {
            uiPixelByte = *(pPixelRow + (uiX >> 3));
            uiAttrByte  = *(pAttrRow  + (uiX >> 3));

            /*
            Bit 7   FLASH   (0 = normal, 1 = blinkend Vorder-/Hintergrund wechseln)
            Bit 6   BRIGHT  (0 = normale Helligkeit, 1 = erhoehte Helligkeit)
            Bits 5–3 PAPER  (Hintergrundfarbe, 3 Bit: 0–7)
            Bits 2–0 INK    (Vordergrundfarbe, 3 Bit: 0–7)
            */

            for (uint16_t uiZ = 0; uiZ < 8; ++uiZ)
            {
              if (uiAttrByte & FLASH)
              {
                uiBmpPixel = (uiPixelByte & (1 << (7 - uiZ)) ?
                             (uiAttrByte & PAPER_WHITE) >> 3 :
                             (uiAttrByte & INK_WHITE));
              }
              else
              {
                uiBmpPixel = (uiPixelByte & (1 << (7 - uiZ)) ?
                             (uiAttrByte & INK_WHITE) :
                             (uiAttrByte & PAPER_WHITE) >> 3);
              }
              uiBmpPixel += (uiAttrByte & BRIGHT ? 8 : 0);

              uiBmpIdx = (uiX + uiZ) >> 1;
              pBmpLine[uiBmpIdx] = (uiX + uiZ) & 0x01 ? /* odd nibble ? */
                                   (pBmpLine[uiBmpIdx] & 0xF0) | uiBmpPixel :
                                   (pBmpLine[uiBmpIdx] & 0x0F) | (uiBmpPixel << 4);
            }
          }

          if (uiLineLen != esx_f_write(g_tState.bmpfile.hFile, pBmpLine, uiLineLen))
          {
            iReturn = EBADF;
            goto EXIT_NESTED_LOOPS;
          }
        }

      EXIT_NESTED_LOOPS:

        free(pBmpLine);
        pBmpLine = 0;
      }
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L12()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L12(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    uint16_t uiPalSize = pInfo->uiColors * sizeof(bmppaletteentry_t);
    uint8_t  uiLineLen = pInfo->uiResX >> 3;   /* 32bit aligned */
    uint32_t uiPxlSize = ((uint32_t) uiLineLen) * ((uint32_t) pInfo->uiResY);

    /*
    The ULA used by the Timex machines provides a number of additional screen
    modes. These are controlled using Port 0xff. An unfortunate side effect of
    this is that a few games, like Arkanoid, which expect reading 0xff to pro-
    duce screen and ATTR data bytes when the ULA is reading the screen memory,
    will not work, since reading 0xff on the Timex returns the last byte sent
    to the port. It is not known if this port is fully decoded but it seems
    likely that it is partially decoded, as on the Spectrum. Port 0xff is also
    used to enable/disable the timer interrupt and select which bank of memory
    to use for the horizontal MMU. The byte to output will be interpreted thus:

      Bits 0-2: Screen mode.
                  000 - screen 0
                  001 - screen 1
                  010 - hi-colour
                  110 - hi-res
      Bits 3-5: Sets the screen colour in hi-res mode.
                  000 - Black on White     100 - Green on Magenta
                  001 - Blue on Yellow     101 - Cyan on Red
                  010 - Red on Cyan        110 - Yellow on Blue
                  011 - Magenta on Green   111 - White on Black
      Bit 6:    If set disables the generation of the timer interrupt.
      Bit 7:    Selects which bank the horizontal MMU should use. 0=DOCK, 1=EX-ROM.
  
    Screen 0 is the normal screen at 0x4000. Screen 1 uses the same format but at 0x6000.

    The hi-colour screen uses the data area of screen 0 and screen 1 to create
    a 512x192 pixel screen. Columns are taken alternately from screen 0 and
    screen 1. The attribute area is not used. In this mode all colurs, including
    the BORDER, are BRIGHT, and the BORDER colour is the same as the PAPER colour.
    */

    /* Create BMP header */
    if (EOK == iReturn)
    {
      /* file header */
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      /* info header */
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;   /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;   /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 1;               /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = uiPxlSize;       /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = pInfo->uiColors; /* palette entries */

      iReturn = saveImageHeader();
    }

    /* Save color palette ... */
    if (EOK == iReturn)
    {
      uint8_t uiColorSet;
      bmppaletteentry_t tPalette[2];

      uiColorSet = (z80_inp(0xFF) >> 3) & 0x07;

      tPalette[0] = g_tColorPalL0[15 - uiColorSet];
      tPalette[1] = g_tColorPalL0[ 8 + uiColorSet];

      if (sizeof(tPalette) != esx_f_write(g_tState.bmpfile.hFile, &tPalette, sizeof(tPalette)))
      {
        iReturn = EBADF;
      }
    }

    /* Write pixel data ... */
    if (EOK == iReturn)
    {
      uint8_t* pBmpLine = 0;

      if (0 == (pBmpLine = malloc(uiLineLen)))
      {
        iReturn = ENOMEM;
      }
      else
      {
        memset(pBmpLine, 0, uiLineLen);

        for (uint16_t uiY = pInfo->uiResY - 1; uiY != 0xFFFF; --uiY)
        {
          intrinsic_di();

          for (uint16_t uiX = 0; uiX < pInfo->uiResX; uiX += 8)
          {
            /*
            Pixeldata bytewise alternating between the two memory-banks ...
            */
            pBmpLine[uiX >> 3] = *tshr_pxy2saddr(uiX, uiY);
          }

          intrinsic_ei();

          if (uiLineLen != esx_f_write(g_tState.bmpfile.hFile, pBmpLine, uiLineLen))
          {
            iReturn = EBADF;
            goto EXIT_NESTED_LOOPS;
          }
        }

      EXIT_NESTED_LOOPS:

        free(pBmpLine);
        pBmpLine = 0;
      }
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L13()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L13(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    uint16_t uiPalSize  = pInfo->uiColors * sizeof(bmppaletteentry_t);
    uint8_t  uiLineLen  = pInfo->uiResX >> 1;   /* 32bit aligned */
    uint32_t uiPxlSize  = ((uint32_t) pInfo->uiResY) * ((uint32_t) uiLineLen);

    /* Create BMP header */
    if (EOK == iReturn)
    {
      /* file header */
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      /* info header */
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;                         /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;                         /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 4;                                     /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = uiPxlSize;                             /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */

      iReturn = saveImageHeader();
    }

    /* Save color palette ... */
    if (EOK == iReturn)
    {
      iReturn = saveColourPalette(pInfo);
    }

    /* write pixel data ... */
    if (EOK == iReturn)
    {
      const uint8_t* pPixelData = (const uint8_t*) zxn_memmap(pInfo->tMemPixel.uiAddr);
      const uint8_t* pAttrData  = (const uint8_t*) zxn_memmap(pInfo->tMemAttr.uiAddr);
      const uint8_t* pPixelRow  = 0;
      const uint8_t* pAttrRow   = 0;
      uint8_t  uiPixelByte;
      uint8_t  uiAttrByte;
      uint8_t* pBmpLine = 0;
      uint8_t  uiBmpIdx;
      uint8_t  uiBmpPixel;

      if (0 == (pBmpLine = malloc(uiLineLen)))
      {
        iReturn = ENOMEM;
      }
      else
      {
        memset(pBmpLine, 0, uiLineLen);

        for (uint16_t uiY = pInfo->uiResY - 1; uiY != 0xFFFF; --uiY)
        {
          /*
          Attributes interleaved like pixel data ...
          */

         #if (_PIXEL_CALC_ == 0)
          uiZ = ((uiY & 0x07) << 8) +  /* Zeile innerhalb der 8er-Gruppe        */
                ((uiY & 0x38) << 2) +  /* 8er-Gruppe innerhalb des 64er-Blocks  */
                ((uiY & 0xC0) << 5);   /* welcher 64er-Block (oben/mitte/unten) */
          pPixelRow = pPixelData + uiZ;
          pAttrRow  = pAttrData  + uiY;
         #elif (_PIXEL_CALC_ == 1)
          pPixelRow = tshc_py2saddr(uiY);
          pAttrRow  = tshc_py2aaddr(uiY);
         #else
          pPixelRow = tshc_py2saddr(uiY);
          pAttrRow  = tshc_saddr2aaddr(pPixelRow);
         #endif

          for (uint16_t uiX = 0; uiX < pInfo->uiResX; uiX += 8)
          {
            uiPixelByte = *(pPixelRow + (uiX >> 3));
            uiAttrByte  = *(pAttrRow  + (uiX >> 3));

            /*
            Bit 7   FLASH   (0 = normal, 1 = blinkend Vorder-/Hintergrund wechseln)
            Bit 6   BRIGHT  (0 = normale Helligkeit, 1 = erhoehte Helligkeit)
            Bit 5–3 PAPER   (Hintergrundfarbe, 3 Bit: 0–7)
            Bit 2–0 INK     (Vordergrundfarbe, 3 Bit: 0–7)
            */

            for (uint16_t uiZ = 0; uiZ < 8; ++uiZ)
            {
              if (uiAttrByte & FLASH)
              {
                uiBmpPixel = (uiPixelByte & (1 << (7 - uiZ)) ?
                             (uiAttrByte & PAPER_WHITE) >> 3 :
                             (uiAttrByte & INK_WHITE));
              }
              else
              {
                uiBmpPixel = (uiPixelByte & (1 << (7 - uiZ)) ?
                             (uiAttrByte & INK_WHITE) :
                             (uiAttrByte & PAPER_WHITE) >> 3);
              }
              uiBmpPixel += (uiAttrByte & BRIGHT ? 8 : 0);

              uiBmpIdx = (uiX + uiZ) >> 1;
              pBmpLine[uiBmpIdx] = (uiX + uiZ) & 0x01 ? /* odd nibble ? */
                                   (pBmpLine[uiBmpIdx] & 0xF0) |  uiBmpPixel :
                                   (pBmpLine[uiBmpIdx] & 0x0F) | (uiBmpPixel << 4);
            }
          }

          if (uiLineLen != esx_f_write(g_tState.bmpfile.hFile, pBmpLine, uiLineLen))
          {
            iReturn = EBADF;
            goto EXIT_NESTED_LOOPS;
          }
        }

      EXIT_NESTED_LOOPS:

        free(pBmpLine);
        pBmpLine = 0;
      }
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
