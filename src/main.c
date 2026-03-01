/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: main.c                                                             |
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

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "scrnshot.h"
#include "layer0.h"
#include "layer1.h"
#include "layer2.h"
#include "layer3.h"
#include "version.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Namespaces                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Konstanten                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Variablen                                    */
/*============================================================================*/

/*!
All global data of the appüliction is stored in this structure
*/
appstate_t g_tState;

/*!
Table to describe all basic properties of valid video-/screenmodes of the
Spectrum Next
*/
const screenmode_t g_tScreenModes[] =
{
  /* --- LAYER 0 ----------------------------------------------------------------------- */
  {0x00, 256, 192,  16,  32,  24,   2, {0x4000, 0x1800}, {0x5800, 0x0300}}, /* Layer 0   */
  /* --- LAYER 1 ----------------------------------------------------------------------- */
  {0x10, 128,  96, 256,   0,   0,   0, {0x4000, 0x1800}, {0x6000, 0x1800}}, /* Layer 1,0 */
  {0x11, 256, 192,  16,  32,  24,   2, {0x4000, 0x1800}, {0x5800, 0x0300}}, /* Layer 1,1 */
  {0x12, 512, 192,   2,   0,   0,   0, {0x4000, 0x1800}, {0x6000, 0x1800}}, /* Layer 1,2 */
  {0x13, 256, 192,  16,  32, 192,   2, {0x4000, 0x1800}, {0x6000, 0x1800}}, /* Layer 1,3 */
  /* --- LAYER 2 ----------------------------------------------------------------------- */
  {0x20, 256, 192, 256,   0,   0,   0, {0x4000, 0x2000}, {0x0000, 0x0000}}, /* Layer 2   */
  {0x22, 320, 256, 256,   0,   0,   0, {0x4000, 0x2000}, {0x0000, 0x0000}}, /* Layer 2,2 */
  {0x23, 640, 256,  16,   0,   0,   0, {0x4000, 0x2000}, {0x0000, 0x0000}}, /* Layer 2,3 */
  /* --- LAYER 3 ----------------------------------------------------------------------- */
  {0x30, 320, 256, 256,  40,  32,   2}, /* Layer 3,0 */
  {0x31, 640, 256, 256,  80,  32,   2}, /* Layer 3,1 */
  {0x32, 320, 256, 256,  40,  32,  16}, /* Layer 3,2 */
  {0x33, 320, 256, 256,  80,  32,  16}, /* Layer 3,3 */
  /* --- END-OF-LIST ------------------------------------------------------------------- */
  {0xFF,   0,   0,   0,   0,   0,   0, {0x0000, 0x0000}, {0x0000, 0x0000}} 
};

/*!
BMP color palette including all spectrum layer 0 colors.
(https://en.wikipedia.org/wiki/ZX_Spectrum_graphic_modes)
(https://lospec.com/palette-list/zx-spectrum)
(read out from NREGs in LAYER0, LAYER 1,1)
*/
const bmppaletteentry_t g_tColorPalL0[] =
{
  /*--------------------------------------------*/
  /* B     G     R     A                        */
  /*--------------------------------------------*/
#if 0
  {0x00, 0x00, 0x00, 0x00}, /*  0 black         */
  {0xCE, 0x00, 0x01, 0x00}, /*  1 blue          */
  {0x00, 0x01, 0xCF, 0x00}, /*  2 red           */
  {0xCE, 0x01, 0xCF, 0x00}, /*  3 magenta       */
  {0x15, 0xCF, 0x00, 0x00}, /*  4 green         */
  {0xCF, 0xCF, 0x01, 0x00}, /*  5 cyan          */
  {0x15, 0xCF, 0xCF, 0x00}, /*  6 yellow        */
  {0xCF, 0xCF, 0xCF, 0x00}, /*  7 white/grey    */
  {0x00, 0x00, 0x00, 0x00}, /*  8 bright black  */
  {0xFD, 0x00, 0x02, 0x00}, /*  9 bright blue   */
  {0x01, 0x02, 0xFF, 0x00}, /* 10 bright red    */
  {0xFD, 0x02, 0xFF, 0x00}, /* 11 bright magenta*/
  {0x1C, 0xFF, 0x00, 0x00}, /* 12 bright green  */
  {0xFF, 0xFF, 0x02, 0x00}, /* 13 bright cyan   */
  {0x1D, 0xFF, 0xFF, 0x00}, /* 14 bright yellow */
  {0xFF, 0xFF, 0xFF, 0x00}, /* 15 bright white  */
  /*--------------------------------------------*/
#endif
#if 0
  {0x00, 0x00, 0x00, 0x00}, /*  0 black         */
  {0xD8, 0x00, 0x00, 0x00}, /*  1 blue          */
  {0x00, 0x00, 0xD8, 0x00}, /*  2 red           */
  {0xD8, 0x00, 0xD8, 0x00}, /*  3 magenta       */
  {0x00, 0xD8, 0x00, 0x00}, /*  4 green         */
  {0xD8, 0xD8, 0x00, 0x00}, /*  5 cyan          */
  {0x00, 0xD8, 0xD8, 0x00}, /*  6 yellow        */
  {0xD8, 0xD8, 0xD8, 0x00}, /*  7 white/grey    */
  {0x00, 0x00, 0x00, 0x00}, /*  8 bright black  */
  {0xFF, 0x00, 0x00, 0x00}, /*  9 bright blue   */
  {0x00, 0x00, 0xFF, 0x00}, /* 10 bright red    */
  {0xFF, 0x00, 0xFF, 0x00}, /* 11 bright magenta*/
  {0x00, 0xFF, 0x00, 0x00}, /* 12 bright green  */
  {0xFF, 0xFF, 0x00, 0x00}, /* 13 bright cyan   */
  {0x00, 0xFF, 0xFF, 0x00}, /* 14 bright yellow */
  {0xFF, 0xFF, 0xFF, 0x00}, /* 15 bright white  */
  /*--------------------------------------------*/
#endif
  {0x00, 0x00, 0x00, 0x00}, /*  0 black         */
  {0xB6, 0x00, 0x00, 0x00}, /*  1 blue          */
  {0x00, 0x00, 0xB6, 0x00}, /*  2 red           */
  {0xB6, 0x00, 0xB6, 0x00}, /*  3 magenta       */
  {0x00, 0xB6, 0x00, 0x00}, /*  4 green         */
  {0xB6, 0xB6, 0x00, 0x00}, /*  5 cyan          */
  {0x00, 0xB6, 0xB6, 0x00}, /*  6 yellow        */
  {0xB6, 0xB6, 0xB6, 0x00}, /*  7 white/grey    */
  {0x00, 0x00, 0x00, 0x00}, /*  8 bright black  */
  {0xFF, 0x00, 0x00, 0x00}, /*  9 bright blue   */
  {0x00, 0x00, 0xFF, 0x00}, /* 10 bright red    */
  {0xFF, 0x24, 0xFF, 0x00}, /* 11 bright magenta*/
  {0x00, 0xFF, 0x00, 0x00}, /* 12 bright green  */
  {0xFF, 0xFF, 0x00, 0x00}, /* 13 bright cyan   */
  {0x00, 0xFF, 0xFF, 0x00}, /* 14 bright yellow */
  {0xFF, 0xFF, 0xFF, 0x00}  /* 15 bright white  */
  /*--------------------------------------------*/
};

/*============================================================================*/
/*                               Strukturen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Typ-Definitionen                             */
/*============================================================================*/

/*============================================================================*/
/*                               Prototypen                                   */
/*============================================================================*/

/*!
Diese Funktion wird einmalig beim Start der Anwendung zur Initialisierung aller
benoetigten Ressourcen aufgerufen.
*/
void _construct(void);

/*!
Diese Funktion wird einmalig beim Beenden der Anwendung zur Freigabe aller
belegten Ressourcen aufgerufen.
*/
void _destruct(void);

/*!
Diese Funktion interpretiert alle Argumente, die der Anwendung uebergeben
wurden.
*/
int parseArguments(int argc, char_t* argv[]);

/*!
Ausgabe der Hilfe dieser Anwendung.
*/
int showHelp(void);

/*!
Ausgabe der Versionsinformation dieser Anwendung.
*/
int showInfo(void);

/*!
This function is responsible for creation of the BMP file. It calls
specialized functions for the active video-/screenmode.
*/
int makeScreenshot(void);

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* _construct()                                                               */
/*----------------------------------------------------------------------------*/
void _construct(void)
{
  if (!g_tState.bInitialized)
  {
    g_tState.eAction       = ACTION_NONE;
    g_tState.bQuiet        = false;
    g_tState.bForce        = false;
    g_tState.iExitCode     = EOK;
    g_tState.uiCpuSpeed    = zxn_getspeed();
    g_tState.bmpfile.hFile = INV_FILE_HND;

    esx_f_getcwd(g_tState.bmpfile.acPathName);

    memset(&g_tState.bmpfile.tFileHdr, 0, sizeof(g_tState.bmpfile.tFileHdr));
    memset(&g_tState.bmpfile.tInfoHdr, 0, sizeof(g_tState.bmpfile.tInfoHdr));

    zxn_setspeed(RTM_28MHZ);

    g_tState.bInitialized  = true;
  }
}


/*----------------------------------------------------------------------------*/
/* _destruct()                                                                */
/*----------------------------------------------------------------------------*/
void _destruct(void)
{
  if (g_tState.bInitialized)
  {
    if (INV_FILE_HND != g_tState.bmpfile.hFile)
    {
      (void) esx_f_close(g_tState.bmpfile.hFile);
      g_tState.bmpfile.hFile = INV_FILE_HND;
    }

    zxn_setspeed(g_tState.uiCpuSpeed);
    g_tState.bInitialized = false;
  }
}


/*----------------------------------------------------------------------------*/
/* main()                                                                     */
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
  _construct();
  atexit(_destruct);

  if (EOK == (g_tState.iExitCode = parseArguments(argc, argv)))
  {
    switch (g_tState.eAction)
    {
      case ACTION_NONE:
        g_tState.iExitCode = EOK;
        break;

      case ACTION_HELP:
        g_tState.iExitCode = showHelp();
        break;

      case ACTION_INFO:
        g_tState.iExitCode = showInfo();
        break;

      case ACTION_SHOT:
        g_tState.iExitCode = makeScreenshot();
        break;

      default:
        g_tState.iExitCode = ESTAT;
    }
  }

  return (int) (EOK == g_tState.iExitCode ? 0 : zxn_strerror(g_tState.iExitCode));
}


/*----------------------------------------------------------------------------*/
/* parseArguments()                                                           */
/*----------------------------------------------------------------------------*/
int parseArguments(int argc, char_t* argv[])
{
  int iReturn = EOK;

  g_tState.eAction = ACTION_NONE;

  int i = 1;

  while (i < argc)
  {
    const char_t* acArg = argv[i];

    if ('-' == acArg[0])
    {
      if ((0 == stricmp(acArg, "-h")) /* || (0 == stricmp(acArg, "--help")) */)
      {
        g_tState.eAction = ACTION_HELP;
      }
      else if ((0 == strcmp(acArg, "-v")) /* || (0 == stricmp(acArg, "--version")) */)
      {
        g_tState.eAction = ACTION_INFO;
      }
      else if ((0 == strcmp(acArg, "-q")) /* || (0 == stricmp(acArg, "--quiet")) */)
      {
        g_tState.bQuiet = true;
      }
      else if ((0 == strcmp(acArg, "-f")) /* || (0 == stricmp(acArg, "--force")) */)
      {
        g_tState.bForce = true;
      }
#if 0
      else if ((0 == strcmp(acArg, "-p")) /* || (0 == stricmp(acArg, "--palette")) */)
      {
        if ((i + 1) < argc)
        {
          uint8_t uiPalette = strtoul(argv[i + 1], 0, 0);
          g_tState.uiPalette = constrain(uiPalette, 0, PALETTES - 1);
          ++i;
        }
      }
#endif
      else
      {
        fprintf(stderr, "unknown option: %s\n", acArg);
        iReturn = EINVAL;
        break;
      }
    }
    else
    {
      snprintf(g_tState.bmpfile.acPathName, sizeof(g_tState.bmpfile.acPathName), "%s", acArg);
    }

    ++i;
  }

  if (ACTION_NONE == g_tState.eAction)
  {
    g_tState.eAction = ACTION_SHOT;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* showHelp()                                                                 */
/*----------------------------------------------------------------------------*/
int showHelp(void)
{
  char_t acAppName[0x10];
  strncpy(acAppName, VER_INTERNALNAME_STR, sizeof(acAppName));
  strupr(acAppName);

  printf("%s\n\n", VER_FILEDESCRIPTION_STR);

  printf("%s file [-f][-q][-h][-v]\n\n", acAppName);
  /*      0.........1.........2.........3. */
  printf(" file        pathname of file\n");
  printf(" -f[orce]    force overwrite\n");
  printf(" -q[uiet]    print no messages\n");
  printf(" -h[elp]     print this help\n");
  printf(" -v[ersion]  print version info\n");

  return EOK;
}


/*----------------------------------------------------------------------------*/
/* showInfo()                                                                 */
/*----------------------------------------------------------------------------*/
int showInfo(void)
{
  uint16_t uiVersion;
  char_t acBuffer[0x10];

  strncpy(acBuffer, VER_INTERNALNAME_STR, sizeof(acBuffer));
  strupr(acBuffer);
  printf("%s " VER_LEGALCOPYRIGHT_STR "\n", acBuffer);

  if (ESX_DOSVERSION_NEXTOS_48K != (uiVersion = esx_m_dosversion()))
  {
    snprintf(acBuffer, sizeof(acBuffer), "NextOS %u.%02u",
             ESX_DOSVERSION_NEXTOS_MAJOR(uiVersion),
             ESX_DOSVERSION_NEXTOS_MINOR(uiVersion));
  }
  else
  {
    strncpy(acBuffer, "48K mode", sizeof(acBuffer));
  }

  /*      0.........1.........2.........3. */
  printf(" Version %s (%s)\n", VER_FILEVERSION_STR, acBuffer);
  printf(" Stefan Zell (info@diezells.de)\n");

  return EOK;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot()                                                           */
/*----------------------------------------------------------------------------*/
int makeScreenshot(void)
{
  int iReturn = EOK;

  uint8_t uiMode = detectScreenMode();
  const screenmode_t* pInfo = getScreenModeInfo(uiMode);

  if (EOK == iReturn)
  {
    /* Is argument a directory ? */
    if (INV_FILE_HND != (g_tState.bmpfile.hFile = esx_f_opendir(g_tState.bmpfile.acPathName)))
    {
      uint16_t uiIndex = 0;
      char_t acPathName[ESX_PATHNAME_MAX];

      esx_f_closedir(g_tState.bmpfile.hFile);
      g_tState.bmpfile.hFile = INV_FILE_HND;

      while (uiIndex < 0xFFFF)
      {
        snprintf(acPathName, sizeof(acPathName),
                 "%s" ESX_DIR_SEP VER_INTERNALNAME_STR "-%u.bmp",
                 g_tState.bmpfile.acPathName,
                 uiIndex);

        if (INV_FILE_HND == (g_tState.bmpfile.hFile = esx_f_open(acPathName, ESXDOS_MODE_R | ESXDOS_MODE_OE)))
        {
          snprintf(g_tState.bmpfile.acPathName, sizeof(g_tState.bmpfile.acPathName), "%s", acPathName);
          break;  /* filename found */
        }
        else
        {
          esx_f_close(g_tState.bmpfile.hFile);
          g_tState.bmpfile.hFile = INV_FILE_HND;
        }

        ++uiIndex;
      }

      if (0xFFFF == uiIndex)
      {
        iReturn = ERANGE; /* Error */
      }
    }
    else /* Argument is a file ... */
    {
      g_tState.bmpfile.hFile = esx_f_open(g_tState.bmpfile.acPathName, ESXDOS_MODE_R | ESXDOS_MODE_OE);

      if (INV_FILE_HND != g_tState.bmpfile.hFile)
      {
        esx_f_close(g_tState.bmpfile.hFile);
        g_tState.bmpfile.hFile = INV_FILE_HND;

        if (g_tState.bForce)
        {
          esx_f_unlink(g_tState.bmpfile.acPathName);
        }
        else
        {
          iReturn = EBADF; /* Error: File exists */
        }
      }
    }
  }

  if (EOK == iReturn)
  {
    g_tState.bmpfile.hFile = esx_f_open(g_tState.bmpfile.acPathName, ESXDOS_MODE_W | ESXDOS_MODE_CN);

    if (INV_FILE_HND == g_tState.bmpfile.hFile)
    {
      iReturn = EACCES;
    }
  }

  if (EOK == iReturn)
  {
    /* Prepare BMP file header */
    g_tState.bmpfile.tFileHdr.uiType         = 0x4D42;                            /* 'BM' (LE!)      */
    g_tState.bmpfile.tFileHdr.uiSize         = sizeof(g_tState.bmpfile.tFileHdr); /* file size       */
    g_tState.bmpfile.tFileHdr.uiSize        += sizeof(g_tState.bmpfile.tInfoHdr);
    g_tState.bmpfile.tFileHdr.uiRes          = 0;                                 /* reserved        */
    g_tState.bmpfile.tFileHdr.uiOffBits      = sizeof(g_tState.bmpfile.tFileHdr); /* offset bits     */
    g_tState.bmpfile.tFileHdr.uiOffBits     += sizeof(g_tState.bmpfile.tInfoHdr);

    /* Prepare BMP info header  */
    g_tState.bmpfile.tInfoHdr.uiSize         = sizeof(g_tState.bmpfile.tInfoHdr); /* header size     */
    g_tState.bmpfile.tInfoHdr.uiPlanes       = 1;                                 /* only one layer  */
    g_tState.bmpfile.tInfoHdr.uiCompression  = 0;                                 /* BI_RGB          */
    g_tState.bmpfile.tInfoHdr.iXPelsPerMeter = BMP_DPI_72;                        /* 72 DPI          */
    g_tState.bmpfile.tInfoHdr.iYPelsPerMeter = BMP_DPI_72;                        /* 72 DPI          */
    g_tState.bmpfile.tInfoHdr.uiClrImportant = 0;                                 /* all colors used */
  }

  if (EOK == iReturn)
  {
    switch (uiMode)
    {
      /* LAYER 0 */
      case 0x00: 
        iReturn = makeScreenshot_L00(pInfo);
        break;

      /* LAYER 1 */
      case 0x10:
        iReturn = makeScreenshot_L10(pInfo);
        break;
      case 0x11:
        iReturn = makeScreenshot_L11(pInfo);
        break;
      case 0x12:
        iReturn = makeScreenshot_L12(pInfo);
        break;
      case 0x13:
        iReturn = makeScreenshot_L13(pInfo);
        break;

      /* LAYER 2 */
      case 0x20:
        iReturn = makeScreenshot_L20(pInfo);
        break;
      case 0x22:
        iReturn = makeScreenshot_L22(pInfo);
        break;
      case 0x23:
        iReturn = makeScreenshot_L23(pInfo);
        break;

      /* LAYER 3 */
      case 0x30:
        iReturn = makeScreenshot_L30(pInfo);
        break;
      case 0x31:
        iReturn = makeScreenshot_L31(pInfo);
        break;
      case 0x32:
        iReturn = makeScreenshot_L32(pInfo);
        break;
      case 0x33:
        iReturn = makeScreenshot_L33(pInfo);
        break;

      default:
        iReturn = ENOTSUP; /* Error */
    }
  }

  /* Close file */
  if (INV_FILE_HND != g_tState.bmpfile.hFile)
  {
    (void) esx_f_close(g_tState.bmpfile.hFile);
    g_tState.bmpfile.hFile = INV_FILE_HND;
  }

  if (EOK != iReturn)
  {
    /* Remove file */
    (void) esx_f_unlink(g_tState.bmpfile.acPathName);
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*  saveImageHeader()                                                         */
/*----------------------------------------------------------------------------*/
int saveImageHeader(void)
{
  int iReturn = EOK;

  if (INV_FILE_HND != g_tState.bmpfile.hFile)
  {
    /* Save BMP file header */
    if (EOK == iReturn)
    {
      if (sizeof(g_tState.bmpfile.tFileHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tFileHdr, sizeof(g_tState.bmpfile.tFileHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* Save BMP info header */
    if (EOK == iReturn)
    {
      if (sizeof(g_tState.bmpfile.tInfoHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tInfoHdr, sizeof(g_tState.bmpfile.tInfoHdr)))
      {
        iReturn = EBADF;
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
/* saveColourPalette()                                                        */
/*----------------------------------------------------------------------------*/
int saveColourPalette(const screenmode_t* pInfo)
{
  int iReturn = EINVAL;

  if ((0 != pInfo) && (INV_FILE_HND != g_tState.bmpfile.hFile))
  {
    bmppaletteentry_t tEntry = {.a = 0x00};
    uint16_t uiValue;
    uint8_t  uiPalIdx;
    uint8_t  uiPalCtl;
    uint8_t  uiPalAct;
    uint16_t uiColors = pInfo->uiColors;

    /* Only LAYER 1,0: Detect number of colours */
    if ((0x10 == pInfo->uiMode) && zxn_radastan_mode())
    {
      uiColors = 16;
    }

    /* Status sichern, um nichts zu verstellen */
    uiPalIdx = ZXN_READ_REG(REG_PALETTE_INDEX  );
    uiPalCtl = ZXN_READ_REG(REG_PALETTE_CONTROL);

    /* Detect active palette */
    switch ((pInfo->uiMode >> 4) & 0x0F)
    {
      case 0:
      case 1: /* 1.Pal 000, 2.Pal 100 */
        uiPalAct = (uiPalCtl >> 1) & 0x01;
        uiValue  = (uiPalCtl & 0x8F) | ((uiPalAct ? 0x04 : 0x00) << 4);
        break;

      case 2: /* 1. 001, 2. 101 */
        uiPalAct = (uiPalCtl >> 2) & 0x01;
        uiValue  = (uiPalCtl & 0x8F) | ((uiPalAct ? 0x05 : 0x01) << 4);
        break;

      default:
        uiValue  = uiPalCtl;
    }

    /* Select active palette */
    ZXN_WRITE_REG(REG_PALETTE_CONTROL, uiValue);

    /* Write palette entries */
    for (uint16_t i = 0; i < uiColors; ++i)
    {
      /* Palettenindex auswaehlen */
      ZXN_WRITE_REG(REG_PALETTE_INDEX, i);

      /* Aktuellen Farbwert lesen:
      0x41 liefert RRR GGG BB.
      0x44 liefert ... ... ..B
      */
      uiValue  = ((uint16_t) ZXN_READ_REG(REG_PALETTE_VALUE_8 )) << 1;
      uiValue |= ((uint16_t) ZXN_READ_REG(REG_PALETTE_VALUE_16)) & 0x01;

      tEntry.b = rgb3_to_rgb8( uiValue       & 0x07);
      tEntry.g = rgb3_to_rgb8((uiValue >> 3) & 0x07);
      tEntry.r = rgb3_to_rgb8((uiValue >> 6) & 0x07);

      if (sizeof(tEntry) != esx_f_write(g_tState.bmpfile.hFile, &tEntry, sizeof(tEntry)))
      {
        ZXN_WRITE_REG(REG_PALETTE_INDEX,   uiPalIdx);
        ZXN_WRITE_REG(REG_PALETTE_CONTROL, uiPalCtl);
        iReturn = EBADF;
        goto EXIT_PALETTE_LOOP;
      }
    }

    /* Registerzustand wiederherstellen */
    ZXN_WRITE_REG(REG_PALETTE_INDEX,   uiPalIdx);
    ZXN_WRITE_REG(REG_PALETTE_CONTROL, uiPalCtl);

    iReturn = EOK;
  }

EXIT_PALETTE_LOOP:  

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* detectScreenMode()                                                         */
/*----------------------------------------------------------------------------*/
uint8_t detectScreenMode(void)
{
  struct esx_mode tMode;
  memset(&tMode, 0, sizeof(tMode));

  if (0 == esx_ide_mode_get(&tMode))
  {
    return ((tMode.mode8.layer & 0x0F) << 4) | (tMode.mode8.submode & 0x0F);
  }

  return 0xFF;
}


/*----------------------------------------------------------------------------*/
/* getScreenModeInfo()                                                        */
/*----------------------------------------------------------------------------*/
const screenmode_t* getScreenModeInfo(uint8_t uiMode)
{
  register const screenmode_t* pReturn = &g_tScreenModes[0];

  while (0xFF != pReturn->uiMode)
  {
    if (uiMode == pReturn->uiMode)
    {
      break;
    }

    ++pReturn;
  }

  return pReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
