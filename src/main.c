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

#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "scrnshot.h"
#include "version.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
#define END_OF_LIST (0x7FFF)

/*!
With this macro it can be controlled wether the pixel calculations should be
done by the functions from "zxn.h" or not ...
*/
/* #define _OWN_PIXEL_CALC_ */

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
  /* --- LAYER 0 -------------------------------- */
  {0x0F, 256, 192,  16,  32,  24,   2, {0x4000, 0x1800}, {0x5800, 0x0300}}, /* Layer 0 */
  /* --- LAYER 1 -------------------------------- */
  {0x10, 128,  96, 256,   0,   0,   0}, /* Layer 1,0 */
  {0x11, 256, 192, 256,  32,  24,   2}, /* Layer 1,1 */
  {0x12, 512, 192, 256,   0,   0,   0}, /* Layer 1,2 */
  {0x13, 256, 192, 256,  32, 192,   2}, /* Layer 1,3 */
  /* --- LAYER 2 -------------------------------- */
  {0x2F, 256, 192, 256,   0,   0,   0, {0x4000, 0xC000}, {0x0000, 0x0000}}, /* Layer 2 */
  {0x22, 320, 256, 256,   0,   0,   0}, /* Layer 2,2 */
  {0x23, 640, 256,  16,   0,   0,   0}, /* Layer 2,3 */
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
*/
const bmppaletteentry_t g_tColorPalL0[16] =
{
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
  {0xFF, 0xFF, 0xFF, 0x00}    /* bright white   */
};

/*!
Table to define all textual error messages that are returned to NextOS/BASIC
*/
const err_entry_t g_tErrTable[] = {
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
static int makeScreenshot_L0(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 1
*/
static int makeScreenshot_L1(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 2
*/
static int makeScreenshot_L2(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3
*/
static int makeScreenshot_L3(const screenmode_t* pInfo);

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
This function is used to map a physical momory address to vaid pointer.
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

      case ACTION_INFO:
        g_tState.iExitCode = showInfo();
        break;

      case ACTION_HELP:
        g_tState.iExitCode = showHelp();
        break;

      case ACTION_SHOT:
        g_tState.iExitCode = makeScreenshot();
        break;
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
     #if 0
      else if ((0 == stricmp(acArg, "-c")) || (0 == stricmp(acArg, "--count")))
      {
        if ((i + 1) < argc)
        {
          g_tState.uiCount = strtoul(argv[i + 1], 0, 0);
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
  const screenmode_t* pInfo  = getScreenModeInfo(uiMode);

  if (EOK == iReturn)
  {
   #ifdef __DEBUG__
    printf("makeScreenshot() - Check name <%s> ..\n", g_tState.bmpfile.acPathName);
   #endif

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

       #ifdef __DEBUG__
        printf("makeScreenshot() - trying <%s> ...\n", acPathName);
       #endif

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
     #ifdef __DEBUG__
      printf("makeScreenshot() - trying <%s> ...\n", g_tState.bmpfile.acPathName);
     #endif

      g_tState.bmpfile.hFile = esx_f_open(g_tState.bmpfile.acPathName, ESXDOS_MODE_R | ESXDOS_MODE_OE);

     #ifdef __DEBUG__
      printf("makeScreenshot() - esx_f_open = %d\n", g_tState.bmpfile.hFile);
     #endif

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
     #ifdef __DEBUG__
      printf("makeScreenshot() - unable to open file <%s>\n", g_tState.bmpfile.acPathName);
     #endif

      iReturn = EACCES; /* Error */
    }
  }

  if (EOK == iReturn)
  {
    switch (uiMode)
    {
      /* LAYER 0 */
      case 0x0F: 
        iReturn = makeScreenshot_L0(pInfo);
        break;

      /* LAYER 1 */
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
        iReturn = makeScreenshot_L1(pInfo);
        break;

      /* LAYER 2 */
      case 0x2F:
      case 0x22:
      case 0x23:
        iReturn = makeScreenshot_L2(pInfo);
        break;

      /* LAYER 3 */
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
        iReturn = makeScreenshot_L3(pInfo);
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
/* makeScreenshot_L0()                                                        */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L0(const screenmode_t* pInfo)
{
  int iReturn = EOK;

  if (0 != pInfo)
  {
    uint16_t uiX, uiY, uiZ;
    uint16_t uiPalSize  = sizeof(g_tColorPalL0) /* 16 * 4  */;
    uint8_t  uiLineLen  = ((pInfo->uiResX >> 1) + 0x03) & 0xFC;
    uint32_t uiPxlSize  = (((uint32_t) uiLineLen) * ((uint32_t) pInfo->uiResY));
    uint8_t* pPixelData = (uint8_t*) memmap(pInfo->tMemPixel.uiAddr);
    uint8_t* pAttrData  = (uint8_t*) memmap(pInfo->tMemAttr.uiAddr);
    uint8_t* pPixelRow  = 0;
    uint8_t* pAttrRow   = 0;

   #ifdef __DEBUG__
    printf("makeScreenshot_L0() - resolution: %dx%d (%u:%u)\n",
           pInfo->uiResX,
           pInfo->uiResY,
           pInfo->tMemPixel.uiAddr,
           pInfo->tMemAttr.uiAddr);
   #endif

    /* create file header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tFileHdr.uiType      = 0x4D42;                             /* 'BM' (LE!) */
      g_tState.bmpfile.tFileHdr.uiSize      = sizeof(g_tState.bmpfile.tFileHdr);  /* file size  */
      g_tState.bmpfile.tFileHdr.uiSize     += sizeof(g_tState.bmpfile.tInfoHdr);
      g_tState.bmpfile.tFileHdr.uiSize     += uiPalSize;
      g_tState.bmpfile.tFileHdr.uiSize     += uiPxlSize;
      g_tState.bmpfile.tFileHdr.uiRes       = 0;                                  /* reserved   */
      g_tState.bmpfile.tFileHdr.uiOffBits   = sizeof(g_tState.bmpfile.tFileHdr);  /* off bits   */
      g_tState.bmpfile.tFileHdr.uiOffBits  += sizeof(g_tState.bmpfile.tInfoHdr);
      g_tState.bmpfile.tFileHdr.uiOffBits  += uiPalSize;

      if (sizeof(g_tState.bmpfile.tFileHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tFileHdr, sizeof(g_tState.bmpfile.tFileHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* create info header ... */
    if (EOK == iReturn)
    {
      g_tState.bmpfile.tInfoHdr.uiSize         = sizeof(g_tState.bmpfile.tInfoHdr);     /* header size     */
      g_tState.bmpfile.tInfoHdr.iWidth         = pInfo->uiResX;                         /* image width     */
      g_tState.bmpfile.tInfoHdr.iHeight        = pInfo->uiResY;                         /* image height    */
      g_tState.bmpfile.tInfoHdr.uiPlanes       = 1;                                     /* only one layer  */
      g_tState.bmpfile.tInfoHdr.uiBitCount     = 4;                                     /* bits per pixel  */
      g_tState.bmpfile.tInfoHdr.uiCompression  = 0;                                     /* BI_RGB          */
      g_tState.bmpfile.tInfoHdr.uiSizeImage    = pInfo->uiResY * uiLineLen;             /* image size      */
      g_tState.bmpfile.tInfoHdr.iXPelsPerMeter = BMP_DPI_72;                            /* 72 DPI          */
      g_tState.bmpfile.tInfoHdr.iYPelsPerMeter = BMP_DPI_72;                            /* 72 DPI          */
      g_tState.bmpfile.tInfoHdr.uiClrUsed      = uiPalSize / sizeof(bmppaletteentry_t); /* palette entries */
      g_tState.bmpfile.tInfoHdr.uiClrImportant = 0;                                     /* all colors used */

      if (sizeof(g_tState.bmpfile.tInfoHdr) != esx_f_write(g_tState.bmpfile.hFile, &g_tState.bmpfile.tInfoHdr, sizeof(g_tState.bmpfile.tInfoHdr)))
      {
        iReturn = EBADF;
      }
    }

    /* create color palette ... */
    if (EOK == iReturn)
    {
      if (uiPalSize != esx_f_write(g_tState.bmpfile.hFile, g_tColorPalL0, uiPalSize))
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
         #ifdef _OWN_PIXEL_CALC_
          pPixelRow = pPixelData
                      + ((uiY & 0x07) << 8)   /* Zeile innerhalb der 8er-Gruppe         */
                      + ((uiY & 0x38) << 2)   /* 8er-Gruppe innerhalb des 64er-Blocks   */
                      + ((uiY & 0xC0) << 5);  /* welcher 64er-Block (oben/mittel/unten) */
          pAttrRow  = pAttrData
                      + ((uiY >> 3) << 5);    /* uiY / 8 * 32 */
         #else
          pPixelRow = zx_pxy2saddr(0, uiY);      /* Pixelbyte-Adresse */
          pAttrRow  = zx_cxy2aaddr(0, uiY >> 3); /* Attributadresse   */      
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
/* makeScreenshot_L1()                                                        */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L1(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L2()                                                        */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L2(const screenmode_t* pInfo)
{
  pInfo = pInfo; /* to make the compiler happy */
  return ENOTSUP;
}


/*----------------------------------------------------------------------------*/
/* makeScreenshot_L3()                                                        */
/*----------------------------------------------------------------------------*/
int makeScreenshot_L3(const screenmode_t* pInfo)
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
   #ifdef __DEBUG__
    printf("screenmode: %u:%u\n", tMode.mode8.layer, tMode.mode8.submode);
   #endif

    return ((tMode.mode8.layer & 0x0F) << 4) | (tMode.mode8.submode ? tMode.mode8.submode & 0x0F : 0x0F);
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
  const err_entry_t* pIndex = g_tErrTable;

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
/*                                                                            */
/*----------------------------------------------------------------------------*/
