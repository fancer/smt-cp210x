// Copyright (c) 2015-2016 by Silicon Laboratories Inc.  All rights reserved.
// The program contained in this listing is proprietary to Silicon Laboratories,
// headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
// protection, including protection under the United States Copyright Act of 1976
// as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
// of the United States code.  Unauthorized copying, adaptation, distribution,
// use, or display is prohibited by this law.

#ifndef __SLABUTIL_H__
#define __SLABUTIL_H__ 1

#ifdef _WIN32
#include <Windows.h>
#else
#include "OsDep.h"
#endif

#ifdef _DEBUG
#define ASSERT(_exp) \
    ((!(_exp)) ? \
        (printf("%s(%d): Assertion failed\n   Expression: %s\n", __FILE__, __LINE__, #_exp),FALSE) : \
        TRUE)
#else
#define ASSERT( exp ) ((void) 0)
#endif

#define SIZEOF_ARRAY( a ) (sizeof( a ) / sizeof( a[0]))
#define MAX_UCHAR      0xff
#define MAX_USHORT     0xffff
#define MAX_ULONG      0xffffffff

// Taken from Wikipedia https://en.wikipedia.org/wiki/Fletcher%27s_checksum
// Must use generic pointer since function is called on XDATA and CODE spaces.
unsigned short fletcher16(unsigned char *dataIn, unsigned short bytes);

class CSyntErr // thrown any time the program can't continue processing input
{
public:
    CSyntErr( const std::string msg )
    {
        std::cerr << "ERROR: syntax: " << msg  << "\n";
    }
};

// convenience wrappers for output
void writeFuncName( std::string funcName);
void writeStr( std::string s);
void writeUlong( int val);
void writeUlong( DWORD val);
void writeUlongParm( DWORD val);
void writeUshort( WORD val);
void writeUshortParm( WORD val);
void writeUchar( BYTE val);
void writeUcharParm( BYTE val);
void writeByteArray( const std::vector<BYTE> &arr);
void writeByteArray( DWORD CbArr, const BYTE *arr);
void writeByteArrayParm( const std::vector<BYTE> &arr);

bool readWord( std::string &word, std::istream & in = std::cin );
void readKeyword( const std::string &keyWord, std::istream & in = std::cin );
void AssertHex( const std::string &valStr, const DWORD digitCnt);
DWORD readUlong(std::istream & in = std::cin);
DWORD readUlongParm(std::istream & in = std::cin);
WORD readUshort(std::istream & in = std::cin);
WORD readUshortParm(std::istream & in = std::cin);
BYTE readUchar(std::istream & in = std::cin);
BYTE readUcharParm(std::istream & in = std::cin);
bool readUcharElseTerm( BYTE &val, const std::string &terminator, std::istream & in = std::cin);
void readByteArrayParm( std::vector<BYTE> &arr, size_t max, std::istream & in = std::cin);
void readByteArrayParmExact( std::vector<BYTE> &arr, size_t CeRequired, std::istream & in = std::cin);

extern bool g_EchoParserReads;

// misc
std::string toString( const std::vector<BYTE> &a);

#endif // __SLABUTIL_H__
