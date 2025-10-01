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
| Screenshot tools for ZX Spectrum Next (dot command)                          |
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
Marker for the list of valid error messages, that can be returned to BASIC.
*/
#define END_OF_LIST (0x7FFF)

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
  Index of the palette that will be used to create the BMP
  */
  uint8_t uiPalette;

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
  /* --- LAYER 0 -------------------------------- */
  {0x00, 256, 192,  16,  32,  24,   2, {0x4000, 0x1800}, {0x5800, 0x0300}}, /* Layer 0 */
  /* --- LAYER 1 -------------------------------- */
  {0x10, 128,  96, 256,   0,   0,   0}, /* Layer 1,0 */
  {0x11, 256, 192, 256,  32,  24,   2, {0x4000, 0x1800}, {0x5800, 0x0300}}, /* Layer 1,1 */
  {0x12, 512, 192, 256,   0,   0,   0}, /* Layer 1,2 */
  {0x13, 256, 192, 256,  32, 192,   2}, /* Layer 1,3 */
  /* --- LAYER 2 -------------------------------- */
  {0x20, 256, 192, 256,   0,   0,   0, {0x4000, 0x2000}, {0x0000, 0x0000}}, /* Layer 2   */
  {0x22, 320, 256, 256,   0,   0,   0, {0x4000, 0x2000}, {0x0000, 0x0000}}, /* Layer 2,2 */
  {0x23, 640, 256,  16,   0,   0,   0, {0x4000, 0x2000}, {0x0000, 0x0000}}, /* Layer 2,3 */
  /* --- LAYER 3 -------------------------------- */
  {0x30, 320, 256, 256,  40,  32,   2}, /* Layer 3,0 */
  {0x31, 640, 256, 256,  80,  32,   2}, /* Layer 3,1 */
  {0x32, 320, 256, 256,  40,  32,  16}, /* Layer 3,2 */
  {0x33, 320, 256, 256,  80,  32,  16}, /* Layer 3,3 */
  /* --- END-OF-LIST ---------------------------- */
  {0xFF,   0,   0,   0,   0,   0,   0} 
};

/*!
BMP color palette including all spectrum layer 0 colors.
(https://en.wikipedia.org/wiki/ZX_Spectrum_graphic_modes)
(https://lospec.com/palette-list/zx-spectrum)
*/
const bmppaletteentry_t g_tColorPalL0[] =
{
  /*--------------------------------------------*/
  /* B     G     R     A                        */
  /*--------------------------------------------*/
  {0x00, 0x00, 0x00, 0x00},   /* black          */
  {0xCE, 0x00, 0x01, 0x00},   /* blue           */
  {0x00, 0x01, 0xCF, 0x00},   /* red            */
  {0xCE, 0x01, 0xCF, 0x00},   /* magenta        */
  {0x15, 0xCF, 0x00, 0x00},   /* green          */
  {0xCF, 0xCF, 0x01, 0x00},   /* cyan           */
  {0x15, 0xCF, 0xCF, 0x00},   /* yellow         */
  {0xCF, 0xCF, 0xCF, 0x00},   /* white/grey     */
  {0x00, 0x00, 0x00, 0x00},   /* bright black   */
  {0xFD, 0x00, 0x02, 0x00},   /* bright blue    */
  {0x01, 0x02, 0xFF, 0x00},   /* bright red     */
  {0xFD, 0x02, 0xFF, 0x00},   /* bright magenta */
  {0x1C, 0xFF, 0x00, 0x00},   /* bright green   */
  {0xFF, 0xFF, 0x02, 0x00},   /* bright cyan    */
  {0x1D, 0xFF, 0xFF, 0x00},   /* bright yellow  */
  {0xFF, 0xFF, 0xFF, 0x00},   /* bright white   */
  /*--------------------------------------------*/
  {0x00, 0x00, 0x00, 0x00},   /* black          */
  {0xD8, 0x00, 0x00, 0x00},   /* blue           */
  {0x00, 0x00, 0xD8, 0x00},   /* red            */
  {0xD8, 0x00, 0xD8, 0x00},   /* magenta        */
  {0x00, 0xD8, 0x00, 0x00},   /* green          */
  {0xD8, 0xD8, 0x00, 0x00},   /* cyan           */
  {0x00, 0xD8, 0xD8, 0x00},   /* yellow         */
  {0xD8, 0xD8, 0xD8, 0x00},   /* white/grey     */
  {0x00, 0x00, 0x00, 0x00},   /* bright black   */
  {0xFF, 0x00, 0x00, 0x00},   /* bright blue    */
  {0x00, 0x00, 0xFF, 0x00},   /* bright red     */
  {0xFF, 0x00, 0xFF, 0x00},   /* bright magenta */
  {0x00, 0xFF, 0x00, 0x00},   /* bright green   */
  {0xFF, 0xFF, 0x00, 0x00},   /* bright cyan    */
  {0x00, 0xFF, 0xFF, 0x00},   /* bright yellow  */
  {0xFF, 0xFF, 0xFF, 0x00},   /* bright white   */
  /*--------------------------------------------*/
};
#define PALETTES (sizeof(g_tColorPalL0) / sizeof(bmppaletteentry_t) / 16)

/*!
Table to define all textual error messages that are returned to NextOS/BASIC
*/
const errentry_t g_tErrTable[] =
{
  {EOK,         "no error"                      "\xA0"},
  {EACCES,      "access denied"                 "\xA0"},
  {EBADF,       "bad file"                      "\xA0"},
  {EBDFD,       "bad file descriptor"           "\xA0"},
  {EDOM,        "out of domain of function"     "\xA0"},
  {EFBIG,       "file too large"                "\xA0"},
  {EINVAL,      "invalid value"                 "\xA0"},
  {EMFILE,      "too many open files"           "\xA0"},
  {ENFILE,      "too many open files in system" "\xA0"},
  {ENOLCK,      "no locks available"            "\xA0"},
  {ENOMEM,      "out of mem"                    "\xA0"},
  {ENOTSUP,     "not supported"                 "\xA0"},
  {EOVERFLOW,   "overflow"                      "\xA0"},
  {ERANGE,      "out of range"                  "\xA0"},
  {ESTAT,       "bad state"                     "\xA0"},
  {EAGAIN,      "resource temp. unavailable"    "\xA0"},
  {EWOULDBLOCK, "operation would block"         "\xA0"},
  /* ---------------- APPLICATOPN SPECIFIC ---------- */
  {EBREAK,      "D BREAK - no repeat"           "\xA0"},
  {ETIMEOUT,    "timeout error"                 "\xA0"},
  /* ---------------- END-OF-LIST ------------------- */
  {END_OF_LIST, "unknown error"                 "\xA0"}
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
This function tries to detect the current screen mode (layer 0, layer 1, ...)
*/
uint8_t detectScreenMode(void);

/*!
This function returns a pointer to a structure of prperties of the given
video-/screenmode
*/
const screenmode_t* getScreenModeInfo(uint8_t uiMode);

/*!
This function is used to map a physical memory address to a void pointer.
*/
void* memmap(uint16_t uiPhysAddr);

/*!
This function returns a pointer to a textual error message for the given
error code
*/
const unsigned char* _strerror(int iCode);

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
  g_tState.uiPalette     = 0;
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

  return (int) (EOK == g_tState.iExitCode ? 0 : _strerror(g_tState.iExitCode));
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
      else if ((0 == strcmp(acArg, "-p")) /* || (0 == stricmp(acArg, "--palette")) */)
      {
        if ((i + 1) < argc)
        {
          uint8_t uiPalette = strtoul(argv[i + 1], 0, 0);
          g_tState.uiPalette = constrain(uiPalette, 0, PALETTES - 1);
          ++i;
        }
      }
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
   #if 1
    g_tState.eAction = ACTION_SHOT;
   #else
    if (0 < strnlen(g_tState.bmpfile.acPathName, sizeof(g_tState.bmpfile.acPathName)))
    {
      g_tState.eAction = ACTION_SHOT;
    }
    else if ('\0' == g_tState.bmpfile.acPathName[0])
    {
      g_tState.eAction = ACTION_SHOT;
    }
   #endif
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/* showHelp()                                                                 */
/*----------------------------------------------------------------------------*/
int showHelp(void)
{
  printf("%s\n\n", VER_FILEDESCRIPTION_STR);

  printf("%s file [-f][-p idx][-q][-h][-v]\n\n", strupr(VER_INTERNALNAME_STR));
  /*      0.........1.........2.........3. */
  printf(" file        pathname of file\n");
  printf(" -f[orce]    force overwrite\n");
  printf(" -p[alette]  index of palette\n");
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

 #ifdef __DEBUG__
  if (0 != pInfo)
  {
    printf("makeScreenshot() - mode: 0x%02X resolution: %dx%d colours: %u\n",
           uiMode,
           pInfo->uiResX,
           pInfo->uiResY,
           pInfo->uiColors);
  }
  else
  {
    printf("makeScreenshot() - Unable to detect screen mode\n");
  }
 #endif

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
      iReturn = EACCES; /* Error */
    }
  }

  if (EOK == iReturn)
  {
    /* prepare BMP file header */
    g_tState.bmpfile.tFileHdr.uiType      = 0x4D42;                             /* 'BM' (LE!) */
    g_tState.bmpfile.tFileHdr.uiSize      = sizeof(g_tState.bmpfile.tFileHdr);  /* file size  */
    g_tState.bmpfile.tFileHdr.uiSize     += sizeof(g_tState.bmpfile.tInfoHdr);
    g_tState.bmpfile.tFileHdr.uiRes       = 0;                                  /* reserved   */
    g_tState.bmpfile.tFileHdr.uiOffBits   = sizeof(g_tState.bmpfile.tFileHdr);  /* off bits   */
    g_tState.bmpfile.tFileHdr.uiOffBits  += sizeof(g_tState.bmpfile.tInfoHdr);

    /* prepare BMP info header  */
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

  if (EOK == iReturn)
  {
    /* Close file */
    esx_f_close(g_tState.bmpfile.hFile);
    g_tState.bmpfile.hFile = INV_FILE_HND;
  }
  else
  {
    /* Remove file */
    esx_f_close(g_tState.bmpfile.hFile);
    g_tState.bmpfile.hFile = INV_FILE_HND;
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
    uint16_t uiX, uiY, uiZ;
    uint16_t uiPalSize  = pInfo->uiColors * sizeof(bmppaletteentry_t);
    uint8_t  uiLineLen  = pInfo->uiResX >> 1;   /* 32bit aligned */
    uint32_t uiPxlSize  = ((uint32_t) pInfo->uiResY) * ((uint32_t) uiLineLen);
    uint8_t* pPixelData = (uint8_t*) memmap(pInfo->tMemPixel.uiAddr);
    uint8_t* pAttrData  = (uint8_t*) memmap(pInfo->tMemAttr.uiAddr);
    uint8_t* pPixelRow  = 0;
    uint8_t* pAttrRow   = 0;

    /* create file header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      if (sizeof(g_tState.bmpfile.tFileHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tFileHdr, sizeof(g_tState.bmpfile.tFileHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* create info header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;                         /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;                         /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 4;                                     /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = pInfo->uiResY * uiLineLen;             /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */

      if (sizeof(g_tState.bmpfile.tInfoHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tInfoHdr, sizeof(g_tState.bmpfile.tInfoHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* Save color palette ... */
    if (EOK == iReturn)
    {
      if (uiPalSize != esx_f_write(g_tState.bmpfile.hFile, &g_tColorPalL0[g_tState.uiPalette * 16], uiPalSize))
      {
        iReturn = EBADF;
      }
    }

    /* write pixel data ... */
    if (EOK == iReturn)
    {
      uint8_t* pBmpLine = 0;
      uint8_t  uiBmpIdx;
      uint8_t  uiBmpPixel;
      uint8_t  uiPixelByte;
      uint8_t  uiAttrByte;

      if (0 == (pBmpLine = malloc(uiLineLen)))
      {
        iReturn = ENOMEM;
      }
      else
      {
        memset(pBmpLine, 0, uiLineLen);

       #ifdef __DEBUG__
        printf("makeScreenshot_L0() - looping ...\n");
       #endif

        /* for (uiY = 0; uiY < pInfo->uiResY; ++uiY) */
        for (uiY = pInfo->uiResY - 1; uiY != 0xFFFF; --uiY)
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

         #ifdef __DEBUG__
          /* printf("-"); */
         #endif

          for (uiX = 0; uiX < pInfo->uiResX; uiX += 8)
          {
            uiPixelByte = *(pPixelRow + (uiX >> 3));
            uiAttrByte  = *(pAttrRow  + (uiX >> 3));

            /*
            Bit 7   FLASH   (0 = normal, 1 = blinkend Vorder-/Hintergrund wechseln)
            Bit 6   BRIGHT  (0 = normale Helligkeit, 1 = erhoehte Helligkeit)
            Bits 5–3 PAPER  (Hintergrundfarbe, 3 Bit: 0–7)
            Bits 2–0 INK    (Vordergrundfarbe, 3 Bit: 0–7)
            */

            for (uiZ = 0; uiZ < 8; ++uiZ)
            {
              uiBmpPixel  = (uiPixelByte & (1 << (7 - uiZ)) ?
                            (uiAttrByte & INK_WHITE) :
                            (uiAttrByte & PAPER_WHITE) >> 3);
              uiBmpPixel += (uiAttrByte & BRIGHT ? 8 : 0);

              /* TODO: Flashing */

              uiBmpIdx = (uiX + uiZ) >> 1;
              pBmpLine[uiBmpIdx] = (uiX + uiZ) & 0x01 ? /* odd nibble ? */
                                   (pBmpLine[uiBmpIdx] & 0xF0) | uiBmpPixel :
                                   (pBmpLine[uiBmpIdx] & 0x0F) | (uiBmpPixel << 4);
            }
          }

         #ifdef __DEBUG__
          /* printf("+"); */
         #endif

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
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L11()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L11(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L12()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L12(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L13()                                                       */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L13(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
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
    uint8_t* pPixelRow = 0;

    /* create file header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      if (sizeof(g_tState.bmpfile.tFileHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tFileHdr, sizeof(g_tState.bmpfile.tFileHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* create info header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;                         /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;                         /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 8;                                     /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = pInfo->uiResY * pInfo->uiResX;         /* image size      */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */

      if (sizeof(g_tState.bmpfile.tInfoHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tInfoHdr, sizeof(g_tState.bmpfile.tInfoHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* save color palette ... */
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

    /* write pixel data ... */
    if (EOK == iReturn)
    {
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

        pPixelRow = ((const uint8_t*) memmap(pInfo->tMemPixel.uiAddr)) + (uiPhysAddr & 0x1FFF); 

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
    uint8_t* pPixelRow = 0;

    /* create file header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tFileHdr.uiSize    += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize    += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiOffBits += uiPalSize;

      if (sizeof(g_tState.bmpfile.tFileHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tFileHdr, sizeof(g_tState.bmpfile.tFileHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* create info header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tInfoHdr.iWidth      = pInfo->uiResX;                         /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight     = pInfo->uiResY;                         /* image height    */
      g_tState.bmpfile.tInfoHdr.uiBitCount  = 4;                                     /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiSizeImage = ((uint32_t) pInfo->uiResY) * ((uint32_t) uiLineLen); /* image size */
      g_tState.bmpfile.tInfoHdr.uiClrUsed   = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */

      if (sizeof(g_tState.bmpfile.tInfoHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tInfoHdr, sizeof(g_tState.bmpfile.tInfoHdr)))
      {
        iReturn = EBADF;
      }
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
      if (uiPalSize != esx_f_write(g_tState.bmpfile.hFile, &g_tColorPalL0[g_tState.uiPalette * 16], uiPalSize))
      {
        iReturn = EBADF;
      }
    }
   #endif

    /* write pixel data ... */
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

        uint8_t  uiMMU2 = 0xFF;
        uint8_t  uiPhysBank; /* 8K bank */
        uint16_t uiPhysBank_ = 0xFFFF;
        uint32_t uiPhysBase;
        uint32_t uiPhysAddr;
        const uint8_t* pVirtBase = (const uint8_t*) memmap(pInfo->tMemPixel.uiAddr);

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
  register screenmode_t* pReturn = &g_tScreenModes[0];

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
/* memmap()                                                                   */
/*----------------------------------------------------------------------------*/
void* memmap(uint16_t uiPhysAddr)
{
  return (void*) uiPhysAddr;
}


/*----------------------------------------------------------------------------*/
/* _strerror()                                                                */
/*----------------------------------------------------------------------------*/
const unsigned char* _strerror(int iCode)
{
  const errentry_t* pIndex = g_tErrTable;

  while (END_OF_LIST != pIndex->iCode)
  {
    if (iCode == pIndex->iCode)
    {
      break;
    }

    ++pIndex;
  }

  return pIndex->acText;
}


/*----------------------------------------------------------------------------*/
/* msbidx8()                                                                  */
/*----------------------------------------------------------------------------*/
int8_t msbidx8(uint8_t uiValue)
{
  int iReturn = 0;

  if (0 == uiValue)
  {
    return -1; /* error: no bit set */
  }

  while (uiValue >>= 1)
  {
    ++iReturn;
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#if 0
void read_layer2_palette(uint16_t count, uint8_t *dst) {
  // $43: Bits 6..4 = 001 (Layer2 erste Palette), Bit7=1 (kein AutoInc beim Schreiben nötig)
  nextreg_write(0x43, 0x80 | (0b001 << 4));
  nextreg_write(0x40, 0);            // Index 0

  for (uint16_t i=0; i<count; ++i) {
    uint8_t c8  = nextreg_read(0x41); // RRRGGGBB
    uint8_t ext = nextreg_read(0x44); // P..... .B

    // Zusammensetzen zu 9-Bit-RGB (je 3 Bit), Priority separat, falls gewünscht:
    uint8_t r = (c8 >> 5) & 0x07;
    uint8_t g = (c8 >> 2) & 0x07;
    uint8_t b = ((c8 & 0x03) << 1) | (ext & 0x01); // B2..B1 aus c8, B0 aus ext
    uint8_t prio = (ext >> 7) & 1;                 // nur Layer2 relevant

    // Beispiel: 4 Bytes pro Eintrag: R,G,B,Prio
    *dst++ = r; *dst++ = g; *dst++ = b; *dst++ = prio;

    // Nächsten Index wählen (Lesen auto-inkrementiert nicht):
    nextreg_write(0x40, (uint8_t)(i+1));
  }
}
#endif
