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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include <z80.h>
#include <intrinsic.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "scrnshot.h"
#include "version.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
/*!
With this macro it can be controlled wether the pixel calculations should be
done by the functions from "zxn.h" or not ...
  - 0 = calculate pixeladdress the hard way (bit-fiddling in C)
  - 1 = calculate pixeladdress with function from z88dk-newlib
  - 2 = calculate pixeladdress with PIXELAD opcode
  - 3 = calculate pixeladdress with PIXELAD and bit-fiddling
*/
#define _PIXEL_CALC_ 3

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
In dieser Struktur werden alle globalen Daten der Anwendung gespeichert.
*/
static struct _state
{
  /*!
  If this flag is set, then this structure is initialized
  */
  bool bInitialized;

  /*!
  Action to execute (help, version, sreenshot, ...)
  */
  action_t eAction;

  /*!
  If this flag is set, no messages are printed to the console while creating a
  screenshot.
  */
  bool bQuiet;

  /*!
  If this flag is set, existing output files are overwritten
  */
  bool bForce;

  /*!
  Backup: Current speed of Z80
  */
  uint8_t uiCpuSpeed;

  /*!
  Exitcode of the application, that is handovered to BASIC
  */
  int iExitCode;

  /*!
  Structure of all information of the BMP output file
  */
  struct
  {
    /*!
    Pathname of the output file
    */
    char acPathName[ESX_PATHNAME_MAX];

    /*!
    Handle of the open BMP file while writing
    */
    uint8_t hFile;

    /*!
    File header of the BMP file
    */
    bmpfileheader_t tFileHdr;

    /*!
    Info header of the BMO file
    */
    bmpinfoheader_t tInfoHdr;
  } bmpfile;

} g_tState;

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
  {0xFF,   0,   0,   0,   0,   0,   0} 
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

/* TODO: Read out L0:palette from HW */

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
int parseArguments(int argc, char* argv[]);

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

/*!
Reading the video memory and transcode it to BMP data for LAYER 0
*/
static int makeScreenshot_L00(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 1,0
*/
static int makeScreenshot_L10(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 1,1
*/
static int makeScreenshot_L11(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 1,2
*/
static int makeScreenshot_L12(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 1,3
*/
static int makeScreenshot_L13(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 2
*/
static int makeScreenshot_L20(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 2,2
*/
static int makeScreenshot_L22(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 2,3
*/
static int makeScreenshot_L23(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,0
*/
static int makeScreenshot_L30(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,1
*/
static int makeScreenshot_L31(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,2
*/
static int makeScreenshot_L32(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,3
*/
static int makeScreenshot_L33(const screenmode_t* pInfo);

/*!
This function saves the predefined BMP header to the already opened file
@return "EOK" = no error
*/
static int saveImageHeader(void);

/*!
This function reads the current colour palette from NREGs and saves it to the
already opened BMP file.
@return "EOK" = no error
*/
static int saveColourPalette(const screenmode_t* pInfo);

/*!
This function tries to detect the current screen mode (layer 0, layer 1, ...)
*/
uint8_t detectScreenMode(void);

/*!
This function returns a pointer to a structure of prperties of the given
video-/screenmode
*/
const screenmode_t* getScreenModeInfo(uint8_t uiMode);

/*!
Convert a RGB3 value to a corresponding RGB8 value
3-Bit (0..7) -> 8-Bit (0..255): "bit replicate"
*/
static inline uint8_t rgb3_to_8(uint8_t v)
{
  return (uint8_t)((v << 5) | (v << 2) | (v >> 1));  // 0,36,73,109,146,182,219,255
}

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
  g_tState.eAction       = ACTION_NONE;
  g_tState.bQuiet        = false;
  g_tState.bForce        = false;
  g_tState.iExitCode     = EOK;
  g_tState.uiCpuSpeed    = ZXN_READ_REG(REG_TURBO_MODE) & 0x03;
  g_tState.bmpfile.hFile = INV_FILE_HND;

  esx_f_getcwd(g_tState.bmpfile.acPathName);

  memset(&g_tState.bmpfile.tFileHdr, sizeof(g_tState.bmpfile.tFileHdr), 0);
  memset(&g_tState.bmpfile.tInfoHdr, sizeof(g_tState.bmpfile.tInfoHdr), 0);

  ZXN_NEXTREG(REG_TURBO_MODE, RTM_28MHZ);

  g_tState.bInitialized  = true;
}


/*----------------------------------------------------------------------------*/
/* _destruct()                                                                */
/*----------------------------------------------------------------------------*/
void _destruct(void)
{
  if (g_tState.bInitialized)
  {
    ZXN_NEXTREGA(REG_TURBO_MODE, g_tState.uiCpuSpeed);
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
int parseArguments(int argc, char* argv[])
{
  int iReturn = EOK;

  g_tState.eAction = ACTION_NONE;

  int i = 1;

  while (i < argc)
  {
    const char* acArg = argv[i];

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
  printf("%s\n\n", VER_FILEDESCRIPTION_STR);

  printf("%s file [-f][-q][-h][-v]\n\n", strupr(VER_INTERNALNAME_STR));
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
  printf("%s " VER_LEGALCOPYRIGHT_STR "\n", strupr(VER_INTERNALNAME_STR));
  /*      0.........1.........2.........3. */
  printf(" Version %s\n", VER_FILEVERSION_STR);
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
      char acPathName[ESX_PATHNAME_MAX];

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
    esx_f_close(g_tState.bmpfile.hFile);
    g_tState.bmpfile.hFile = INV_FILE_HND;
  }

  if (EOK != iReturn)
  {
    /* Remove file */
    esx_f_unlink(g_tState.bmpfile.acPathName);
  }

  return iReturn;
}


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
/* makeScreenshot_L30()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L30(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L31()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L31(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L32()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L32(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L33()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L33(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/*  saveImageHeader()                                                         */
/*----------------------------------------------------------------------------*/
static int saveImageHeader(void)
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
static int saveColourPalette(const screenmode_t* pInfo)
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

      tEntry.b = rgb3_to_8( uiValue       & 0x07);
      tEntry.g = rgb3_to_8((uiValue >> 3) & 0x07);
      tEntry.r = rgb3_to_8((uiValue >> 6) & 0x07);

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
  memset(&tMode, sizeof(tMode), 0);

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
