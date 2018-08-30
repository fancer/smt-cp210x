// Copyright (c) 2015-2016 by Silicon Laboratories Inc.  All rights reserved.
// The program contained in this listing is proprietary to Silicon Laboratories,
// headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
// protection, including protection under the United States Copyright Act of 1976
// as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
// of the United States code.  Unauthorized copying, adaptation, distribution,
// use, or display is prohibited by this law.

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

bool g_EchoParserReads = false;

// convenience wrappers for output
void writeFuncName( std::string funcName)
{
    printf( "\n%s", funcName.c_str());
}
void writeStr( std::string s)
{
    printf( " %s", s.c_str());
}
void writeUlong( int val)
{
    printf( " %08lx", (DWORD) val);
}
void writeUlong( DWORD val)
{
    printf( " %08lx", val);
}
void writeUlongParm( DWORD val)
{
    printf( " { %08lx }", val);
}
void writeUshort( WORD val)
{
    printf( " %04x", val);
}
void writeUshortParm( WORD val)
{
    printf( " { %04x }", val);
}
void writeUchar( BYTE val)
{
    printf( " %02x", val);
}
void writeUcharParm( BYTE val)
{
    printf( " { %02x }", val);
}
void writeByteArray( const std::vector<BYTE> &arr)
{
    for( size_t i = 0; i < arr.size(); i++)
    {
        writeUchar( arr[ i]);
    }
}
void writeByteArray( DWORD CbArr, const BYTE *arr)
{
    for( DWORD i = 0; i < CbArr; i++)
    {
        writeUchar( arr[ i]);
    }
}
void writeByteArrayParm( const std::vector<BYTE> &arr)
{
    writeStr( "{");
    writeByteArray( arr);
    writeStr( "}");
}

// read from stdin any word, that is, a sequence without spaces
bool readWord( std::string &word, std::istream & in  )
{
    word.clear();
    int last = 0;
    while( (last = in.get()) != EOF)
    {
        if( !isspace( last))
        {
            // entered the word
            word += static_cast<char>( last);
            while( (last = in.get()) != EOF)
            {
                if( isspace( last))
                {
                    if( g_EchoParserReads)
                    {
                        printf( "%s%c", word.c_str(), last);
                    }
                    return true; // exited the word
                }
                word += static_cast<char>( last);
            }
            return true; // end of parameter list
        }
    }
    return false; // couldn't read a word
}

// read from stdin a specific word
void readKeyword( const std::string &keyWord, std::istream & in)
{
    std::string word;
    if( !readWord( word, in) || word != keyWord)
    {
        throw CSyntErr( std::string( "expected ") + keyWord);
    }
}

void AssertHex( const std::string &valStr, const DWORD digitCnt)
{
    if( valStr.size() != digitCnt)
    {
        throw CSyntErr( "invalid hex number size");
    }
    for( DWORD i = 0; i < digitCnt; i++)
    {
        if( !isxdigit( valStr[i]))
        {
            throw CSyntErr( "invalid hex number size");
        }
    }
}

// read from stdin a hex ulong
DWORD readUlong(std::istream & in )
{
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( "expected hex ulong");
    }
    AssertHex( valStr, 8);
    return static_cast<DWORD>( strtoul(valStr.c_str(), NULL, 16));
}
DWORD readUlongParm(std::istream & in )
{
    readKeyword( "{");
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( "expected hex ulong");
    }
    readKeyword( "}");
    AssertHex( valStr, 8);
    return static_cast<DWORD>( strtoul(valStr.c_str(), NULL, 16));
}

// read from stdin a hex ushort
WORD readUshort(std::istream & in )
{
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( "expected hex ushort");
    }
    AssertHex( valStr, 4);
    return static_cast<WORD>( strtoul(valStr.c_str(), NULL, 16));
}
WORD readUshortParm(std::istream & in )
{
    readKeyword( "{");
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( "expected hex ushort");
    }
    AssertHex( valStr, 4);
    readKeyword( "}");
    return static_cast<WORD>( strtoul(valStr.c_str(), NULL, 16));
}

// read from stdin a hex uchar
BYTE readUchar(std::istream & in )
{
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( "expected hex uchar");
    }
    AssertHex( valStr, 2);
    return static_cast<BYTE>( strtoul(valStr.c_str(), NULL, 16));
}
BYTE readUcharParm(std::istream & in)
{
    readKeyword( "{");
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( "expected hex uchar");
    }
    AssertHex( valStr, 2);
    readKeyword( "}");
    return static_cast<BYTE>( strtoul(valStr.c_str(), NULL, 16));
}

// read from stdin a byte given as 2 hex digits, else the terminator
bool readUcharElseTerm( BYTE &val, const std::string &terminator, std::istream & in)
{
    std::string valStr;
    if( !readWord( valStr, in))
    {
        throw CSyntErr( std::string( "expected hex byte or ") + terminator);
    }
    if( valStr == std::string( terminator))
    {
        return false;
    }
    if( valStr.size() != 2 || !isxdigit( valStr[0]) || !isxdigit( valStr[1]))
    {
        throw CSyntErr( std::string( "expected hex byte or ") + terminator);
    }
    val = static_cast<BYTE>( strtoul(valStr.c_str(), NULL, 16));
    return true;
}

// read from stdin a variable-size byte array in braces
void readByteArrayParm( std::vector<BYTE> &arr, size_t max, std::istream & in)
{
    readKeyword( "{", in);
    arr.clear();
    BYTE b;
    while( readUcharElseTerm( b, "}", in))
    {
        if( max && arr.size() == max)
        {
            throw CSyntErr( "byte array too large");
        }
        arr.push_back( b);
    }
}

// read from stdin an exact-size byte array in braces
void readByteArrayParmExact( std::vector<BYTE> &arr, size_t CeRequired, std::istream & in)
{
    readByteArrayParm( arr, CeRequired, in);
    if( arr.size() != CeRequired)
    {
        throw CSyntErr( "byte array too small");
    }
}

std::string toString( const std::vector<BYTE> &a)
{
    std::string s;
    for( size_t i = 0; i < a.size(); i++)
    {
        s += (char) a[ i];
    }
    return s;
}

unsigned short fletcher16(unsigned char *dataIn, unsigned short bytes)
{
  unsigned short sum1 = 0xff, sum2 = 0xff;
  unsigned short tlen;

  while (bytes) {
    tlen = bytes >= 20 ? 20 : bytes;
    bytes -= tlen;
    do {
      sum2 += sum1 += *dataIn++;
    } while (--tlen);
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
  }
  /* Second reduction step to reduce sums to 8 bits */
  sum1 = (sum1 & 0xff) + (sum1 >> 8);
  sum2 = (sum2 & 0xff) + (sum2 >> 8);
  return sum2 << 8 | sum1;
}

