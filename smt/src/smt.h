// Copyright (c) 2015-2016 by Silicon Laboratories Inc.  All rights reserved.
// The program contained in this listing is proprietary to Silicon Laboratories,
// headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
// protection, including protection under the United States Copyright Act of 1976
// as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
// of the United States code.  Unauthorized copying, adaptation, distribution,
// use, or display is prohibited by this law.

#ifndef __SLABSMT_H__
#define __SLABSMT_H__ 1

#ifdef _WIN32
#include <Windows.h>
#pragma warning(disable : 4996) // TODO - I'd be happy to use never functions if everybody upgraded to VS2015
#else
#include "OsDep.h"
#endif
#include <errno.h> // for errno
#include "stdio.h"
#include "CErr.h"

#ifdef verify
#undef verify
#endif

inline void delayMsec( DWORD msec)
{
    Sleep( msec);
}


// Just to save 3 lines each time this needs to be done
inline void setSpecified( bool &specified, const std::string &parmName)
{
    if( specified)
    {
        throw CSyntErr( std::string( "multiple occurence of ") + parmName);
    }
    specified = true;
}

//-----------------------------------------------------------------------
// ctor reads SNs from command line or auto-generates if command line says so

struct CSerNumSet
{
    // analyzes the command line; if requested, creates a set of SNs
    CSerNumSet( int argc, const char * argv[], bool mayAutoGen, DWORD requiredCnt);
    void write() const;
    size_t size() const { return m_SN.size(); }
    bool empty() const { return m_SN.empty(); }
    const std::vector< BYTE>& at( DWORD index) const { return m_SN[ index ]; }
    // if the set contains the gives SN, removes it and returns true 
    bool findAndErase( const std::vector< BYTE> &sN);
private:
    void assertUniquness() const;
    bool m_AreNeeded;
    std::vector< std::vector< BYTE> > m_SN;
};

//-----------------------------------------------------------------------
// check if one of command line arguments is equal to the string
bool isSpecified( int argc, const char * argv[], const std::string &parmName);
// find a command line argument equal to the string and convert the next one to DWORD, throw CUsageErr otherwise
DWORD decimalParm( int argc, const char * argv[], const std::string &parmName);

//-----------------------------------------------------------------------
// Identifies a specific device within a family of devices supported by the same customization lib.

struct CDevType
{
    CDevType( BYTE partNum) : m_PartNum( partNum) {}
    BYTE Value() const { return m_PartNum; }
private:
    const BYTE m_PartNum;
};

//-----------------------------------------------------------------------
struct CVidPid
{
    CVidPid( WORD vid, WORD pid) : m_Vid( vid), m_Pid( pid ) {}
    const WORD m_Vid;
    const WORD m_Pid;
};

//-----------------------------------------------------------------------
// These functions must be implemented in the library-specific module
//
DWORD LibSpecificNumDevices( const CVidPid &oldVidPid, const CVidPid &newVidPid);
// This func must call the templated DevSpecificMain with device-specific types
void LibSpecificMain( const CDevType &devType, const CVidPid &vidPid, int argc, const char * argv[]);

//---------------------------------------------------------------------------------
// A helper class for CDevSet, to associate a dtor with the vector of device pointers, that deletes
// all devices. This can't be done in CDevSet dtor because it won't be called if ctor throws.
// If I could use C++ 11 and emplace_back() were not broken in VS2015, I'd simply use a vector of objects 
// instead of vectror of pointers.
template< class TDev >
struct CDevVector : public std::vector< TDev*>
{
    ~CDevVector()
    {
        for( size_t i = 0; i < std::vector< TDev*>::size(); i++)
        {
            delete std::vector<TDev*>::at( i);
        }
    }
};

//---------------------------------------------------------------------------------
// Set of opened devices. Not all devices exposed by the
// customization lib are included, but only those matching FilterVidPid.

template< class TDev >
class CDevSet
{
public:
    CDevSet( const CDevType &FilterDevType, const CVidPid &FilterVidPid, bool allowLocked = false);
    DWORD size() const { return static_cast<DWORD>( m_DevSet.size()); }
    const TDev& at( size_t i) const { return *m_DevSet[ i]; }
    void printDevInfo() const;
private:
    CDevVector<TDev> m_DevSet;
};
template< class TDev >
CDevSet<TDev>::CDevSet( const CDevType &FilterDevType, const CVidPid &FilterVidPid, bool allowLocked)
{
    DWORD NumDevs = LibSpecificNumDevices( FilterVidPid, FilterVidPid);

    ASSERT( m_DevSet.empty());
    for( DWORD i = 0; i < NumDevs; i++)
    {
        TDev *pDev = new TDev( FilterVidPid, i);
        const CDevType devType = pDev->getDevType();
        const CVidPid  vidPid  = pDev->getVidPid();
        if( FilterDevType.Value() == devType.Value() &&
            ( FilterVidPid.m_Vid == vidPid.m_Vid) &&
            ( FilterVidPid.m_Pid == vidPid.m_Pid))
        {
            if( pDev->isLocked() && !allowLocked)
            {
                delete pDev;
                throw CCustErr( "Locked device found");
            }
            m_DevSet.push_back( pDev);
        }
        else
        {
            delete pDev;
        }
    }
}
template< class TDev >
void CDevSet<TDev>::printDevInfo() const
{
    for( size_t i = 0; i < size(); i++)
    {
        const CVidPid vidPid = at( i).getVidPid();
        printf( "VID: %04x PID: %04x ", vidPid.m_Vid, vidPid.m_Pid);
        printf( "Prod Str: %s ", toString( at( i).getProduct( true)).c_str());
        printf( "Ser #: %s", toString( at( i).getSerNum( true)).c_str());
        printf( "\n");
    }
}
//---------------------------------------------------------------------------------
// Manufacturer string customization availability varies by device, so it's extracted into a class
// of its own, then included by each individual device type that supports it.

template< class TDev >
struct CManufacturerString
{
    CManufacturerString( bool supportsUnicode) : m_SupportsUnicode( supportsUnicode) { m_Specified  = false; }
    bool readParm( const std::string &parmName);
    void program( const TDev &dev) const;
    void verify( const TDev &dev) const;
private:
    const bool  m_SupportsUnicode;
    bool        m_Specified;
    bool        m_IsAscii;
    std::vector<BYTE>  m_str;
};
template< class TDev >
bool CManufacturerString<TDev>::readParm( const std::string &parmName)
{
    if( parmName == "ManufacturerStringAscii")
    {
        setSpecified( m_Specified, parmName);
        m_IsAscii    = true;
        readByteArrayParm( m_str, MAX_UCHAR);
        readKeyword( "}"); // end of parameter list
        return true;
    }
    else if( parmName == "ManufacturerStringUnicode" && m_SupportsUnicode)
    {
        setSpecified( m_Specified, parmName);
        m_IsAscii    = false;
        readByteArrayParm( m_str, MAX_UCHAR);
        readKeyword( "}"); // end of parameter list
        return true;
    }
    return false;
}
template< class TDev >
void CManufacturerString<TDev>::program( const TDev &dev) const
{
    if( !m_Specified) { return; }
    dev.setManufacturer( m_str, m_IsAscii);
}
template< class TDev >
void CManufacturerString<TDev>::verify( const TDev &dev) const
{
    if( !m_Specified) { return; }
    if( m_str != dev.getManufacturer( m_IsAscii))
    {
        throw CCustErr( "Failed ManufacturerString verification");
    }
}
//---------------------------------------------------------------------------------
// Common parameters for all devices
// WARNING: program() doesn't program all items readParm() and verify() handle.
// The rest of them is left to device-specific Parms::program().

template< class TDev >
struct CDevParms
{
    CDevParms()
    {
        m_VidPidSpecified     = false;
        m_ProdStrSpecified    = false;
        m_PowerModeSpecified  = false;
        m_MaxPowerSpecified   = false;
        m_DevVerSpecified     = false;
    }
    void read()
    {
        std::string parmName;
        while( readWord( parmName))
        {
            readKeyword( "{"); // start of parameter list
            readParm( parmName);
        }
    }
    virtual bool supportsUnicode() const { return false; }
    virtual void readParm( const std::string &parmName);
    virtual void program( const TDev &dev, const std::vector<BYTE> *pSerNum) const;
    virtual void verify( const TDev &dev, CSerNumSet &serNumSet) const;
    bool    m_VidPidSpecified;
    WORD    m_Vid;
    WORD    m_Pid;
    bool    m_ProdStrSpecified;
    bool    m_ProdStrIsAscii;
    std::vector<BYTE>  m_ProdStr;
    bool    m_PowerModeSpecified;
    BYTE    m_PowerMode;
    bool    m_MaxPowerSpecified;
    BYTE    m_MaxPower;
    bool    m_DevVerSpecified;
    WORD    m_DevVer;

    void resetAll( const CDevSet<TDev> &devSet) const;
    void programAll( const CDevSet<TDev> &devSet, const CSerNumSet &serNumSet) const;
    void verifyAll( const CDevSet<TDev> &devSet, CSerNumSet sSerNumSet) const;
    void lockAll( const CDevSet<TDev> &devSet) const;
};

void waitForTotalDeviceCount( const CVidPid &oldVidPid, const CVidPid &newVidPid, DWORD expectedCount);

template< class TDev >
void CDevParms<TDev>::readParm( const std::string &parmName)
{
    if( parmName == "VidPid")
    {
        setSpecified( m_VidPidSpecified, parmName);
        m_Vid = readUshortParm();
        m_Pid = readUshortParm();
        readKeyword( "}"); // end of parameter list
    }
    else if( parmName == "PowerMode")
    {
        setSpecified( m_PowerModeSpecified, parmName);
        m_PowerMode = readUcharParm();
        readKeyword( "}"); // end of parameter list
    }
    else if( parmName == "MaxPower")
    {
        setSpecified( m_MaxPowerSpecified, parmName);
        m_MaxPower = readUcharParm();
        readKeyword( "}"); // end of parameter list
    }
    else if( parmName == "DeviceVersion")
    {
        setSpecified( m_DevVerSpecified, parmName);
        m_DevVer = readUshortParm();
        readKeyword( "}"); // end of parameter list
    }
    else if( parmName == "ProductStringAscii")
    {
        setSpecified( m_ProdStrSpecified, parmName);
        m_ProdStrIsAscii   = true;
        readByteArrayParm( m_ProdStr, MAX_UCHAR);
        readKeyword( "}"); // end of parameter list
    }
    else if( parmName == "ProductStringUnicode" && supportsUnicode())
    {
        setSpecified( m_ProdStrSpecified, parmName);
        m_ProdStrIsAscii   = false;
        readByteArrayParm( m_ProdStr, MAX_UCHAR);
        readKeyword( "}"); // end of parameter list
    }
    else
    {
        throw CSyntErr( std::string( "unknown customization parameter ") + parmName);
    }
}
template< class TDev >
void CDevParms<TDev>::program( const TDev &dev, const std::vector<BYTE> *pSerNum) const
{
    if( pSerNum)
    {
        dev.setSerNum( *pSerNum, true /*isAscii*/);
    }
    if( m_ProdStrSpecified)
    {
        dev.setProduct( m_ProdStr, m_ProdStrIsAscii);
    }
}
template< class TDev >
void CDevParms<TDev>::verify( const TDev &dev, CSerNumSet &serNumSet) const
{
    if( !serNumSet.empty())
    {
        if( !serNumSet.findAndErase( dev.getSerNum( true /*isAscii*/)))
        {
            throw CCustErr( "Failed serial number verification");
        }
    }
    if( m_VidPidSpecified)
    {
        CVidPid devVidPid = dev.getVidPid();
        if( devVidPid.m_Vid != m_Vid || devVidPid.m_Pid != m_Pid)
        {
            throw CCustErr( "Failed VidPid verification");
        }
    }
    if( m_PowerModeSpecified)
    {
        if( m_PowerMode != dev.getPowerMode())
        {
            throw CCustErr( "Failed PowerMode verification");
        }
    }
    if( m_MaxPowerSpecified)
    {
        if( m_MaxPower != dev.getMaxPower())
        {
            throw CCustErr( "Failed MaxPower verification");
        }
    }
    if( m_DevVerSpecified)
    {
        if( m_DevVer != dev.getDevVer())
        {
            throw CCustErr( "Failed DeviceVersion verification");
        }
    }
    if( m_ProdStrSpecified)
    {
        if( m_ProdStr != dev.getProduct( m_ProdStrIsAscii))
        {
            throw CCustErr( "Failed ProductString verification");
        }
    }
}
template< class TDev >
void CDevParms<TDev>::resetAll( const CDevSet<TDev> &devSet) const
{
    for( size_t i = 0; i < devSet.size(); i++)
    {
        devSet.at( i).reset();
    }
}
template< class TDev >
void CDevParms<TDev>::programAll( const CDevSet<TDev> &devSet, const CSerNumSet &serNumSet) const
{
    for( DWORD i = 0; i < devSet.size(); i++)
    {
        program( devSet.at( i), !serNumSet.empty() ? &serNumSet.at( i) : NULL);
    }
}
template< class TDev >
void CDevParms<TDev>::verifyAll( const CDevSet<TDev> &devSet, CSerNumSet serNumSet) const
{
    for( DWORD i = 0; i < devSet.size(); i++)
    {
        verify( devSet.at( i), serNumSet);
    }
    ASSERT( serNumSet.empty());
}
template< class TDev >
void CDevParms<TDev>::lockAll( const CDevSet<TDev> &devSet) const
{
    for( DWORD i = 0; i < devSet.size(); i++)
    {
        devSet.at( i).lock();
    }
}
//---------------------------------------------------------------------------------
template< class TDev, class TDevParms >
void DevSpecificMain( const CDevType &devType, const CVidPid &FilterVidPid, int argc, const char * argv[])
{
    TDevParms devParms;
    devParms.read();

    // If the cfg file doesn't specify a new vid-pid, it's not changing;
    // so the new vid-pid is the same as filter vid-pid.
    const CVidPid NewFilterVidPid( devParms.m_VidPidSpecified ? devParms.m_Vid : FilterVidPid.m_Vid,
                                   devParms.m_VidPidSpecified ? devParms.m_Pid : FilterVidPid.m_Pid);

    if( isSpecified( argc, argv, "--reset"))
    {
        const CDevSet<TDev> oldDevSet( devType, FilterVidPid, true /*allowLocked*/);
        devParms.resetAll( oldDevSet);
        if( FilterVidPid.m_Vid != NewFilterVidPid.m_Vid || FilterVidPid.m_Pid != NewFilterVidPid.m_Pid)
        {
            const CDevSet<TDev> newDevSet( devType, NewFilterVidPid, true /*allowLocked*/);
            devParms.resetAll( newDevSet);
        }
        return;
    }

    if( isSpecified( argc, argv, "--list"))
    {
        const CDevSet<TDev> oldDevSet( devType, FilterVidPid, true /*allowLocked*/);
        printf( "--- devices --------------\n");
        oldDevSet.printDevInfo();
        if( FilterVidPid.m_Vid != NewFilterVidPid.m_Vid || FilterVidPid.m_Pid != NewFilterVidPid.m_Pid)
        {
            const CDevSet<TDev> newDevSet( devType, NewFilterVidPid, true /*allowLocked*/);
            newDevSet.printDevInfo();
        }
        printf( "--------------------------\n");
        return;
    }

    const bool program = isSpecified( argc, argv, "--set-and-verify-config") ||
                         isSpecified( argc, argv, "--set-config") ;
    const bool verify  = isSpecified( argc, argv, "--set-and-verify-config") ||
                         isSpecified( argc, argv, "--verify-config") ||
                         isSpecified( argc, argv, "--verify-locked-config") ;
    const bool lock    = isSpecified( argc, argv, "--lock");
    if( lock && !verify)
    {
        throw CUsageErr( "--lock must be combined with one of \"verify\" commands");
    }
    const DWORD custNumDevices  = decimalParm( argc, argv, "--device-count");

    const CSerNumSet serNumSet( argc, argv, program, custNumDevices);

    const DWORD startNumDevices = LibSpecificNumDevices( FilterVidPid, FilterVidPid);
    if( program)
    {
        const CDevSet<TDev> devSet( devType, FilterVidPid);
        if( devSet.size() != custNumDevices)
        {
            char msg[ 128];
            sprintf( msg, "programming step: expected %d devices, found %d", custNumDevices, devSet.size());
            throw CCustErr( msg);
        }
        serNumSet.write();
        devParms.programAll( devSet, serNumSet);
        if( verify)
        {
            devParms.resetAll( devSet);
        }
        printf( "programmed %u devices: OK\n", devSet.size());
    }
    if( verify)
    {
#if 0
        printf( "*** unplug now ***\n");
        delayMsec( 2000);
#endif
        // retry verification until it succeeds, the user can press ^C to cancel
#ifdef _WIN32
#pragma warning(suppress : 4127)
#endif
		while( true)
        {
            try // catch the exceptions that mignt go away on retry
            {
                if( program)
                {
                    waitForTotalDeviceCount( FilterVidPid, NewFilterVidPid, startNumDevices);
                }
                const bool allowLocked = !program && !lock && isSpecified( argc, argv, "--verify-locked-config");
                const CDevSet<TDev> devSet( devType, NewFilterVidPid, allowLocked);
                if( devSet.size() != custNumDevices)
                {
                    char msg[ 128];
                    sprintf( msg, "verification step: expected %d devices, found %d", custNumDevices, devSet.size());
                    throw CCustErr( msg);
                }
                devParms.verifyAll( devSet, serNumSet); // gets a copy of serNumSet
                printf( "verified %d devices: OK\n", devSet.size());
                if( lock)
                {
                    devParms.lockAll( devSet);
                    printf( "locked %d devices: OK\n", devSet.size());
                }
                break;
            }
            catch( const CDllErr e)
            {
                std::cerr << "WARNING: library: " << e.msg() << "\n";
            }
            catch( const CCustErr e)
            {
                std::cerr << "WARNING: Manufacturing process: " << e.msg() << "\n";
            }
            delayMsec( 1000);
            std::cerr << "Retrying verification...\n";
        }
    }
}
#endif // __SLABSMT_H__
