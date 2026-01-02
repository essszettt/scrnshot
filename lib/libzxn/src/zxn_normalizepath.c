/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: zxn_normalizepath.c                                                |
| project:  ZX Spectrum Next - libzxn                                          |
| author:   S. Zell                                                            |
| date:     12/20/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Function to normalize a path                                                 |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 12/20/2025 STZ Engineering                                     |
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
#include <ctype.h>
#include "libzxn.h"

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
/* zxn_normalizepath()                                                        */
/*----------------------------------------------------------------------------*/
int zxn_normalizepath(char_t* acPath)
{
  /*
  ZX Spectrum Next Pfad-Normalisierung (in-place, joinbar)
  - '\\' => '/'
  - doppelte '/' zu einem '/'
  - trailing '/' entfernen (ausser bei "/" => "/.")
  - "/" wird zu "/."   (joinbar, aber bleibt im Root)
  - "X:/" wird zu "X:"
  Rueckgabe: EOK oder EINVAL bei Fehler.
  */

  if (NULL == acPath)
  {
    return EINVAL;
  }

  /* 1) '\' => '/' und doppelte '/' entfernen */
  size_t r = 0, w = 0;
  while ('\0' != acPath[r])
  {
    char c = acPath[r++];

    if ('\\' == c)
    {
      c = '/';
    }

    if ('/' == c)
    {
      if ((0 < w) && ('/' == acPath[w - 1]))
      {
        continue;
      }
    }

    acPath[w++] = c;
  }

  acPath[w] = '\0';

  /* 2) Spezialfaelle fuer "joinbare" Basen */
  if ((1 == w) && ('/' == acPath[0]))
  {
    /* "/" => "/." */
    acPath[1] = '.';
    acPath[2] = '\0';
    return EOK;
  }

  if ((3 == w) && isalpha((unsigned char) acPath[0]) && (':' == acPath[1]) && ('/' == acPath[2]))
  {
    /* "X:/" => "X:" */
    acPath[2] = '\0';
    return EOK;
  }

  /* 3) Allgemein: trailing '/' entfernen */
  while ((0 < w) && ('/' == acPath[w - 1]))
  {
    acPath[--w] = '\0';
  }

  return EOK;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
