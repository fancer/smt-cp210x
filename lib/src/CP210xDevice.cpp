/////////////////////////////////////////////////////////////////////////////
// CP210xDevice.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP210xDevice.h"
#include "CP2101Device.h"
#include "CP2102Device.h"
#include "CP2102NDevice.h"
#include "CP2103Device.h"
#include "CP2104Device.h"
#include "CP2105Device.h"
#include "CP2108Device.h"
#include "CP2109Device.h"
#include "CP210xSupportFunctions.h"
#include "CP210xManufacturing.h"

#include "silabs_defs.h"

#define SIZEOF_ARRAY( a ) (sizeof( a ) / sizeof( a[0]))

////////////////////////////////////////////////////////////////////////////////
// constructor is called after the executable is loaded, before main()
// destructor is called before the executable is unloaded, after main()
////////////////////////////////////////////////////////////////////////////////

static libusb_context* libusbContext;

__attribute__((constructor))
static void Initializer()
{
    libusb_init(&libusbContext);
}

__attribute__((destructor))
static void Finalizer()
{
    libusb_exit(libusbContext);
}

static bool IsCP210xCandidateDevice(libusb_device *pdevice)
{
    bool bIsCP210xCandidateDevice = true;   /* innocent til proven guilty */
    
    libusb_device_descriptor devDesc;
    if (0 == libusb_get_device_descriptor(pdevice, &devDesc)) {
        bIsCP210xCandidateDevice = false;
        switch (devDesc.bDeviceClass) {
        case LIBUSB_CLASS_PER_INTERFACE:  /* CP2102, CP2112,  */
            if ((1 == devDesc.iManufacturer) && (2 == devDesc.iProduct) && (3 <= devDesc.iSerialNumber)) {
                libusb_config_descriptor *pconfigDesc;
                bIsCP210xCandidateDevice = true;
                if (0 == libusb_get_config_descriptor(pdevice, 0, &pconfigDesc)) {
                    if (pconfigDesc->bNumInterfaces && pconfigDesc->interface->num_altsetting) {
                        if (LIBUSB_CLASS_VENDOR_SPEC != pconfigDesc->interface->altsetting->bInterfaceClass) {
                            bIsCP210xCandidateDevice = false;
                        }
                    }
                    libusb_free_config_descriptor(pconfigDesc);
                    pconfigDesc = (libusb_config_descriptor *) NULL;
                }
            }
            break;
                
        default:
            bIsCP210xCandidateDevice = false;
            break;
#if defined(DEBUG)
        case LIBUSB_CLASS_COMM: /* */
            /* FALLTHROUGH */
        case LIBUSB_CLASS_HID:  /* */
            bIsCP210xCandidateDevice = false;
            break;

        case 0xef:
            /* FALLTHROUGH */
        case LIBUSB_CLASS_VENDOR_SPEC:
            /* FALLTHROUGH */
        case LIBUSB_CLASS_PRINTER: /* */
            /* FALLTHROUGH */
        case LIBUSB_CLASS_HUB:
            bIsCP210xCandidateDevice = false;
            break;
#endif
        }
    }

    return bIsCP210xCandidateDevice;
}
static bool IsCP210xCandidateDevice(libusb_device_handle *pdevice_handle)
{
    bool bIsCP210xCandidateDevice = true;   /* innocent til proven guilty */
    
    return bIsCP210xCandidateDevice;
}

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class - Static Methods
/////////////////////////////////////////////////////////////////////////////

CP210x_STATUS CCP210xDevice::GetNumDevices(LPDWORD lpdwNumDevices)
{
    if (!lpdwNumDevices) {
        return CP210x_INVALID_PARAMETER;
    }

    // Enumerate all USB devices, returning the number
    // of USB devices and a list of those devices
    libusb_device** list;
    const ssize_t NumOfUSBDevices = libusb_get_device_list(libusbContext, &list);
    
    // A negative count indicates an error
	if (NumOfUSBDevices < 0) {
		return CP210x_GLOBAL_DATA_ERROR;
	}

    size_t NumOfCP210xDevices = 0;
    for (ssize_t i = 0; i < NumOfUSBDevices; i++) {
        libusb_device *device = list[i];
            
        if (IsCP210xCandidateDevice(device)) {
            libusb_device_handle* h;
                
            if ((0 == libusb_open(list[i], &h)) && IsCP210xCandidateDevice(h)) {
                BYTE partNum;
                if( CCP210xDevice::GetDevicePartNumber(h, &partNum) == CP210x_SUCCESS) {
                    if (IsValidCP210X_PARTNUM((CP210X_PARTNUM)partNum)) {
                        NumOfCP210xDevices++;
                    }
                }
                
                libusb_close(h);
                h = (libusb_device_handle *)NULL;
            }
        }
    }
    *lpdwNumDevices = static_cast<DWORD>( NumOfCP210xDevices);
    libusb_free_device_list(list, 1); // Unreference all devices to free the device list
    return CP210x_SUCCESS;
}

// 0-based counting. I.e. dwDevice = 0 gives first CP210x device
CP210x_STATUS CCP210xDevice::Open(const DWORD dwDevice, CCP210xDevice** devObj)
{
    if (!devObj) {
        return CP210x_INVALID_PARAMETER;
    }

	*devObj = NULL;

    // Enumerate all USB devices, returning the number
    // of USB devices and a list of those devices
    libusb_device** list;
    const ssize_t NumOfUSBDevices = libusb_get_device_list(libusbContext, &list);
    
    // A negative count indicates an error
	if (NumOfUSBDevices < 0) {
		return CP210x_GLOBAL_DATA_ERROR;
	}
 
	if (dwDevice >= static_cast<DWORD>(NumOfUSBDevices)) {
		return CP210x_DEVICE_NOT_FOUND;
	}

	size_t NumOfCP210xDevices = 0;
	for (ssize_t i = 0; i < NumOfUSBDevices; i++) {
        libusb_device *device = list[i];
            
		if (IsCP210xCandidateDevice(device)) {
			libusb_device_handle* h;
                
			if ((0 == libusb_open(list[i], &h)) && IsCP210xCandidateDevice(h)) {
				BYTE partNum;

				if( CCP210xDevice::GetDevicePartNumber( h, &partNum) == CP210x_SUCCESS) {
					if (IsValidCP210X_PARTNUM((CP210X_PARTNUM)partNum)) {
						if (dwDevice == NumOfCP210xDevices++) {
							BOOL bFound = TRUE;
									
							switch (partNum) {
							case CP210x_CP2101_VERSION:
								*devObj = (CCP210xDevice*)new CCP2101Device(h);
								break;
							case CP210x_CP2102_VERSION:
								*devObj = (CCP210xDevice*)new CCP2102Device(h);
								break;
							case CP210x_CP2103_VERSION:
								*devObj = (CCP210xDevice*)new CCP2103Device(h);
								break;
							case CP210x_CP2104_VERSION:
								*devObj = (CCP210xDevice*)new CCP2104Device(h);
								break;
							case CP210x_CP2105_VERSION:
								*devObj = (CCP210xDevice*)new CCP2105Device(h);
								break;
							case CP210x_CP2108_VERSION:
								*devObj = (CCP210xDevice*)new CCP2108Device(h);
								break;
							case CP210x_CP2109_VERSION:
								*devObj = (CCP210xDevice*)new CCP2109Device(h);
								break;
                            
							case CP210x_CP2102N_QFN28_VERSION:
								/* FALLTHROUGH */
							case CP210x_CP2102N_QFN24_VERSION:
								/* FALLTHROUGH */
							case CP210x_CP2102N_QFN20_VERSION:
								*devObj = (CCP210xDevice*)new CCP2102NDevice(h, partNum);
								break;
                            
							default:
								bFound = FALSE;
								break;
							}
									
							// We've found the Nth (well, dwDevice'th) CP210x device. Break from the for()-loop purposefully
							// NOT closing handle-h (after all, this in an open() function, we want to return that open handle
							if (bFound) {
								if( !(*devObj)) {
									libusb_close(h);
								}
								break;
							}
						}
					}
				}
                
				libusb_close(h);
				h = (libusb_device_handle *)NULL;
			}
		}
	} // for
	libusb_free_device_list(list, 1); // Unreference all devices to free the device list
	return (*devObj) ? CP210x_SUCCESS : CP210x_DEVICE_NOT_FOUND;
}

#if 0
CP210x_STATUS CCP210xDevice::OldOpen(const DWORD dwDevice, CCP210xDevice** devObj) {
    CP210x_STATUS status = CP210x_INVALID_PARAMETER;
    
    if (!devObj) {
        return CP210x_INVALID_PARAMETER;
    }

    // Enumerate all USB devices, returning the number
    // of devices and a list of devices
    libusb_device** list;
    const ssize_t count = libusb_get_device_list(libusbContext, &list);
    
    *devObj = NULL;

    // A negative count indicates an error
    if (count > 0) {
        if (dwDevice < (DWORD) count) {
            const libusb_device *device = list[dwDevice];
        
            if (IsCP210xCandidateDevice(device)) {
                libusb_device_handle* h;
            
                if (0 == libusb_open(list[dwDevice], &h)) {
                    if (IsCP210xCandidateDevice(h)) {
                        BYTE partNum;
            
                        status = CCP210xDevice::GetDevicePartNumber(h, &partNum);
                        if (status == CP210x_SUCCESS) {
                            switch (partNum) {
                            case CP210x_CP2101_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2101Device(h);
                                break;
                            case CP210x_CP2102_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2102Device(h);
                                break;
                            case CP210x_CP2103_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2103Device(h);
                                break;
                            case CP210x_CP2104_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2104Device(h);
                                break;
                            case CP210x_CP2105_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2105Device(h);
                                break;
                            case CP210x_CP2108_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2108Device(h);
                                break;
                            case CP210x_CP2109_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2109Device(h);
                                break;
                            
                            case CP210x_CP2102N_QFN28_VERSION:
                                /* FALLTHROUGH */
                            case CP210x_CP2102N_QFN24_VERSION:
                                /* FALLTHROUGH */
                            case CP210x_CP2102N_QFN20_VERSION:
                                *devObj = (CCP210xDevice*)new CCP2102NDevice(h, partNum);
                                break;
                            
                            default:
                                status = CP210x_DEVICE_NOT_FOUND;
                                break;
                            }
                        } else {
                            // return the status that GetDevicePartNumber failed with (was: status = CP210x_DEVICE_NOT_FOUND;)
                        }
                    } else {
                        // no error, simply not a IsCP210xCandidateDevice()
                    }
                } else {
                    status = 0x77 ; // CP210x_DEVICE_NOT_FOUND; // libusb_open
                }
            } else {
                // no error, simply not a IsCP210xCandidateDevice()
            }
        } else {
            status = CP210x_DEVICE_NOT_FOUND; // dwDevice out of range
        }
    } else {
        status = CP210x_GLOBAL_DATA_ERROR;  // libusb_get_device_list failed
    }

    // Unreference all devices to free the device list
    libusb_free_device_list(list, 1);

    return status;
}
#endif

CP210x_STATUS CCP210xDevice::GetDevicePartNumber(libusb_device_handle* h, LPBYTE lpbPartNum)
{
    if (!h || !lpbPartNum || !ValidParam(lpbPartNum)) {
        return CP210x_INVALID_PARAMETER;
    }

    const int ret = libusb_control_transfer(h, 0xC0, 0xFF, 0x370B, 0x0000, lpbPartNum, 1, 7000);
    if (1 == ret) {
        return CP210x_SUCCESS;
    }
    if( ret == LIBUSB_ERROR_TIMEOUT) {
        return CP210x_DEVICE_IO_FAILED;
    }

    libusb_config_descriptor* configDesc;
        
    CP210x_STATUS status = CP210x_DEVICE_IO_FAILED;
    if (libusb_get_config_descriptor(libusb_get_device(h), 0, &configDesc) == 0) {
        // Looking for a very particular fingerprint to conclude the device is a CP2101
        if ((configDesc->bNumInterfaces > 0) &&
            (configDesc->interface[0].altsetting->bNumEndpoints > 1) &&
            ((configDesc->interface[0].altsetting->endpoint[0].bEndpointAddress & 0x0F) == 0x03) &&
            ((configDesc->interface[0].altsetting->endpoint[1].bEndpointAddress & 0x0F) == 0x03)) {
            *lpbPartNum = CP210x_CP2101_VERSION;
            status = CP210x_SUCCESS;
        }
        libusb_free_config_descriptor(configDesc);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class - Public Methods
/////////////////////////////////////////////////////////////////////////////

CP210x_STATUS CCP210xDevice::Reset() {
    libusb_reset_device(m_handle);
    return CP210x_SUCCESS;
}

CP210x_STATUS CCP210xDevice::Close() {
    libusb_close(m_handle);
    m_handle = NULL;
    return CP210x_SUCCESS;
}

HANDLE CCP210xDevice::GetHandle() {
    return this;
}

CP210x_STATUS CCP210xDevice::GetPartNumber(LPBYTE lpbPartNum) {
    // Validate parameter
    if (!ValidParam(lpbPartNum)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (lpbPartNum) *lpbPartNum = m_partNumber;

    return CP210x_SUCCESS;
}

CP210x_STATUS CCP210xDevice::SetVid(WORD wVid) {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3701, wVid, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetPid(WORD wPid) {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3702, wPid, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetProductString(LPVOID lpvProduct, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE str[CP210x_MAX_DEVICE_STRLEN];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvProduct)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bLength > maxProductStrLen) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    CopyToString(str, lpvProduct, &length, bConvertToUnicode);

    transferSize = length + 2;
    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3703, 0, str, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }
    
    return status;
}

CP210x_STATUS CCP210xDevice::SetSerialNumber(LPVOID lpvSerialNumber, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE str[CP210x_MAX_DEVICE_STRLEN];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvSerialNumber)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bLength > maxProductStrLen) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    CopyToString(str, lpvSerialNumber, &length, bConvertToUnicode);

    transferSize = length + 2;
    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3704, 0, str, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }
    
    return status;
}

CP210x_STATUS CCP210xDevice::SetSelfPower(BOOL bSelfPower) {
    CP210x_STATUS status;
    BYTE bPowerAttrib = 0x80;

    if (bSelfPower)
        bPowerAttrib |= 0x40; // Set the self-powered bit.

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3705, bPowerAttrib, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetMaxPower(BYTE bMaxPower) {
    CP210x_STATUS status;
    if (bMaxPower > CP210x_MAX_MAXPOWER) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3706, bMaxPower, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetDeviceVersion(WORD wVersion) {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3707, wVersion, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetVid(LPWORD lpwVid) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(lpwVid)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *lpwVid = devDesc.idVendor;

        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetPid(LPWORD lpwPid) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(lpwPid)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *lpwPid = devDesc.idProduct;

        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

struct UsbStrDesc
{
    BYTE TotalBytes; // including this 2-byte prefix
    BYTE Type;       // always 3
    BYTE Data[ 0];
};

CP210x_STATUS CCP210xDevice::GetUnicodeString( uint8_t desc_index, LPBYTE pBuf, int CbBuf, LPBYTE pCchStr)
{
    CP210x_STATUS status;
    const int CbReturned = libusb_get_string_descriptor(m_handle, desc_index, 0x0000 /*desc_type*/, pBuf, CbBuf);
    if( CbReturned > 0) {
        if( CbReturned > 1) { // at least have the prefix
            const struct UsbStrDesc *pDesc = (struct UsbStrDesc *) pBuf;
            if( pDesc->TotalBytes <= CbReturned && pDesc->Type == 3) {
                const BYTE CbStr = pDesc->TotalBytes - 2; // minus the 2-byte prefix 
                // Careful! Pulling up the data by 2 bytes, overwriting the 2-byte prefix
                for( BYTE i = 0; i < CbStr; i++) {
                    pBuf[ i] = pBuf[ i + 2];
                }
                *pCchStr = CbStr / 2;
                status = CP210x_SUCCESS;
            } else {
                status = CP210x_DEVICE_IO_FAILED; // Incomplete or malformed descriptor returned
            }
        } else {
            status = CP210x_DEVICE_IO_FAILED; // Incomplete or malformed descriptor returned
        }
    } else {
        status = CP210x_DEVICE_IO_FAILED; // USB I/O failed
    }
    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE pCchStr, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;
    int length;
    int index;

    // Validate parameter
    if (!ValidParam(lpManufacturer, pCchStr)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        index = devDesc.iManufacturer;
    }

    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpManufacturer, CP210x_MAX_DEVICE_STRLEN);
        if (length > 0) {
            *pCchStr = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        status = GetUnicodeString( index, (LPBYTE) lpManufacturer, CP210x_MAX_DEVICE_STRLEN, pCchStr);
    }

    return status;
}
CP210x_STATUS CCP210xDevice::SetManufacturerString(LPVOID lpvManufacturer, BYTE CchStr, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE length = CchStr;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvManufacturer)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((CchStr > CP210x_MAX_MANUFACTURER_STRLEN) || (CchStr < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    CopyToString(setup, lpvManufacturer, &length, bConvertToUnicode);

    transferSize = length + 2;
    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3714, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }
    
    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceProductString(LPVOID lpProduct, LPBYTE pCchStr, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE index = 2;  // Our "best guess", lest libusb_get_device_descriptor() fails, and we choose to continue anyway

    // Validate parameter
    if (!ValidParam(lpProduct, pCchStr)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    libusb_device_descriptor devDesc;
    if (0 == libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc)) {
        index = devDesc.iProduct;
    }

    if (bConvertToASCII) {
        const int length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpProduct, CP210x_MAX_DEVICE_STRLEN);
        if (length > 0) {
            *pCchStr = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        status = GetUnicodeString( index, (LPBYTE) lpProduct, CP210x_MAX_DEVICE_STRLEN, pCchStr);
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceSerialNumber(LPVOID lpSerial, LPBYTE pCchStr, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE index = 3;  // Our "best guess", lest libusb_get_device_descriptor() fails, and we choose to continue anyway

    // Validate parameter
    if (!ValidParam(lpSerial, pCchStr)) {
        return CP210x_INVALID_PARAMETER;
    }

    libusb_device_descriptor devDesc;
    if (0 == libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc)) {
        index = devDesc.iSerialNumber;
    }

    if (bConvertToASCII) {
        const int length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpSerial, CP210x_MAX_DEVICE_STRLEN);
        if (length > 0) {
            *pCchStr = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        status = GetUnicodeString(index, (LPBYTE) lpSerial, CP210x_MAX_DEVICE_STRLEN, pCchStr);
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetSelfPower(LPBOOL lpbSelfPower) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_config_descriptor* configDesc;

    // Validate parameter
    if (!ValidParam(lpbSelfPower)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_config_descriptor(libusb_get_device(m_handle), 0, &configDesc) == 0) {
        if (configDesc->bmAttributes & 0x40)
            *lpbSelfPower = TRUE;
        else
            *lpbSelfPower = FALSE;
        libusb_free_config_descriptor(configDesc);
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetMaxPower(LPBYTE lpbMaxPower) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_config_descriptor* configDesc;

    // Validate parameter
    if (!ValidParam(lpbMaxPower)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_config_descriptor(libusb_get_device(m_handle), 0, &configDesc) == 0) {
        *lpbMaxPower = configDesc->MaxPower;
        libusb_free_config_descriptor(configDesc);
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceVersion(LPWORD lpwVersion) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(lpwVersion)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *lpwVersion = devDesc.bcdDevice;
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}



