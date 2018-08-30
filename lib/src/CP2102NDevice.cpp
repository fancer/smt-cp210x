/*
 * CP2102NDevice.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2102NDevice.h"
#include "CP210xSupportFunctions.h"

CCP2102NDevice::CCP2102NDevice(libusb_device_handle* h, BYTE partNum) {
    m_handle = h;
    m_partNumber = partNum;
    maxSerialStrLen = CP210x_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP210x_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2102NDevice::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetLockValue(LPBYTE lpbLockValue) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::SetLockValue() {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2102NDevice::GetFirmwareVersion( pFirmware_t	lpVersion)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE	setup[CP210x_MAX_SETUP_LENGTH];

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    if (libusb_control_transfer(m_handle,
            0xC0, // bmRequestType
            0xFF, // bRequest
            0x10, // wValue
            0, // WIndex
            setup, // data
            3, // data size
            0) == 3)
	{
		lpVersion->major = setup[0];
		lpVersion->minor = setup[1];
		lpVersion->build = setup[2];
		status = CP210x_SUCCESS;
	}
	else
	{
		lpVersion->major = 0x0;
		lpVersion->minor = 0x0;
		lpVersion->build = 0x0;
		status = CP210x_DEVICE_IO_FAILED;
	}
	return status;
}

CP210x_STATUS CCP2102NDevice::GetConfig( LPBYTE	lpbConfig, WORD	bLength)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE	setup[CP210x_MAX_SETUP_LENGTH];

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

	// 8 bytes are taken by setup packet, rest of buffer can be config data
	if (bLength > (CP210x_MAX_SETUP_LENGTH-8))
	{
		return CP210x_INVALID_PARAMETER;
	}

    if (libusb_control_transfer(m_handle,
            0xC0,
            0xFF,
            0xe, // wValue
            0, // WIndex
            setup, // data
            bLength, // data size
            0) == bLength)
    {
		memcpy((BYTE*)lpbConfig, (BYTE*)&(setup[ 0]), bLength);
		status = CP210x_SUCCESS;
	}
	else
	{
		status = CP210x_DEVICE_IO_FAILED;
	}
	return status;
}

CP210x_STATUS CCP2102NDevice::SetConfig(LPBYTE	lpbConfig,	WORD	bLength)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE	setup[CP210x_MAX_SETUP_LENGTH];

	// 8 bytes are taken by setup packet, rest of buffer can be config data
	if (bLength > (CP210x_MAX_SETUP_LENGTH-8))
	{
		return CP210x_INVALID_PARAMETER;
	}

	memcpy( (BYTE*)&(setup[0]), (BYTE*) lpbConfig, bLength);

    if (libusb_control_transfer(m_handle,
            0x40,
            0xFF,
            0x370F, // wValue
            0, // WIndex
            setup, // data
            bLength, // data size
            0) == bLength)
    {
		status = CP210x_SUCCESS;
	}
	else
	{
		status = CP210x_DEVICE_IO_FAILED;
	}
	return status;
}

CP210x_STATUS CCP2102NDevice::UpdateFirmware()
{
    (void) libusb_control_transfer(m_handle,
            0x40,
            0xFF,
            0x37FF, // wValue
            0, // WIndex
            NULL, // data
            0, // data size
            0);
		
    // SendSetup will always fail because the device
    // will get reset - so the USB request doesn't get completed
    // properly. Because of this we will always return success
    return CP210x_SUCCESS;
}

CP210x_STATUS CCP2102NDevice::GetGeneric( LPBYTE	lpbGeneric, WORD	bLength)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE	data[CP210x_MAX_SETUP_LENGTH];

	if (bLength > (CP210x_MAX_SETUP_LENGTH))
	{
		return CP210x_INVALID_PARAMETER;
	}
	if (bLength < 8)
	{
		return CP210x_INVALID_PARAMETER;
	}
    uint8_t 	bmRequestType = lpbGeneric[ 0];
    uint8_t 	bRequest      = lpbGeneric[ 1];
    uint16_t 	wValue        = lpbGeneric[ 2] | (lpbGeneric[ 3] << 8);
    uint16_t 	wIndex        = lpbGeneric[ 4] | (lpbGeneric[ 5] << 8);
    uint16_t 	wLength       = bLength - 8;

	memcpy((BYTE*)&data[0], (BYTE*)lpbGeneric + 8, wLength);

    if (libusb_control_transfer(m_handle,
            bmRequestType,
            bRequest,
            wValue,
            wIndex,
            data, // data
            wLength, // data size
            0) == wLength)
    {
		memcpy((BYTE*)lpbGeneric + 8, (BYTE*)&(data[0]), wLength);
		status = CP210x_SUCCESS;
	}
	else
	{
		status = CP210x_DEVICE_IO_FAILED;
	}
	return status;
}

CP210x_STATUS CCP2102NDevice::SetGeneric( LPBYTE	lpbGeneric, WORD	bLength)
{
	CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE	data[CP210x_MAX_SETUP_LENGTH];

	if (bLength > (CP210x_MAX_SETUP_LENGTH))
	{
		return CP210x_INVALID_PARAMETER;
	}
	if (bLength < 8)
	{
		return CP210x_INVALID_PARAMETER;
	}
    uint8_t 	bmRequestType = lpbGeneric[ 0];
    uint8_t 	bRequest      = lpbGeneric[ 1];
    uint16_t 	wValue        = lpbGeneric[ 2] | (lpbGeneric[ 3] << 8);
    uint16_t 	wIndex        = lpbGeneric[ 4] | (lpbGeneric[ 5] << 8);
    uint16_t 	wLength       = bLength - 8;

	memcpy((BYTE*)&data[0], (BYTE*)lpbGeneric + 8, wLength);

    if (libusb_control_transfer(m_handle,
            bmRequestType,
            bRequest,
            wValue,
            wIndex,
            data, // data
            wLength, // data size
            0) == wLength)
    {
		status = CP210x_SUCCESS;
	}
	else
	{
		status = CP210x_DEVICE_IO_FAILED;
	}
	return status;
}

#if 0
//===================================================================
CP210x_GetBaudRateConfig(	const HANDLE	cyHandle,  BAUD_CONFIG* baudConfigData)
{
	BYTE	setup[CP210x_MAX_SETUP_LENGTH];
	DWORD	dwBytesSent	= 0;

	memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

	setup[0] = 0xC0;	// p1						// read
	setup[1] = 0xFF;	// p2						// 
	setup[2] = 0x09;	// p3L						// get baud rate command
	setup[3] = 0x37;	// p3H						// always 0x37
	setup[4] = 0x00;							// 
	setup[5] = 0x00;							// 
	setup[6] = (NUM_BAUD_CONFIGS * BAUD_CONFIG_SIZE) & 0x00FF;			// Descriptor length, low byte, only want the first 2 bytes (0x03 and length)
	setup[7] = ((NUM_BAUD_CONFIGS * BAUD_CONFIG_SIZE) & 0xFF00) >> 8;	// Descriptor length, high byte

	status = GetSetup(cyHandle, setup, (8 + (NUM_BAUD_CONFIGS * BAUD_CONFIG_SIZE)), &dwBytesSent);
	if (status == CP210x_SUCCESS)
	{
		BAUD_CONFIG* currentBaudConfig;
		currentBaudConfig = baudConfigData;

		for (DWORD i = 8; i < dwBytesSent; i += BAUD_CONFIG_SIZE)
		{
			currentBaudConfig->BaudGen			= (setup[i] << 8) + setup[i+1];	
			currentBaudConfig++;
GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    int transferSize = (NUM_BAUD_CONFIGS * BAUD_CONFIG_SIZE);

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    if (libusb_control_transfer(m_handle,
            0xC0,
            0xFF,
            0x3709, // wValue
            0, // WIndex
            setup, // data
            transferSize, // size
            0) == transferSize)
    {
        status = CP210x_SUCCESS;
        BAUD_CONFIG* currentBaudConfig;
        currentBaudConfig = baudConfigData;

        for (int i = 0; i < transferSize; i += BAUD_CONFIG_SIZE) {
            currentBaudConfig->BaudGen = (setup[i] << 8) + setup[i + 1];
            currentBaudConfig++;
}
//===================================================================
#endif
