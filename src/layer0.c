/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: capture_L0.c                                                       |
| project:  zxn::scrnshot                                                      |
| author:   S. Zell                                                            |
| date:     02/28/2026                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Image capture functions for LAYER0                                           |
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
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "scrnshot.h"
#include "layer0.h"

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
/* makeScreenshot_L00()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L00(const screenmode_t* pInfo)
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
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;              /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;              /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 4;                          /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = pInfo->uiResY * uiLineLen;  /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = pInfo->uiColors;            /* palette entries */

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
         #if (_PIXEL_CALC_ == 0)
          pPixelRow = pPixelData
                      + ((uiY & 0x07) << 8)   /* Zeile innerhalb der 8er-Gruppe        */
                      + ((uiY & 0x38) << 2)   /* 8er-Gruppe innerhalb des 64er-Blocks  */
                      + ((uiY & 0xC0) << 5);  /* welcher 64er-Block (oben/mitte/unten) */
          pAttrRow  = pAttrData
                      + ((uiY >> 3) << 5);    /* uiY / 8 * 32 */
         #elif (_PIXEL_CALC_ == 1)
          pPixelRow = zx_pxy2saddr(0, uiY);           /* Pixeladresse    */
          pAttrRow  = zx_cxy2aaddr(0, uiY >> 3);      /* Attributadresse */      
         #elif (_PIXEL_CALC_ == 2)
          pPixelRow = zxn_pixelad(0, (uint8_t) uiY);  /* Pixeladresse    */
          pAttrRow  = zx_cxy2aaddr(0, uiY >> 3);      /* Attributadresse */      
         #elif (_PIXEL_CALC_ == 3)
          pPixelRow = zxn_pixelad(0, (uint8_t) uiY);  /* Pixeladresse    */
          pAttrRow  = pAttrData + ((uiY >> 3) << 5);  /* Attributadresse */
         #else
          #error Invalid setting for calculation of pixel address !
         #endif

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
/*                                                                            */
/*----------------------------------------------------------------------------*/
