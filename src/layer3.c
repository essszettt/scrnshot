/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: capture_L3.c                                                       |
| project:  zxn::scrnshot                                                      |
| author:   S. Zell                                                            |
| date:     02/28/2026                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Image capture functions for LAYER3                                           |
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
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "libzxn.h"
#include "scrnshot.h"
#include "layer3.h"

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
/*                                                                            */
/*----------------------------------------------------------------------------*/
