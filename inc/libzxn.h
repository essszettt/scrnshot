/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: libzxn.h                                                           |
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

#if !defined(__LIBZXN_H__)
  #define __LIBZXN_H__

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/
#include <stdint.h>

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
/*!
*/
#define constrain(val, min, max) (val <= min ? min : val >= max ? max : val)

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
/*!
This function calculates the address of the byte in video-memory of a pixel at
given position on the screen.
@param x  x-coordinate (0 - 255)
@param y  y-coordinate (0 - 192)
@return Pointer to the byte in video-memory
*/
extern uint8_t* zxn_pixelad_callee(uint8_t x, uint8_t y) __z88dk_callee;
#define zxn_pixelad(x, y) zxn_pixelad_callee(x, y)

/*============================================================================*/
/*                               Klassen                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementierung                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#endif /* __LIBZXN_H__ */
