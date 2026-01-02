/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: zxn_gotoxy.c                                                       |
| project:  ZX Spectrum Next - libzxn                                          |
| author:   S. Zell                                                            |
| date:     12/20/2025                                                         |
|                                                                              |
+------------------------------------------------------------------------------+
|                                                                              |
| description:                                                                 |
|                                                                              |
| Function to print at specified position on screen                            |
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
#include <stdlib.h>
#include <stdio.h>
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
/* zxn_gotoxy()                                                               */
/*----------------------------------------------------------------------------*/
void zxn_gotoxy(uint8_t uiX, uint8_t uiY)
{
  /*
  4,x       - Disable (0) or enable (1) vertical scrolling

  8,9,11    - Move in x and y as you would expect
  12        - Form feed - clears the screen and moves print posn to 0,0
  10        - Line feed - advances y and sets x to 0
  13        - Carriage return - sets x to 0
  16,n      - Set the ink colour (*)
  17,n      - Set the paper colour (*)
  20,n      - Enable/disable inverse video (*)
  22,y,x    - Move to position y,x on the screen (0<=y<=23, 0<=x<=63)
              NB. y and x are displaced by 32 eg to move the print position
              to (0,0) use 22,32,32.

  The parameter for those marked with (*) is taken as a bitwise and of the
  lower 4 bits. Typically these are offset to [0-9] for the lower values.

  11/16/2025 SZ: If using offset "32", then "not implemented" from CRT30 
  12/30/2025 SZ: If using offset "1", then "not implemented" from CRT30 
  12/30/2025 SZ: If using offset "0", then "not implemented" for col|row=13 
  12/30/2025 SZ: If using offset "0", then output jumps by +2 for col|row=10 
  */

  fputc((int) 0x16, stdout);
  fputc((int) uiY,  stdout);
  fputc((int) uiX,  stdout);
  fflush(stdout);
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
