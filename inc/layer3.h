/*-----------------------------------------------------------------------------+
|                                                                              |
| filename: layer3.h                                                           |
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

#if !defined(__LAYER3_H__)
  #define __LAYER3_H__

/*============================================================================*/
/*                               Includes                                     */
/*============================================================================*/

/*============================================================================*/
/*                               Defines                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Namespaces                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Constants                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Variables                                    */
/*============================================================================*/

/*============================================================================*/
/*                               Structures                                   */
/*============================================================================*/

/*============================================================================*/
/*                               Type-Definitions                             */
/*============================================================================*/

/*============================================================================*/
/*                               Prototypes                                   */
/*============================================================================*/
/*!
Reading the video memory and transcode it to BMP data for LAYER 3,0
*/
int makeScreenshot_L30(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,1
*/
int makeScreenshot_L31(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,2
*/
int makeScreenshot_L32(const screenmode_t* pInfo);

/*!
Reading the video memory and transcode it to BMP data for LAYER 3,3
*/
int makeScreenshot_L33(const screenmode_t* pInfo);

/*============================================================================*/
/*                               Classes                                      */
/*============================================================================*/

/*============================================================================*/
/*                               Implementation                               */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#endif /* __LAYER3_H__ */
