//
//  CErr.h
//  mfgtool
//
//  Created by Brant Merryman on 5/22/17.
//  Copyright © 2017 Silicon Labs. All rights reserved.
//

#ifndef CErr_h
#define CErr_h

#include <string>
#include <stdio.h>

class CErrMsg
{
public:
  CErrMsg( const char *msg) : m_Msg( msg) {}
  CErrMsg( const std::string msg ) : m_Msg( msg) {}
  const std::string &msg() const { return m_Msg; }
private:
  const std::string m_Msg;
};

class CDllErr : public CErrMsg // thrown any time a call to the DLL fails
{
public:
  CDllErr() : CErrMsg("CDllErr") {}
  CDllErr( const char *msg) : CErrMsg( msg) {}
};

class CCustErr : public CErrMsg // thrown any time the customization process goes wrong
{
public:
  CCustErr( const char *msg) : CErrMsg( msg) {}
};

class CUsageErr {
public:
  CUsageErr(std::string error_string) : mError(error_string) {}

  void print_description() { printf("Fatal Error: %s\n", mError.c_str()); }
  const char * msg() const { return mError.c_str(); }

private:
  std::string mError;
};


#endif /* CErr_h */
