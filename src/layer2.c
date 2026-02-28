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
| Image capture functions for LAYER2                                           |
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
#include "layer2.h"

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
/* makeScreenshot_L20()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L20(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    uint16_t uiPalSize = pInfo->uiColors * sizeof(bmppaletteentry_t);
    uint32_t uiPxlSize = ((uint32_t) pInfo->uiResY) * ((uint32_t) pInfo->uiResX);

    /* Create BMP header */
    if (EOK == iReturn)
    {
      /* file header */
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      /* info header */
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;                  /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;                  /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 8;                              /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = pInfo->uiResY * pInfo->uiResX;  /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = pInfo->uiColors;                /* palette entries */

      iReturn = saveImageHeader();
    }

    /* Save color palette ... */
    if (EOK == iReturn)
    {
      iReturn = saveColourPalette(pInfo);
    }

#if 0
    if (EOK == iReturn)
    {
      bmppaletteentry_t tEntry;
      tEntry.a = 0;

      for (uint8_t uiR = 0; uiR < 8; ++uiR)
      {
        for (uint8_t uiG = 0; uiG < 8; ++uiG)
        {
          for (uint8_t uiB = 0; uiB < 4; ++uiB)
          {
           #if 0
            tEntry.r = (uiR * 255 + 3) / 7;   // oder Bit-Replikation
            tEntry.g = (uiG * 255 + 3) / 7;
            tEntry.b = (uiB * 255 + 1) / 3;
           #else
            tEntry.r = (uiR << 5) | (uiR << 2) | (uiR >> 1);
            tEntry.g = (uiG << 5) | (uiG << 2) | (uiG >> 1);
            tEntry.b = (uiB << 6) | (uiB << 4) | (uiB << 2) | uiB;
           #endif

            if (sizeof(tEntry) != esx_f_write(g_tState.bmpfile.hFile, &tEntry, sizeof(tEntry)))
            {
              iReturn = EBADF;
            }
          }
        }
      }
    }
#endif

    /* Write pixel data ... */
    if (EOK == iReturn)
    {
      const uint8_t* pPixelRow = 0;
      uint8_t  uiMMU2 = 0xFF;
      uint8_t  uiPhysBank; /* 8K bank */
      uint16_t uiPhysBank_ = 0xFFFF;
      uint32_t uiPhysBase;
      uint32_t uiPhysAddr;

      uiPhysBank = ZXN_READ_REG(0x12) << 1; /* 0x12 L2.ACTIVE.RAM.BANK | 16K bank => 8K bank*/
      uiPhysBase = UINT32_C(0x2000) * ((uint32_t) uiPhysBank);  

      intrinsic_di();
      uiMMU2 = ZXN_READ_MMU2();

      for (uint16_t uiY = pInfo->uiResY - 1; uiY != 0xFFFF; --uiY)
      {
        uiPhysAddr = uiPhysBase + (((uint32_t) uiY) * ((uint32_t) pInfo->uiResX));
        uiPhysBank = (uiPhysAddr >> 13) & 0xFF;

        if (uiPhysBank_ != ((uint16_t) uiPhysBank))
        {
          // Neue BANK mappen
          ZXN_WRITE_MMU2(uiPhysBank);
          uiPhysBank_ = ((uint16_t) uiPhysBank);
        }

        pPixelRow = ((const uint8_t*) zxn_memmap(pInfo->tMemPixel.uiAddr)) + (uiPhysAddr & 0x1FFF); 

        if (pInfo->uiResX != esx_f_write(g_tState.bmpfile.hFile, pPixelRow, pInfo->uiResX))
        {
          iReturn = EBADF;
          break;
        }
      }

      ZXN_WRITE_MMU2(uiMMU2);
      intrinsic_ei();
    }
  }
  else
  {
    iReturn = EINVAL;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L22()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L22(const screenmode_t* pInfo)
{
  return makeScreenshot_L20(pInfo);
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L23()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L23(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    uint16_t uiPalSize = pInfo->uiColors * sizeof(bmppaletteentry_t);
    uint16_t uiLineLen = pInfo->uiResX >> 1;  /* 640 pixel = 320 byte; 32bit aligned */
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
      g_tState.bmpfile.tInfoHdr.uiSizeImage = ((uint32_t) pInfo->uiResY) * ((uint32_t) uiLineLen); /* image size */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */

      iReturn = saveImageHeader();
    }

    /* save color palette ... */
   #if 0
    if (EOK == iReturn)
    {
      bmppaletteentry_t tEntry;
      tEntry.a = 0;

      for (uint8_t uiR = 0; uiR < 8; ++uiR)
      {
        for (uint8_t uiG = 0; uiG < 8; ++uiG)
        {
          for (uint8_t uiB = 0; uiB < 4; ++uiB)
          {
            tEntry.r = (uiR << 5) | (uiR << 2) | (uiR >> 1);
            tEntry.g = (uiG << 5) | (uiG << 2) | (uiG >> 1);
            tEntry.b = (uiB << 6) | (uiB << 4) | (uiB << 2) | uiB;

            if (sizeof(tEntry) != esx_f_write(g_tState.bmpfile.hFile, &tEntry, sizeof(tEntry)))
            {
              iReturn = EBADF;
            }
          }
        }
      }
    }
   #else
    if (EOK == iReturn)
    {
      iReturn = saveColourPalette(pInfo);
    }
   #endif

    /* write pixel data ... */
    if (EOK == iReturn)
    {
      uint8_t* pPixelRow = 0;
      uint8_t* pBmpLine = 0;

      if (0 == (pBmpLine = malloc(uiLineLen)))
      {
        iReturn = ENOMEM;
      }
      else
      {
        memset(pBmpLine, 0, uiLineLen);

        uint8_t  uiMMU2 = 0xFF;
        uint8_t  uiPhysBank; /* 8K bank */
        uint16_t uiPhysBank_ = 0xFFFF;
        uint32_t uiPhysBase;
        uint32_t uiPhysAddr;
        const uint8_t* pVirtBase = (const uint8_t*) zxn_memmap(pInfo->tMemPixel.uiAddr);

        uiPhysBank = ZXN_READ_REG(0x12) * 2; /* 0x12 L2.ACTIVE.RAM.BANK | 16K bank => 8K bank*/
        uiPhysBase = UINT32_C(0x2000) * ((uint32_t) uiPhysBank);  

        uiMMU2 = ZXN_READ_MMU2();

        for (uint16_t uiY = pInfo->uiResY - 1; uiY != 0xFFFF; --uiY)
        {
          intrinsic_di();

          for (uint16_t uiX = 0; uiX < uiLineLen; ++uiX)
          {
            uiPhysAddr = uiPhysBase + (((uint32_t) uiY) * ((uint32_t) uiLineLen)) + ((uint32_t) uiX);
            uiPhysBank = (uiPhysAddr >> 13) & 0xFF;

            if (uiPhysBank_ != ((uint16_t) uiPhysBank))
            {
              ZXN_WRITE_MMU2(uiPhysBank);
              uiPhysBank_ = ((uint16_t) uiPhysBank);
            }

            pBmpLine[uiX] = pVirtBase[uiPhysAddr & 0x1FFF]; 
          }

          intrinsic_ei();

          if (uiLineLen != esx_f_write(g_tState.bmpfile.hFile, pBmpLine, uiLineLen))
          {
            iReturn = EBADF;
            break;
          }
        }

        ZXN_WRITE_MMU2(uiMMU2);

        if (0 != pBmpLine)
        {
          free(pBmpLine);
          pBmpLine = 0;
        }
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
