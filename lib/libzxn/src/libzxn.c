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
| Common library for ZX Spectrum Next (maybe useful functions)                 |
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
#include <stdio.h>
#include <errno.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
/*!
End-marker of the list of valid error messages, that can be returned
to BASIC.
*/
#define END_OF_LIST (0x7FFF)

/*============================================================================*/
/*                               Namespaces                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Strukturen                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Typ-Definitionen                             */
/*============================================================================*/
/*!
Structure to describe a entry of the global table with all valid error codes and
messages that can be handovered back to BASIC.
*/
typedef struct _errentry
{
  /*!
  Error code as defined in <errno.h> or project specific.
  */
  int iCode;

  /*!
  Pointer to a textual representation of the error code.
  */
  const char_t* acText;
} errentry_t;

/*============================================================================*/
/*                               Konstanten                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Variablen                                    */
/*============================================================================*/
/*!
Table to define textual error messages that are returned to NextOS/BASIC.
Each text-entry needs to be terminated with BIT7 set in the last character of
the string. 
*/
const errentry_t g_tErrTable[] =
{
  {EOK,         "no erro"                   "\xF2"}, /* 'r' | 0x80 */
  {EACCES,      "access denie"              "\xE4"}, /* 'd' | 0x80 */
  {EBADF,       "bad fil"                   "\xE5"}, /* 'e' | 0x80 */
  {EBDFD,       "bad file descripto"        "\xF2"}, /* 'r' | 0x80 */
  {EDOM,        "out of domain of functio"  "\xEE"}, /* 'n' | 0x80 */
  {EFBIG,       "file too larg"             "\xE5"}, /* 'e' | 0x80 */
  {EINVAL,      "invalid valu"              "\xE5"}, /* 'e' | 0x80 */
  {EMFILE,      "too many open file"        "\xE5"}, /* 'e' | 0x80 */
  {ENFILE,      "too many open files in syste\xED"}, /* 'm' | 0x80 */
  {ENOLCK,      "no locks availabl"         "\xE5"}, /* 'e' | 0x80 */
  {ENOMEM,      "out of me"                 "\xED"}, /* 'm' | 0x80 */
  {ENOTSUP,     "not supporte"              "\xE4"}, /* 'd' | 0x80 */
  {EOVERFLOW,   "overflo"                   "\xEF"}, /* 'w' | 0x80 */
  {ERANGE,      "out of rang"               "\xE5"}, /* 'e' | 0x80 */
  {ESTAT,       "bad stat"                  "\xE5"}, /* 'e' | 0x80 */
  {EAGAIN,      "resource temp. unavailabl" "\xE5"}, /* 'e' | 0x80 */
  {EWOULDBLOCK, "operation would bloc"      "\xEB"}, /* 'k' | 0x80 */
  /* ---------------- APPLICATION SPECIFIC ----------------------- */
  {EBREAK,      "D BREAK - no repea"        "\xF4"}, /* 't' | 0x80 */
  {ETIMEOUT,    "timeout erro"              "\xF2"}, /* 'r' | 0x80 */
  /* ---------------- END-OF-LIST -------------------------------- */
  {END_OF_LIST, "unknown erro"              "\xF2"}  /* 'r' | 0x80 */
};

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
const char_t* zxn_strerror(int iCode)
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
/*                                                                            */
/*----------------------------------------------------------------------------*/
