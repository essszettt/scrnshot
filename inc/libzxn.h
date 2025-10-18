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

#if !defined(__LIBZXN_H__)
  #define __LIBZXN_H__

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/
#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/
/*!
Beginning of project specific error codes.
*/
#define ERROR_SPECIFIC (0x0200)

/*!
Error code: BREAK was pressed; abort execution
*/
#define EBREAK (ERROR_SPECIFIC + 0x00)

/*!
Error code: Timeout in operation
*/
#define ETIMEOUT (ERROR_SPECIFIC + 0x01)

#ifndef ERANGE
  /*!
  (Re)definition of error "ERANGE". Maybe just a type in <errno.h> that needs to
  be corrected ...
  */
  #define ERANGE __ERANGE
  #warning "ERANGE not defined in errno.h (typo ?)"
#endif

#ifndef RTM_28MHZ
  /*!
  Missing constant for 28-Mhz-speed of ZXN (missing in <zxn.h>)
  */
  #define RTM_28MHZ 0x03
  #warning "RTM_28MHZ not defined in zxn.h"
#endif

/*!
With this macro the value of a variable can be limited to the given interval.
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

/*!
This function sets the colour of the border by a call of the ROM-function
(in 48K-Spectrum-ROM)
@param uiColour Colour of the border to set (0 .. 7)
*/
void zxn_border_fastcall(uint8_t uiColour) __z88dk_fastcall;
#define zxn_border(x) zxn_border_fastcall(x)

/*!
This function detects, if Radastan mode is active or not
(LAYER 1,0 with 16 colours).
@return "true" = Radastan mode active; "false" = Radastan mode not active
*/
bool zxn_radastan_mode(void);

/*!
This function returns a pointer to a textual error message for the given
error code.
@param iCode Error code to get it's textual representation
@return Pointer to a human readable message
*/
const unsigned char* zxn_strerror(int iCode);

/*!
This function is used to map a physical memory address to a void pointer.
@return Pointer to the given phsysical memory address
@remark Sorry, can't just cast a integer literal to a pointer.
        That's pain in my eyes ...
*/
inline void* zxn_memmap(uint16_t uiPhysAddr)
{
  return ((void*) uiPhysAddr);
}

/*!
This function detects the index of the most significant bit in an value of
type "uint8_t"
@param uiValue Value to compute index of most significant bit
@return Index of the most signifikant bit
*/
int8_t msbidx8(uint8_t uiValue);

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
