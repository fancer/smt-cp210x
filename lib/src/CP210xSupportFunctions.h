/*
 * CP210xSupportFunctions.h
 *
 *  Created on: Oct 30, 2012
 *      Author: strowlan
 */

#ifndef CP210XSUPPORTFUNCTIONS_H_
#define CP210XSUPPORTFUNCTIONS_H_

#include <string.h>
#include "Types.h"

#define		CP210x_MAX_SETUP_LENGTH				1024

BOOL _CP210x_ValidParam(LPVOID lpVoidPointer);
BOOL _CP210x_ValidParam(LPWORD lpwPointer);
BOOL _CP210x_ValidParam(LPBYTE lpbPointer);
BOOL _CP210x_ValidParam(LPVOID lpVoidPointer);
BOOL _CP210x_ValidParam(LPVOID lpVoidPointer, LPBYTE lpbPointer);
void _CP210x_CopyToString(BYTE* setup, LPVOID string, BYTE* bLength, BOOL bConvertToUnicode);
void _CP210x_ConvertToUnicode(BYTE* dest, BYTE* source, BYTE bLength);

#define ValidParam _CP210x_ValidParam
#define CopyToString _CP210x_CopyToString
#define ConvertToUnicode _CP210x_ConvertToUnicode

#endif /* CP210XSUPPORTFUNCTIONS_H_ */
