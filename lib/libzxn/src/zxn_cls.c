/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: zxn_cls.c                                                          |
| project:  ZX Spectrum Next - libzxn                                          |
| author:   S. Zell                                                            |
| date:     12/30/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Function to clear the screen                                                 |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| Copyright (c) 12/30/2025 STZ Engineering                                     |
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
/* zxn_cls()                                                                  */
/*----------------------------------------------------------------------------*/
void zxn_cls(uint8_t uiAttr) __z88dk_fastcall
{
  static struct esx_mode tMode;

  if (EOK == esx_ide_mode_get(&tMode))
  {
    switch (tMode.mode8.layer)
    {
      case 0x00:
        zx_cls(uiAttr);
        break;

      case 0x01:
        switch (tMode.mode8.submode)
        {
          case 0:
            /* TODO */
            break;
          case 1:
            tshc_cls(uiAttr);
            break;
          case 2:
            tshr_cls(uiAttr);
            break;
        }
        break;

      case 0x02:
        /* TODO */
        break;
    }
  }
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
