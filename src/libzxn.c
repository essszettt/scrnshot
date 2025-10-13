/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: libzxn.c                                                           |
| project:  ZX Spectrum Next - Common functions                                |
| author:   Stefan Zell                                                        |
| date:     09/09/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Common library for ZX Spectrum Next (maybe useful functions                  |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 09/09/2025 STZ Engineering                                     |
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
#include <stdlib.h>
#include <errno.h>

#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
/*!
Marker for the list of valid error messages, that can be returned to BASIC.
*/
#define END_OF_LIST (0x7FFF)

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

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* zxn_strerror()                                                             */
/*----------------------------------------------------------------------------*/
const unsigned char* zxn_strerror(int iCode)
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
/* memmap()                                                                   */
/*----------------------------------------------------------------------------*/
void* memmap(uint16_t uiPhysAddr)
{
  return (void*) uiPhysAddr;
}


/*----------------------------------------------------------------------------*/
/* msbidx8()                                                                  */
/*----------------------------------------------------------------------------*/
int8_t msbidx8(uint8_t uiValue)
{
  int iReturn = 0;

  if (0 != uiValue)
  {
    while (uiValue >>= 1)
    {
      ++iReturn;
    }
  }
  else
  {
    iReturn = -1 * EINVAL; /* error: no bit set */
  }

  return iReturn;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
