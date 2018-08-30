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
#include <climits>
#include "util.h"
#include "smt.h"
#include <stdio.h>
#include <stdlib.h>

void printProgDesc()
{
    printf(
"Name\n"
"    Standalone Manufacturing Tool. Version 1.0.\n"
"Synopsis\n"
"    smt option1 option1_argument option2 option2_argument ...\n"
"Description\n"
"    SMT is a standalone executable tool that provides command line\n"
"    capability to flash Silicon Labs fixed function devices with\n"
"    desired configuration values. The tool takes as input\n"
"    a configuration file in text format that was created using the\n"
"    Xpress Family configuration tools provided in Simplicity Studio.\n"
"    All devices must be of the same device model (e.g. CP2102N).\n"
    );
}

void printCmdLineHelp()
{
    printf(
"Options\n"
"--help\n"
"    Output this page.\n"
"--verbose\n"
"    Shows the location of a syntax error in the configuration file.\n"
"--device-count <decimal number>\n"
"    Mandatory. Specifies how many devices are connected. Programming\n"
"    process will not start if it finds a different number of devices\n"
"    or fails to open them. Verification process will endlessly retry\n"
"    until it can verify this number of devices.\n"
"--reset config_file_name\n"
"    Performs soft reset of all connected devices identified by the\n"
"    configuration file, equivalent to a USB disconnect/reconnect.\n"
"--set-config config_file_name\n"
"    Programs each device using the configuration provided in the\n"
"    configuration file. Prints the list of serial numbers programmed.\n"
"    You must save this list to pass later to --verify-config.\n"
"--verify-config config_file_name\n"
"    Verifies each device using the configuration provided in the\n"
"    configuration file. \"--serial-nums GUID\" can't be used with this\n"
"    option; you must specify the numbers reported earlier by the\n"
"    programming command. Fails if device is locked.\n"
"--lock\n"
"    Legal only together with verification. If all devices are successfully\n"
"    verified, permanently locks them so they can't be customized anymore.\n"
"    Be sure you want to do this!\n"
"--verify-locked-config config_file_name\n"
"    Same as --verify-config, but will also verify locked devices\n"
"--set-and-verify-config config_file_name\n"
"    Programs and verifies each device using the configuration provided in\n"
"    the configuration file.  Prints the list of serial numbers programmed.\n"
"--serial-nums { X Y Z ... } | GUID\n"
"    Specifies that serial numbers should be written to the devices.\n"
"    If omitted, serial numbers are not programmed.\n"
"        { X Y Z } - List of strings to be used as serial numbers. The\n"
"        number of strings must be equal to the number of connected\n"
"        devices. There must be spaces between the strings and braces.\n"
"        The strings can't contain space characters.\n"
"        GUID - SMT will automatically generate unique serial numbers\n"
"        to be written to connected devices, using a platform-supplied\n"
"        UUID generation function.\n"
"--list config_file_name\n"
"    Displays a list of all connected devices identified by the\n"
"    configuration file.\n"
"\nNormal usage example\n"
"    The following command will program, verify and permanently lock the\n"
"    customizable parameters of all 3 connected devices. (Serial numbers\n"
"    will be automatically generated.)\n"
"\n    smt --device-count 3 --set-and-verify-config my_config.txt --serial-nums GUID --lock\n"
"\nExample for custom commands\n"
"    If you need to insert your own custom steps between programming and\n"
"    verification, use the following commands to separate steps. (Serial\n"
"    numbers are provided by the user to write to the device and provided\n"
"    again by the user for verification.)\n"
"\n    smt --device-count 3 --set-config my_config.txt --serial-nums { 20 21 22 23 }\n"
"    smt --device-count 3 --reset\n"
"    smt --device-count 3 --verify-config my_config.txt --serial-nums { 20 21 22 23 }\n"
    );
}

CDevType readDevType()
{
    readKeyword( "FilterPartNumByte");
    readKeyword( "{"); // start of parameter list
    BYTE partNum = readUcharParm();
    readKeyword( "}"); // end of parameter list
    return CDevType( partNum);
}

CVidPid readVidPid()
{
    readKeyword( "FilterVidPid");
    readKeyword( "{"); // start of parameter list
    WORD vid = readUshortParm();
    WORD pid = readUshortParm();
    readKeyword( "}"); // end of parameter list
    return CVidPid( vid, pid);
}

bool isSpecified( int argc, const char * argv[], const std::string &parmName, std::string &fName)
{
    for( int i = 0; i < argc; i++)
    {
        if( std::string( argv[ i]) == parmName)
        {
            i++;
            if( i < argc)
            {
                fName = argv[ i];
                return true;
            }
            throw CUsageErr( std::string( "file name is missing after ") + parmName + " command line option");
        }
    }
    return false;
}

void redirStdinToCfgFile( int argc, const char * argv[])
{
    std::string cfgFileName;
    int fileNameCnt = 0;
    if( isSpecified( argc, argv, "--set-and-verify-config", cfgFileName))
    {
        fileNameCnt++;
    }
    if( isSpecified( argc, argv, "--set-config", cfgFileName))
    {
        fileNameCnt++;
    }
    if( isSpecified( argc, argv, "--verify-config", cfgFileName))
    {
        fileNameCnt++;
    }
    if( isSpecified( argc, argv, "--verify-locked-config", cfgFileName))
    {
        fileNameCnt++;
    }
    if( isSpecified( argc, argv, "--reset", cfgFileName))
    {
        fileNameCnt++;
    }
    if( isSpecified( argc, argv, "--list", cfgFileName))
    {
        fileNameCnt++;
    }
    if( fileNameCnt == 1)
    {
        FILE * fp = freopen( cfgFileName.c_str(), "r", stdin);
        if( !fp)
        {
            char msg[ 128];
            sprintf( msg, /*SIZEOF_ARRAY( msg),*/ "configuration file open error %d", errno);
            throw CCustErr( msg);
        }
    }
    else
    {
        throw CUsageErr( "command line must specify 1 configuration file");
    }
}

int main( int argc, const char * argv[])
{
    if( argc == 1 || isSpecified( argc, argv, "--help"))
    {
        printProgDesc();
        printCmdLineHelp();
        return 0;
    }

    int rc = 1;
    try
    {
        g_EchoParserReads = isSpecified( argc, argv, "--verbose");
        redirStdinToCfgFile( argc, argv);
        const CDevType  devType = readDevType();
        const CVidPid   vidPid  = readVidPid();
        LibSpecificMain( devType, vidPid, argc, argv);
        rc = 0;
    }
    catch( const CDllErr e)
    {
        std::cerr << "ERROR: library: " << e.msg() << "\n";
    }
    catch( const CCustErr e)
    {
        std::cerr << "ERROR: Manufacturing process: " << e.msg() << "\n";
    }
    catch( const CUsageErr e)
    {
        std::cerr << e.msg() << "\n\n";
        printCmdLineHelp();
    }
    catch( const CSyntErr )
    {
    }
    catch( const std::bad_alloc& ba)
    {
        std::cerr << "P: " << ba.what()  << "\n";
    }
    return rc;
}
//---------------------------------------------------------------------------------
extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#pragma comment (lib, "Rpcrt4.lib")
#else
// TODO USER_GUIDE "apt-get install uuid-dev"
// libuuid is part of the util-linux package since version 2.15.
#include <uuid/uuid.h>
#endif
}

// asks OS to generate a UUID, then converts to a byte vector with each element containing an ASCII character
std::vector< BYTE> generateUuidAscii()
{
#ifdef WIN32
    UUID uuid;
    RPC_STATUS rpcRc = UuidCreate( &uuid);
    if( rpcRc != RPC_S_OK)
    {
        char msg[ 128];
        sprintf( msg, /*SIZEOF_ARRAY( msg),*/ "RPC failed to generate UUID, error 0x%x", rpcRc);
        throw CDllErr( msg);
    }
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
#endif
    BYTE *pByteUuid = reinterpret_cast<BYTE*>(&uuid);

    std::vector< BYTE> v;
    for( size_t i = 0; i < sizeof( uuid); i++)
    {
        char byteStr[ 3];
        sprintf( byteStr, /*SIZEOF_ARRAY( byteStr),*/ "%02x", pByteUuid[ i ]);
        v.push_back( byteStr[ 0]);
        v.push_back( byteStr[ 1]);
    }
    return v;
}

// converts a zero-term char string to a byte vector with each element containing an ASCII character
std::vector< BYTE> toAscii( const char *sz)
{
    std::vector< BYTE> v;
    for( size_t i = 0; i < strlen( sz); i++)
    {
        v.push_back( reinterpret_cast<const BYTE&>( sz[ i]));
    }
    return v;
}

CSerNumSet::CSerNumSet( int argc, const char * argv[], bool mayAutoGen, DWORD requiredCnt)
{
    m_AreNeeded    = false;
    for( int i = 0; i < argc; i++)
    {
        if( std::string( argv[ i]) == "--serial-nums")
        {
            m_AreNeeded = true;
            i++;
            if( i < argc)
            {
                if( std::string( argv[ i]) == "{")
                {
                    // SNs are specified in the command line
                    i++;
                    for( ; i < argc; i++)
                    {
                        if( std::string( argv[ i]) == "}")
                        {
                            if( m_SN.size() != requiredCnt)
                            {
                                throw CCustErr( "Serial number count is different from --device-count");
                            }
                            assertUniquness();
                            return;
                        }
                        m_SN.push_back( toAscii( argv[ i]));
                    }
                    throw CUsageErr( "Invalid serial number command line option");
                }
                else if( std::string( argv[ i]) == "GUID")
                {
                    // User requests SN aotu-generation
                    if( mayAutoGen)
                    {
                        ASSERT( m_SN.empty());
                        for( DWORD j = 0; j < requiredCnt; j++)
                        {
                            m_SN.push_back( generateUuidAscii());
                        }
                        assertUniquness();
                        return;
                    }
                    else
                    {
                        throw CUsageErr( "GUID option is illegal");
                    }
                }
            }
            throw CUsageErr( "Invalid serial number command line option");
        }
    }
    ASSERT( !m_AreNeeded);
}
void CSerNumSet::assertUniquness() const
{
    ASSERT( m_AreNeeded);
    if( !m_SN.size())
    {
        return;
    }
    for( size_t i = 0; i < m_SN.size() - 1; i++)
    {
        for( size_t j = i + 1; j < m_SN.size(); j++)
        {
            if( m_SN[ i] == m_SN[ j])
            {
                throw CUsageErr( "Identical serial numbers");
            }
        }
    }
}
void CSerNumSet::write() const
{
    if( m_SN.empty())
    {
        return;
    }
    printf( "--- new serial numbers ---\n");
    for( size_t i = 0; i < m_SN.size(); i++)
    {
        for( size_t j = 0; j < m_SN[ i].size(); j++)
        {
            printf( "%c", m_SN[ i][ j]);
        }
        printf( "\n");
    }
    printf( "--------------------------\n");
}
bool CSerNumSet::findAndErase( const std::vector< BYTE> &sN)
{
    for (std::vector< std::vector< BYTE> >::iterator it = m_SN.begin() ; it != m_SN.end(); ++it)
    {
        if( *it == sN)
        {
            m_SN.erase( it);
            return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------------
bool isSpecified( int argc, const char * argv[], const std::string &parmName)
{
    for( int i = 0; i < argc; i++)
    {
        if( std::string( argv[ i]) == parmName)
        {
            return true;
        }
    }
    return false;
}
DWORD decimalParm( int argc, const char * argv[], const std::string &parmName)
{
    for( int i = 0; i < argc; i++)
    {
        if( std::string( argv[ i]) == parmName)
        {
            i++;
            if( i < argc)
            {
                unsigned long rc = strtoul( argv[ i], NULL, 10);
                if( rc && rc != ULONG_MAX)
                {
                    return rc;
                }
            }
        }
    }
    throw CUsageErr( std::string( "Invalid or missing ") + parmName + " command line option");
}
//---------------------------------------------------------------------------------
void waitForTotalDeviceCount( const CVidPid &oldVidPid, const CVidPid &newVidPid, DWORD expectedCount)
{
#ifdef _WIN32
#pragma warning(suppress : 4127)
	while (true)
#else
	while (true)
#endif
    {
        const DWORD NumDevs = LibSpecificNumDevices( oldVidPid, newVidPid);
        if( NumDevs == expectedCount) 
        {
            printf( "waiting 3 sec...\n");
            delayMsec( 3000); // TODO - either this or extra retry messages on screen
            return;
        }
        std::cerr << "INFO: Waiting. " << NumDevs << " devices found, need " << expectedCount << "\n";
        delayMsec( 1000);
    }
    throw CCustErr( "devices failed to reboot after reset"); // dead code
}
