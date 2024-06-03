/* SPDX-License-Identifier: MIT */

#include "netana_user.h"
#include "netana_toolkit.h"

#ifndef min
  #define min(a,b)  ((a < b) ? a : b)
#endif

#define NANOSEC_BUFFER_OVERFLOW 0x3B9ACA00

/*****************************************************************************/
/*! Errorcode to Errordescription lookup table (english only)                */
/*****************************************************************************/
static struct NETANA_ERROR_TO_DESCRtag
{
  long  lError;
  char* szErrorDescr;

} s_atErrorToDescrTable[] =
{
#ifndef NETANA_TKIT_NO_ERRORLOOKUP
  /*******************************************************************************
  * netANALYZER Device Driver Errors
  *******************************************************************************/
  {NETANA_TKIT_INITIALIZATION_FAILED,   "Toolkit initialization failed"},
  {NETANA_DMABUFFER_CREATION_FAILED,    "Creation of DMA buffers failed"},
  {NETANA_DEVICE_STILL_OPEN,            "Device is still in use by another application"},
  {NETANA_HWRESET_ERROR,                "Error during hardware reset of device"},
  {NETANA_MEMORY_MAPPING_FAILED,        "Failed to map memory"},
  {NETANA_CHIP_NOT_SUPPORTED,           "Chip type is not supported by toolkit"},
  {NETANA_DOWNLOAD_FAILED,              "Download of Bootloader / Firmware failed"},
  {NETANA_INVALID_PARAMETER,            "Invalid parameter"},
  {NETANA_DEVICE_NOT_FOUND,             "Device with the given name does not exist"},
  {NETANA_DEVICE_NOT_OPEN,              "Device is not open"},
  {NETANA_FUNCTION_FAILED,              "Function failed (Generic Error)"},
  {NETANA_INVALID_PORT,                 "Invalid port number"},
  {NETANA_INVALID_POINTER,              "Invalid pointer"},
  {NETANA_INVALID_HANDLE,               "Invalid handle"},
  {NETANA_NO_WORKING_DIRECTORY,         "No working directory"},
  {NETANA_NO_ENTRY_FOUND,               "No entry found"},
  {NETANA_FUNCTION_NOT_AVAILABLE,       "Function not available"},
  {NETANA_INVALID_BUFFERSIZE,           "Size of given buffer is invalid"},
  {NETANA_FILE_OPEN_ERROR,              "Error opening file"},
  {NETANA_FILE_READ_FAILED,             "Error reading file"},
  {NETANA_TRANSFER_TIMEOUT,             "Transfer timed out"},
  {NETANA_CAPTURE_ACTIVE,               "Capturing is currently active"},
  {NETANA_IRQEVENT_CREATION_FAILED,     "Error creating interrupt events"},
  {NETANA_CONFIGURATION_ERROR,          "Capture configuration error"},
  {NETANA_IRQLOCK_CREATION_FAILED,      "Error creating internal IRQ locks"},
  {NETANA_FILE_CREATION_FAILED,         "Error creating file"},
  {NETANA_FILE_WRITE_FAILED,            "Error writing file"},
  {NETANA_IOCTL_FAILED,                 "Error sending IOCTL to driver"},
  {NETANA_FW_START_FAILED,              "Error starting firmware"},
  {NETANA_DRIVER_NOT_RUNNING,           "netANALYZER Driver is not running"},
  {NETANA_OUT_OF_MEMORY,                "Out of memory"},
  {NETANA_THREAD_CREATION_FAILED,       "Creation of worker thread failed"},
  {NETANA_NO_BUFFER_DATA,               "Buffer content not valid"},
  {NETANA_NO_STATE_CHANGE,              "No state change"},
  {NETANA_NO_PLUGIN_FOUND,              "No plugin found"},
  {NETANA_FILECAPTURE_NOT_ACTIVE,       "Capturing to file is not enabled"},
  {NETANA_CAPTURE_NOT_ACTIVE,           "Capturing is currently stopped"},
  {NETANA_ACCESS_DENIED,                "Access denied"},
/* Errors that may happen during firmware load/start */
  {NETANA_DEV_MAILBOX_FULL,             "Device mailbox is full"},
  {NETANA_DEV_NOT_READY,                "Device not ready"},
  {NETANA_DEV_MAILBOX_TOO_SHORT,        "Mailbox is too short for packet"},
  {NETANA_DEV_GET_NO_PACKET,            "No packet available"},
  {NETANA_BUFFER_TOO_SHORT,             "Given buffer is too short"},
  {NETANA_NO_ERROR,                     ""},
/* capture error codes */
  {NETANA_CAPTURE_ERROR_STOP_TRIGGER,     "No error"},
  {NETANA_CAPTURE_ERROR_NO_DMACHANNEL,    "No free DMA channel available. Probably host is too slow"},
  {NETANA_CAPTURE_ERROR_URX_OVERFLOW,     "XC buffer overflow (URX overflow)"},
  {NETANA_CAPTURE_ERROR_NO_HOSTBUFFER,    "No free DMA buffer available. Host is too slow to handle data efficiently"},
  {NETANA_CAPTURE_ERROR_NO_INTRAMBUFFER,  "Internal capture buffer overflow (no free INTRAM)"},
  {NETANA_CAPTURE_ERROR_STATUS_FIFO_FULL, "Capture FIFO overflow"},
  {NETANA_CAPTURE_ERROR_DRIVER_FILE_FULL, "End of capture file reached"},
 /* Marshaller transport errors */
  {NETANA_TRANSPORT_CHECKSUM_ERROR,       "Checksum incorrect"},
  {NETANA_TRANSPORT_LENGTH_INCOMPLETE,    "Transport lenght is incomplete"},
  {NETANA_TRANSPORT_DATA_TYPE_UNKOWN,     "Unknown datatype"},
  {NETANA_TRANSPORT_DEVICE_UNKNOWN,       "Device is unknown"},
  {NETANA_TRANSPORT_CHANNEL_UNKNOWN,      "Channel is unknown"},
  {NETANA_TRANSPORT_SEQUENCE,             "Sequence error"},
  {NETANA_TRANSPORT_BUFFEROVERFLOW,       "Bufferoverflow"},
  {NETANA_TRANSPORT_KEEPALIVE,            "Keepalive error"},
  {NETANA_TRANSPORT_RESOURCE,             "Resource error"},
  {NETANA_TRANSPORT_ERROR_UNKNOWN,        "Unknown error"},
  {NETANA_TRANSPORT_RECV_TIMEOUT,         "Receive timeout"},
  {NETANA_TRANSPORT_SEND_TIMEOUT,         "Time out while sending data"},
  {NETANA_TRANSPORT_CONNECT,              "Unable to communicate to the device / no answer"},
  {NETANA_TRANSPORT_ABORTED,              "Transfer has been aborted due to keep alive timeout or interface detachment"},
  {NETANA_TRANSPORT_INVALID_RESPONSE,     "The packet response was rejected due to invalid packet data"},
  {NETANA_TRANSPORT_UNKNOWN_DATALAYER,    "The data layer provided by the device is not supported"},
  {NETANA_CONNECTOR_FUNCTIONS_READ_ERROR, "Error reading the connector functions from the DLL"},
  {NETANA_CONNECTOR_IDENTIFIER_TOO_LONG,  "Connector delivers an identifier longer than 6 characters"},
  {NETANA_CONNECTOR_IDENTIFIER_EMPTY,     "Connector delivers an empty dentifier"},
  {NETANA_CONNECTOR_DUPLICATE_IDENTIFIER, "Connector identifier already used"},
  {NETANA_TRANSPORT_DATA_TOO_SHORT,       "Received transaction data too short"},
  {NETANA_TRANSPORT_UNSUPPORTED_FUNCTION, "Function is not supported"},
  {NETANA_TRANSPORT_TIMEOUT,              "Transport timeout"},
  {NETANA_CAPTURE_ERROR_ON_TARGET,        "Capture error on the target device"},
  /*******************************************************************************/
#else
  {NETANA_NO_ERROR, ""},
#endif
};

static void    CloseCaptureFiles(struct NETANA_DEVINSTANCE_T* ptDevInst);
static int32_t OpenCaptureFiles(struct NETANA_DEVINSTANCE_T* ptDevInst);


static int32_t OpenCaptureFiles(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
  int32_t  lRet = NETANA_NO_ERROR;
  uint32_t ulFile;
  char     szFileName[NETANA_MAX_FILEPATH * 2];

  CloseCaptureFiles(ptDevInst);
  ptDevInst->ulCurrentFile = 0;

  for(ulFile = 0; ulFile < ptDevInst->ulFileCount; ++ulFile)
  {
    NETANA_FILE_DATA_T* ptFile = &ptDevInst->atFiles[ulFile];

    /* Open File */
    if(sizeof(szFileName) < OS_Snprintf(szFileName, sizeof(szFileName),
                                        "%s%s%s_%u.hea",
                                        ptDevInst->szPath,
                                        OS_PATH_SEPERATOR,
                                        ptDevInst->szBaseFilename,
                                        ulFile))
    {
      /* Unable to format string correctly */
      if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
      {
        USER_Trace(ptDevInst,
                   NETANA_TRACELEVEL_ERROR,
                   "Error formatting Filename. Internal buffer is too short!");
      }

      lRet = NETANA_FILE_CREATION_FAILED;
      break;

    } else if(NULL == (ptFile->hFile = OS_FileOpen(szFileName,
                                                   &ptDevInst->ullFileSize,
                                                   OS_FILE_FLAG_CREATEIFNEEDED)))
    {
      /* Error opening file */
      lRet = NETANA_FILE_CREATION_FAILED;
      break;
    } else
    {
      ptFile->ullOffset      = 0;
      ptFile->ullDataWritten = 0;
    }
  }

  return lRet;
}

static void CloseCaptureFiles(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
  uint32_t ulFile;

  for(ulFile = 0; ulFile < ptDevInst->ulFileCount; ++ulFile)
  {
    NETANA_FILE_DATA_T* ptFile = &ptDevInst->atFiles[ulFile];

    if(NULL != ptFile->hFile)
    {
      OS_FileClose(ptFile->hFile);
      ptFile->hFile = NULL;
    }
  }
}

static int IsCaptureRunning(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
  int                         fRet          = 0;
  NETANA_NETX_STATUS_BLOCK_T* ptStatusBlock = &ptDevInst->ptBaseDPM->tNetxStatusBlock;

  if(ptStatusBlock->ulCaptureState != NETANA_CAPTURE_STATE_OFF)
  {
    fRet = 1;
  }

  return fRet;
}

static uint8_t GetHskBitState(struct NETANA_DEVINSTANCE_T* ptDevInst,
                              uint32_t ulBitMask,
                              uint8_t  bState)
{
  uint8_t bRet;

  switch(bState)
  {
    case NETANA_BITSTATE_SET:
    case NETANA_BITSTATE_CLEAR:
      if(ptDevInst->ulNetxFlags & ulBitMask)
        bRet = NETANA_BITSTATE_SET;
      else
        bRet = NETANA_BITSTATE_CLEAR;
      break;

    default:
      if((ptDevInst->ulHostFlags ^ ptDevInst->ulNetxFlags) & ulBitMask)
        bRet = NETANA_BITSTATE_UNEQUAL;
      else
        bRet = NETANA_BITSTATE_EQUAL;
      break;
  }

  return bRet;
}

int netana_tkit_WaitForBitState(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulBit,
                                uint8_t bState, uint32_t ulTimeout)
{
  int       iRet              = 0;
  uint32_t  ulBitMask         = 1 << ulBit;
  int32_t   lStartTime        = 0;
  uint32_t  ulInternalTimeout = ulTimeout;
  uint8_t   bActualState      = GetHskBitState(ptDevInst, ulBitMask, bState);

  /* The desired state is already there, so just return true */
  if(bActualState == bState)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  /* Just wait for the Interrupt event to be signalled. This bit was toggled if the interrupt
     is executed, so we don't need to check bit state afterwards
     Note: Wait first time with timeout 0 and check if the state is the expected one.
           If not it was a previously set event and we need to wait with the user supplied time out */
  lStartTime = (int32_t)OS_GetMilliSecCounter();

  do
  {
    uint32_t ulCurrentTime;
    uint32_t ulDiffTime;

    /* Wait for DSR to signal Handshake bit change event */
    OS_WaitEvent(ptDevInst->ahHandshakeBitEvents[ulBit], ulInternalTimeout);

    ulCurrentTime = OS_GetMilliSecCounter();
    ulDiffTime    = ulCurrentTime - lStartTime;

    /* Adjust timeout for next run */
    ulInternalTimeout = ulTimeout - ulDiffTime;

    /* Check bit state */
    bActualState = GetHskBitState(ptDevInst, ulBitMask, bState);

    if(bActualState == bState)
    {
      iRet = 1;
      break;
    }

    if( ulDiffTime > ulTimeout)
    {
      /* Timeout expired */
      break;
    }

  } while(iRet == 0);

  return iRet;
}

void netana_tkit_ToggleBit(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulBit)
{
  NETANA_HANDSHAKE_CELL* ptHskCell = &ptDevInst->ptHandshakeBlock->atHandshake[0];
  OS_EnterLock(ptDevInst->pvLock);

  ptDevInst->ulHostFlags        ^= (1 << ulBit);
  ptHskCell->t16Bit.usHostFlags = (uint16_t)ptDevInst->ulHostFlags;

  OS_LeaveLock(ptDevInst->pvLock);
}

/*****************************************************************************/
/*! Generate device information from device instance structure
*   \param ptDevice       Device instance
*   \param ulDevInfoSize  Size of the device information structure
*   \param ptDevInfo      Device information structure                       */
/*****************************************************************************/
void GenerateDeviceInformation(struct NETANA_DEVINSTANCE_T* ptDevice,
                                      uint32_t                     ulDevInfoSize,
                                      NETANA_DEVICE_INFORMATION_T* ptDevInfo)
{
  NETANA_DEVICE_INFORMATION_T tDevInfo    = {{0}};
  NETANA_SYSTEM_INFO_BLOCK_T* ptInfoBlock = &ptDevice->ptBaseDPM->tSystemInfoBlock;

  OS_Strncpy(tDevInfo.szDeviceName, ptDevice->szAliasName, sizeof(tDevInfo.szDeviceName));

  tDevInfo.ulPhysicalAddress  = ptDevice->ulDPMPhysicalAddress;
  tDevInfo.ulDPMSize          = ptDevice->ulDPMSize;
  tDevInfo.ulUsedDPMSize      = ptInfoBlock->ulDPMUsedSize;
  tDevInfo.ulDPMLayoutVersion = ptInfoBlock->ulDPMLayoutVersion;
  tDevInfo.ulInterrupt        = ptDevice->ulInterruptNr;

  tDevInfo.ulDeviceNr         = ptDevice->ulDeviceNr;
  tDevInfo.ulSerialNr         = ptDevice->ulSerialNr;

  OS_Memcpy(tDevInfo.ausHwOptions, (void*)ptInfoBlock->ausHwOptions, sizeof(tDevInfo.ausHwOptions));

  tDevInfo.usManufacturer     = ptInfoBlock->usManufacturer;
  tDevInfo.usProductionDate   = ptInfoBlock->usProductionDate;
  tDevInfo.usDeviceClass      = ptInfoBlock->usDeviceClass;
  tDevInfo.bHwRevision        = ptInfoBlock->bHwRevision;
  tDevInfo.bHwCompatibility   = ptInfoBlock->bHwCompatibility;

  tDevInfo.ulOpenCnt          = ptDevice->ulOpenCnt;
  tDevInfo.ulPortCnt          = ptDevice->ulPortCnt;
  tDevInfo.ulGpioCnt          = ptDevice->ulGpioCnt;
  tDevInfo.ulFilterSize       = ptDevice->ulFilterSize;
  tDevInfo.ulCounterSize      = ptInfoBlock->ulCounterSize;

  OS_Memcpy(tDevInfo.szFirmwareName, (void*)ptInfoBlock->szFirmwareName, sizeof(ptInfoBlock->szFirmwareName));

  tDevInfo.ulVersionMajor     = ptInfoBlock->ulVersionMajor;
  tDevInfo.ulVersionMinor     = ptInfoBlock->ulVersionMinor;
  tDevInfo.ulVersionBuild     = ptInfoBlock->ulVersionBuild;
  tDevInfo.ulVersionRevision  = ptInfoBlock->ulVersionRevision;

  OS_Memset( (void*)&tDevInfo.tExtendedInfo, 0, sizeof(NETANA_EXTENDED_DEV_INFO_T));

  /* TODO: setup information structure depending on ulExtendedInfoType */
  /* currently only the GBE variant is supported                       */
  switch (tDevInfo.ulExtendedInfoType)
  {
    case NETANA_GBE_VARIANT: /* GbE variant */
    {
      //tDevInfo.ulExtendedInfoSize = sizeof(NETANA_EXTENDED_GBE_DEV_INFO_T);
      tDevInfo.tExtendedInfo.tGBEExtendedInfo.ulToolkitVersionMajor    = NETANA_TOOLKIT_VERSION_MAJOR;
      tDevInfo.tExtendedInfo.tGBEExtendedInfo.ulToolkitVersionMinor    = NETANA_TOOLKIT_VERSION_MINOR;
      tDevInfo.tExtendedInfo.tGBEExtendedInfo.ulToolkitVersionBuild    = NETANA_TOOLKIT_VERSION_BUILD;
      tDevInfo.tExtendedInfo.tGBEExtendedInfo.ulToolkitVersionRevision = NETANA_TOOLKIT_VERSION_REVISION;
    }
    break;

    default:
    break;
  }

  OS_Memcpy(ptDevInfo, &tDevInfo, ulDevInfoSize);
}

/*****************************************************************************/
/*! Get driver / toolkit base information
*   \param ulDrvInfoSize  Size of the Driver information buffer
*   \param ptDrvInfo      Driver information buffer
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_driver_information(uint32_t ulDrvInfoSize, NETANA_DRIVER_INFORMATION_T* ptDrvInfo)
{
  NETANA_DRIVER_INFORMATION_T tDrvInfo = {0};

  if(NULL == ptDrvInfo)
    return NETANA_INVALID_PARAMETER;

  tDrvInfo.ulCardCnt        = g_ulCardCnt;

  tDrvInfo.ulMaxFileCount   = NETANA_MAX_FILES;
  tDrvInfo.ulDMABufferSize  = g_ulDMABufferSize;
  tDrvInfo.ulDMABufferCount = g_ulDMABufferCount;

  tDrvInfo.ulToolkitVersionMajor    = NETANA_TOOLKIT_VERSION_MAJOR;
  tDrvInfo.ulToolkitVersionMinor    = NETANA_TOOLKIT_VERSION_MINOR;
  tDrvInfo.ulToolkitVersionBuild    = NETANA_TOOLKIT_VERSION_BUILD;
  tDrvInfo.ulToolkitVersionRevision = NETANA_TOOLKIT_VERSION_REVISION;

  OS_Memcpy(ptDrvInfo, &tDrvInfo, ulDrvInfoSize);

  return NETANA_NO_ERROR;
}

/*****************************************************************************/
/*! Get human-readable error description (englisch) from error code
*   \param lError       Error code
*   \param ulBufferSize Size of the user buffer
*   \param szBuffer     Returned error string
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_get_error_description(int32_t lError, uint32_t ulBufferSize, char* szBuffer)
{
  long lRet = NETANA_FUNCTION_FAILED;
  int  iIdx = 0;

  for(iIdx = 0; iIdx < (unsigned long)(sizeof(s_atErrorToDescrTable) / sizeof(s_atErrorToDescrTable[0])); ++iIdx)
  {
    if(s_atErrorToDescrTable[iIdx].lError == lError)
    {
      OS_Strncpy(szBuffer, s_atErrorToDescrTable[iIdx].szErrorDescr, ulBufferSize);
      lRet = NETANA_NO_ERROR;
      break;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Enumerate all devices handled by driver / toolkit
*   \param ulCardNr       Number of the card
*   \param ulDevInfoSize  Size of the device information structure
*   \param ptDevInfo      Device information structure
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_enum_device(uint32_t ulCardNr, uint32_t ulDevInfoSize, NETANA_DEVICE_INFORMATION_T* ptDevInfo)
{
  int32_t                      lRet     = NETANA_DEVICE_NOT_FOUND;  /* Default to device not found */
  struct NETANA_DEVINSTANCE_T* ptDevice = STAILQ_FIRST(&g_tCardList);

  if(ulCardNr > g_ulCardCnt)
    return NETANA_DEVICE_NOT_FOUND;

  OS_EnterLock(g_pvCardLock);

  /* Search for device */
  while( (ulCardNr-- > 0) && (NULL != (ptDevice = STAILQ_NEXT(ptDevice, tList) )) ) ;

  OS_LeaveLock(g_pvCardLock);

  if(NULL != ptDevice)
  {
    NETANA_SYSTEM_INFO_BLOCK_T* ptInfoBlock = &ptDevice->ptBaseDPM->tSystemInfoBlock;
    uint32_t                    ulFilter    = 0;
    int                         fSkip       = 1;

    lRet = NETANA_DEVICE_NOT_FOUND;

    if (0 == (ulFilter = USER_GetDevClassFilter()))
    {
      USER_SetDevClassFilter( NETANA_DEV_CLASS_NANL_500);
      ulFilter = NETANA_DEV_CLASS_NANL_500;
    }

    switch (ptInfoBlock->usDeviceClass)
    {
      case NANL_DEV_CLASS:
      {
        if (NETANA_DEV_CLASS_NANL_500 == (NETANA_DEV_CLASS_NANL_500 & ulFilter))
          fSkip = 0;
      }
      break;
      case NSCP_DEV_CLASS:
      {
        if (NETANA_DEV_CLASS_NSCP_100 == (NETANA_DEV_CLASS_NSCP_100 & ulFilter))
          fSkip = 0;
      }
      break;
      case CIFX_DEV_CLASS:
      {
        if (NETANA_DEV_CLASS_CIFX == (NETANA_DEV_CLASS_CIFX & ulFilter))
          fSkip = 0;
      }
      break;
      default:
      break;
    }

    if (fSkip == 0)
    {
      GenerateDeviceInformation(ptDevice, ulDevInfoSize, ptDevInfo);
      lRet = NETANA_NO_ERROR;
    } else
    {
      lRet = NETANA_ACCESS_DENIED;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Get device information from opened device
*   \param hDev       Device handle
*   \param ptDevInfo  Returned device information
*   \param ptDevInfo      Device information structure
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_device_info(NETANA_HANDLE hDev, uint32_t ulDevInfoSize, NETANA_DEVICE_INFORMATION_T* ptDevInfo)
{
  struct NETANA_DEVINSTANCE_T* ptDevice;

  if(NULL == hDev)
    return NETANA_INVALID_PARAMETER;

  ptDevice = (struct NETANA_DEVINSTANCE_T*)hDev;
  GenerateDeviceInformation(ptDevice, ulDevInfoSize, ptDevInfo);

  return NETANA_NO_ERROR;
}

/*****************************************************************************/
/*! Open device by name
*   \param szDevice   Name of the device
*   \param phDev      Returned device handle
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_open_device(char* szDevice, NETANA_HANDLE* phDev)
{
  /* Default to device not found */
  int32_t                      lRet     = NETANA_DEVICE_NOT_FOUND;
  struct NETANA_DEVINSTANCE_T* ptDevice = STAILQ_FIRST(&g_tCardList);

  if( (NULL == szDevice) || (NULL == phDev) )
    return NETANA_INVALID_PARAMETER;

  OS_EnterLock(g_pvCardLock);

  /* Search for device */
  while(ptDevice != NULL)
  {
    if(0 == OS_Strnicmp(szDevice,
                        ptDevice->szAliasName,
                        sizeof(ptDevice->szAliasName)))
    {
      NETANA_SYSTEM_INFO_BLOCK_T* ptInfoBlock = &ptDevice->ptBaseDPM->tSystemInfoBlock;
      uint32_t                    ulFilter    = 0;
      int                         fSkip       = 1;

      lRet = NETANA_DEVICE_NOT_FOUND;

      if (0 == (ulFilter = USER_GetDevClassFilter()))
      {
        USER_SetDevClassFilter( NETANA_DEV_CLASS_NANL_500);
        ulFilter = NETANA_DEV_CLASS_NANL_500;
      }
      switch (ptInfoBlock->usDeviceClass)
      {
        case NANL_DEV_CLASS:
        {
          if (NETANA_DEV_CLASS_NANL_500 == (NETANA_DEV_CLASS_NANL_500 & ulFilter))
            fSkip = 0;
        }
        break;
        case NSCP_DEV_CLASS:
        {
          if (NETANA_DEV_CLASS_NSCP_100 == (NETANA_DEV_CLASS_NSCP_100 & ulFilter))
            fSkip = 0;
        }
        break;
        case CIFX_DEV_CLASS:
        {
          if (NETANA_DEV_CLASS_CIFX == (NETANA_DEV_CLASS_CIFX & ulFilter))
            fSkip = 0;
        }
        break;
        default:
        break;
      }
      if (fSkip == 0)
      {
        /* Device found */
        if(ptDevice->ulOpenCnt > 0)
        {
          lRet = NETANA_DEVICE_STILL_OPEN;
        } else
        {
          uint32_t ulNo = 0;

          ++ptDevice->ulOpenCnt;
          *phDev = ptDevice;
          lRet   = NETANA_NO_ERROR;

          /* reset filter configuration */
          for (ulNo=0;ulNo<ptDevice->ulPortCnt;ulNo++)
          {
            NETANA_FILTER_T tFilterA = {0};
            NETANA_FILTER_T tFilterB = {0};

            tFilterA.ulFilterSize = ptDevice->ulFilterSize;
            tFilterA.pbMask       = ptDevice->atDefaultFilter[ulNo].atFilter[0].pbMask;
            tFilterA.pbValue      = ptDevice->atDefaultFilter[ulNo].atFilter[0].pbValue;
            tFilterB.ulFilterSize = ptDevice->ulFilterSize;
            tFilterB.pbMask       = ptDevice->atDefaultFilter[ulNo].atFilter[1].pbMask;
            tFilterB.pbValue      = ptDevice->atDefaultFilter[ulNo].atFilter[1].pbValue;

            netana_set_filter( ptDevice, ulNo, &tFilterA, &tFilterB, 0);
          }
          /* reset gpio configuration */
          for(ulNo = 0; ulNo < ptDevice->ulGpioCnt; ++ulNo)
          {
            OS_Memcpy( ptDevice->aptGPIOs[ulNo], &ptDevice->atDefaultGPIOs[ulNo], sizeof(NETANA_GPIO_BLOCK_T));
          }
        }
      } else
      {
        lRet = NETANA_ACCESS_DENIED;
      }
      break;
    } else
    {
      ptDevice = STAILQ_NEXT(ptDevice, tList);
    }
  }

  OS_LeaveLock(g_pvCardLock);

  return lRet;
}

/*****************************************************************************/
/*! Close an open device
*   \param hDev      Returned device handle
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_close_device(NETANA_HANDLE hDev)
{
  struct NETANA_DEVINSTANCE_T* ptDevice = NULL;
  int32_t                      lRet     = NETANA_DEVICE_NOT_OPEN;

  if(NULL == hDev)
    return NETANA_INVALID_PARAMETER;

  ptDevice = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(ptDevice->ulOpenCnt > 0)
  {
    --ptDevice->ulOpenCnt;
    lRet = NETANA_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Get filter for a specific port
*   \param hDev             Device handle
*   \param ulPort           Port number (0..3)
*   \param ptFilterA        Returned filter A
*   \param ptFilterB        Returned filter B
*   \param pulRelationShip  Returned logical relationship of filters
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_get_filter(NETANA_HANDLE hDev, uint32_t ulPort,
                        NETANA_FILTER_T* ptFilterA, NETANA_FILTER_T* ptFilterB, uint32_t* pulRelationShip)
{
  int32_t                      lRet      = NETANA_INVALID_PORT;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(ulPort < ptDevInst->ulPortCnt)
  {
    NETANA_DPM_FILTER_T* ptFilter = &ptDevInst->atFilters[ulPort];

    if(pulRelationShip != NULL)
      *pulRelationShip = ptFilter->ptBase->ulRelationShip;

    if((ptFilterA != NULL) && (ptFilterA->ulFilterSize>0))
    {
      if ((NULL == ptFilterA->pbMask) || (NULL == ptFilterA->pbValue))
        return NETANA_INVALID_PARAMETER;

      if (ptFilterA->ulFilterSize>ptDevInst->ulFilterSize)
        ptFilterA->ulFilterSize = ptDevInst->ulFilterSize;

      OS_Memcpy(ptFilterA->pbMask,  ptFilter->atFilter[0].pbMask,  ptFilterA->ulFilterSize);
      OS_Memcpy(ptFilterA->pbValue, ptFilter->atFilter[0].pbValue, ptFilterA->ulFilterSize);
    }

    if((ptFilterB != NULL) && (ptFilterB->ulFilterSize>0))
    {
      if ((NULL == ptFilterB->pbMask) || (NULL == ptFilterB->pbValue))
        return NETANA_INVALID_PARAMETER;

      if (ptFilterB->ulFilterSize>ptDevInst->ulFilterSize)
        ptFilterB->ulFilterSize = ptDevInst->ulFilterSize;

      OS_Memcpy(ptFilterB->pbMask,  ptFilter->atFilter[1].pbMask,  ptFilterB->ulFilterSize);
      OS_Memcpy(ptFilterB->pbValue, ptFilter->atFilter[1].pbValue, ptFilterB->ulFilterSize);
    }

    lRet = NETANA_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Set filter for a specific port
*   \param hDev             Device handle
*   \param ulPort           Port number (0..3)
*   \param ptFilterA        Filter A
*   \param ptFilterB        Filter B
*   \param pulRelationShip  Logical relationship of filters
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_set_filter(NETANA_HANDLE hDev, uint32_t ulPort,
                          NETANA_FILTER_T* ptFilterA, NETANA_FILTER_T* ptFilterB, uint32_t ulRelationShip)
{
  int32_t                      lRet      = NETANA_INVALID_PORT;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_ACTIVE;

  /* validate given parameters */
  if((ptFilterA != NULL) && (ptFilterA->ulFilterSize>0))
  {
    if ((NULL == ptFilterA->pbMask) || (NULL == ptFilterA->pbValue))
      return NETANA_INVALID_PARAMETER;

    if ((ptFilterA->ulFilterSize > ptDevInst->ulFilterSize))
      return NETANA_INVALID_BUFFERSIZE;
  }
  if((ptFilterB != NULL) && (ptFilterB->ulFilterSize>0))
  {
    if ((NULL == ptFilterB->pbMask) || (NULL == ptFilterB->pbValue))
      return NETANA_INVALID_PARAMETER;

    if (ptFilterB->ulFilterSize > ptDevInst->ulFilterSize)
      return NETANA_INVALID_BUFFERSIZE;
  }

  if(ulPort < ptDevInst->ulPortCnt)
  {
    NETANA_DPM_FILTER_T* ptFilter = &ptDevInst->atFilters[ulPort];

    ptFilter->ptBase->ulRelationShip = ulRelationShip;

    /* Clear all filters */
    OS_Memset(ptFilter->atFilter[0].pbMask,  0, ptDevInst->ulFilterSize);
    OS_Memset(ptFilter->atFilter[0].pbValue, 0, ptDevInst->ulFilterSize);
    OS_Memset(ptFilter->atFilter[1].pbMask,  0, ptDevInst->ulFilterSize);
    OS_Memset(ptFilter->atFilter[1].pbValue, 0, ptDevInst->ulFilterSize);

    /* Set new filters */
    if((ptFilterA != NULL) && (ptFilterA->ulFilterSize>0))
    {
      OS_Memcpy(ptFilter->atFilter[0].pbMask,  ptFilterA->pbMask,  ptFilterA->ulFilterSize);
      OS_Memcpy(ptFilter->atFilter[0].pbValue, ptFilterA->pbValue, ptFilterA->ulFilterSize);
    }

    if((ptFilterB != NULL) && (ptFilterB->ulFilterSize>0))
    {
      OS_Memcpy(ptFilter->atFilter[1].pbMask,  ptFilterB->pbMask,  ptFilterB->ulFilterSize);
      OS_Memcpy(ptFilter->atFilter[1].pbValue, ptFilterB->pbValue, ptFilterB->ulFilterSize);
    }

    lRet = NETANA_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Retrieve port status
*   \param hDev       Device handle
*   \param ulPort     Port number (0..3)
*   \param ptStatus   Pointer for returned status data
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_get_portstat(NETANA_HANDLE hDev, uint32_t ulPort,
                            uint32_t ulSize, NETANA_PORT_STATE_T* ptStatus)
{
  int32_t                      lRet      = NETANA_INVALID_PORT;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(ulPort < ptDevInst->ulPortCnt)
  {
    uint32_t ulCopySize = min((size_t)ulSize, sizeof(NETANA_PORT_STATE_T));

    lRet = NETANA_NO_ERROR;

    /* Send Update Command to firmware */
    if(!netana_tkit_WaitForBitState(ptDevInst,
                                    NETANA_HSK_HOST_UPDATE_CMD,
                                    NETANA_BITSTATE_EQUAL,
                                    NETANA_DEFAULT_TIMEOUT))
    {
      lRet = NETANA_FUNCTION_FAILED;

    } else
    {
      netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_UPDATE_CMD);

      if(!netana_tkit_WaitForBitState(ptDevInst,
                                      NETANA_HSK_HOST_UPDATE_CMD,
                                      NETANA_BITSTATE_EQUAL,
                                      NETANA_DEFAULT_TIMEOUT))
      {
        lRet = NETANA_FUNCTION_FAILED;
      }
    }
    if(NETANA_NO_ERROR == lRet)
      OS_Memcpy(ptStatus, ptDevInst->aptCounters[ulPort], ulCopySize);
  }

  return lRet;
}

/*****************************************************************************/
/*! Set file list for use with driver. Driver will create all files and
*   use them on next run.
*   \param hDev           Device handle
*   \param szPath         Directory for files
*   \param szBaseFilename Basefilename (will be automatically be extended with filenumber)
*   \param ulFileCount    Number of files to use, set to 0 if no file should be written
*   \param ullFileSize    Size of each file
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_set_filelist(NETANA_HANDLE hDev,
                            char* szPath, char* szBaseFilename,
                            uint32_t ulFileCount, uint64_t ullFileSize)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_ACTIVE;

  if(ulFileCount > NETANA_MAX_FILES)
    return NETANA_INVALID_PARAMETER;

  /* A file must be at least as large as a single DMA buffer + file header,
     as we can only write complete DMA buffers to disk */
  if(ullFileSize < (g_ulDMABufferSize + sizeof(NETANA_FILE_HEADER_T)))
    return NETANA_INVALID_PARAMETER;

  OS_Strncpy(ptDevInst->szPath,         szPath,         sizeof(ptDevInst->szPath));
  OS_Strncpy(ptDevInst->szBaseFilename, szBaseFilename, sizeof(ptDevInst->szBaseFilename));
  ptDevInst->ulFileCount = ulFileCount;
  ptDevInst->ullFileSize = ullFileSize;

  if(NETANA_NO_ERROR != (lRet = OpenCaptureFiles(ptDevInst)))
  {
    CloseCaptureFiles(ptDevInst);

    /* if the function fails we need to clear the filecount,                     */
    /* otherwise starting the capture (without "file-capturing") is not possible */
    ptDevInst->ulFileCount = 0;
  }

  return lRet;
}

/*****************************************************************************/
/*! Start a live capture
*   \param hDev               Device handle
*   \param ulCaptureMode      Capture mode (see NETANA_CAPTUREMODE_XXXX)
*   \param ulPortEnableMask   Bitmask of used ports
*   \param ulMacMode          Mac Mode (see NETANA_MACMODE_XXX)
*   \param ullReferenceTime   Time reference for measurement
*   \param pfnStatus          Status callback (e.g. Capture stopped), set to NULL if not used
*   \param pfnData            Callback if new data is available, set to NULL if not used (e.g. only write to file mode)
*   \param pvUser             User parameter to pass on callbacks
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_start_capture(NETANA_HANDLE hDev, uint32_t ulCaptureMode, uint32_t ulPortEnableMask,
                             uint32_t ulMacMode, uint64_t ullReferenceTime,
                             PFN_STATUS_CALLBACK pfnStatus, PFN_DATA_CALLBACK pfnData,
                             void* pvUser)
{
  int32_t                      lRet       = NETANA_NO_ERROR;
  struct NETANA_DEVINSTANCE_T* ptDevInst  = (struct NETANA_DEVINSTANCE_T*)hDev;
  NETANA_HOST_CONTROL_BLOCK_T* ptHostCtrl = &ptDevInst->ptBaseDPM->tHostControlBlock;
  uint32_t                     ulIdx;

  if(IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_ACTIVE;

  if( (ptDevInst->ulFileCount == 0) &&
      (pfnData == NULL) )
    return NETANA_CONFIGURATION_ERROR;

  ptDevInst->pfnStatus = pfnStatus;
  ptDevInst->pfnData   = pfnData;
  ptDevInst->pvUser    = pvUser;

  /* Fill Up DMA Buffers */
  ptHostCtrl->ulBufferFlags       = 0;
  ptHostCtrl->ulBufferCount       = ptDevInst->ulDMABufferCount;
  ptHostCtrl->ulHostBufferTimeout = g_ulDMABufferTimeout;

  for(ulIdx = 0; ulIdx < ptDevInst->ulDMABufferCount;++ulIdx)
  {
    NETANA_HOST_DMA_BUFFER_T* ptBufferDPM  = &ptHostCtrl->atDmaBuffers[ulIdx];
    NETANA_DMABUFFER_T*       ptBufferHost = &ptDevInst->atDMABuffers[ulIdx];

    ptBufferDPM->ulPhysicalAddressHigh = 0;
    ptBufferDPM->ulPhysicalAddressLow  = ptBufferHost->ulPhysicalAddress;

    ptBufferDPM->ulBufferSize          = ptBufferHost->ulBufferSize;
  }

  /* Setup Capturing */
  ptHostCtrl->ulCaptureMode    = ulCaptureMode;
  ptHostCtrl->ulPortEnableMask = ulPortEnableMask;
  ptHostCtrl->ulMacMode        = ulMacMode;

  ptHostCtrl->ulRefSeconds     = (uint32_t)(ullReferenceTime >> 32);
  ptHostCtrl->ulRefNanoseconds = (uint32_t)(ullReferenceTime & 0xFFFFFFFF);

  ptHostCtrl->ulHostError      = 0; /* Reset Host Error for easier debugging */

  if(ulCaptureMode & NETANA_CAPTUREMODE_RINGBUFFER)
  {
    ptDevInst->fRingBuffer = 1;
  } else
  {
    ptDevInst->fRingBuffer = 0;
  }

  if(ptDevInst->ulFileCount > 0)
  {
    ptDevInst->ulCurrentFile  = 0;
    ptDevInst->fWriteToFile   = 1;

    ptDevInst->tFileHeader.ulCookie            = NETANA_FILE_HEADER_COOKIE;
    ptDevInst->tFileHeader.ulIdx               = 0;
    ptDevInst->tFileHeader.ullCaptureStart     = ullReferenceTime;
    ptDevInst->tFileHeader.lTimezoneCorrection = g_lTimezoneCorrection;
    ptDevInst->tFileHeader.ulReserved          = 0;


    /* Write file header */
    if(NETANA_NO_ERROR != netana_tkit_writefileheader(ptDevInst,
                                                      NETANA_FILE_HEADER_SIZE_INVALID))
    {
      lRet = NETANA_FILE_WRITE_FAILED;
    }
  } else
  {
    ptDevInst->fWriteToFile = 0;
  }

  ptDevInst->ulCurrentBuffer = 0;

  if(NETANA_NO_ERROR == lRet)
  {
    /* Send Start command to firmware */
    if(!netana_tkit_WaitForBitState(ptDevInst,
                                    NETANA_HSK_HOST_STARTSTOP_CMD,
                                    NETANA_BITSTATE_EQUAL,
                                    NETANA_DEFAULT_TIMEOUT))
    {
      lRet = NETANA_FUNCTION_FAILED;

    } else
    {
      netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_STARTSTOP_CMD);

      if(!netana_tkit_WaitForBitState(ptDevInst,
                                      NETANA_HSK_HOST_STARTSTOP_CMD,
                                      NETANA_BITSTATE_EQUAL,
                                      NETANA_DEFAULT_TIMEOUT))
      {
        lRet = NETANA_FUNCTION_FAILED;
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Stop current capture
*   \param hDev               Device handle
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_stop_capture(NETANA_HANDLE hDev)
{
  int32_t                      lRet      = NETANA_NO_ERROR;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(!IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_NOT_ACTIVE;

  /* Send stop command to firmware */
  if(!netana_tkit_WaitForBitState(ptDevInst,
                                  NETANA_HSK_HOST_STARTSTOP_CMD,
                                  NETANA_BITSTATE_EQUAL,
                                  NETANA_DEFAULT_TIMEOUT))
  {
    lRet = NETANA_FUNCTION_FAILED;

  } else
  {
    OS_ResetEvent(ptDevInst->hStopComplete);

    netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_STARTSTOP_CMD);

    /* Firmware will toggle it's bit, when the last DMA has been finished */
    if(!netana_tkit_WaitForBitState(ptDevInst,
                                    NETANA_HSK_HOST_STARTSTOP_CMD,
                                    NETANA_BITSTATE_EQUAL,
                                    NETANA_DEFAULT_TIMEOUT))
    {
      lRet = NETANA_FUNCTION_FAILED;
    } else
    {
      uint32_t ulCycles = 50;

      do
      {
        if(OS_WaitEvent(ptDevInst->hStopComplete, 100))
          break;

      } while(--ulCycles > 0);

      ptDevInst->pfnStatus = NULL;
      ptDevInst->pfnData   = NULL;

      if(ptDevInst->ulFileCount > 0)
      {
        /* Only update header if it was not already stopped by DSR */
        if(ptDevInst->fWriteToFile)
        {
          /* Write File header into last written file */
          uint32_t            ulFile       = ptDevInst->ulCurrentFile;
          NETANA_FILE_DATA_T* ptFileData   = &ptDevInst->atFiles[ulFile];

          netana_tkit_writefileheader(ptDevInst, ptFileData->ullOffset -
                                                 sizeof(NETANA_FILE_HEADER_T));
        }

        /* Release all open capture files */
        CloseCaptureFiles(ptDevInst);

        /* It's defined that all files are closed and user is expected to call
           netana_setfilelist before next capture */
        ptDevInst->ulFileCount = 0;
      }
    }
  }

  return lRet;
}



/*****************************************************************************/
/*! Get current capture state
*   \param hDev             Device handle
*   \param pulCaptureState  Returned capture state
*   \param pulCaptureError  Returned capture error
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_get_state(NETANA_HANDLE hDev, uint32_t* pulCaptureState, uint32_t* pulCaptureError)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if( (NULL != pulCaptureState) &&
      (NULL != pulCaptureError) )
  {
    NETANA_NETX_STATUS_BLOCK_T* ptStatus = &ptDevInst->ptBaseDPM->tNetxStatusBlock;

    *pulCaptureState = ptStatus->ulCaptureState;
    *pulCaptureError = ptStatus->ulCaptureError;

    lRet = NETANA_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Read or Write a PHY register
*   \param hDev         Device handle
*   \param ulDirection  Read / Write (see NETANA_PHY_DIRECTION_XXX)
*   \param ulPhyNum     PHY Address (0..31)
*   \param ulPhyReg     PHY Register to read (0..31)
*   \param pusValue     Register value ([IN] on write, [OUT] on read)
*   \param ulTimeout    Timeout in ms to wait for response
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_access_phy_reg(NETANA_HANDLE hDev, uint32_t ulDirection,
                              uint8_t ulPhyNum, uint8_t ulPhyReg, uint16_t* pusValue,
                              uint32_t ulTimeout)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(ulPhyNum < ptDevInst->ulPortCnt)
  {
    if(!netana_tkit_WaitForBitState(ptDevInst,
                                    NETANA_HSK_HOST_MDIO_CMD,
                                    NETANA_BITSTATE_EQUAL,
                                    ulTimeout))
    {
      /* Error waiting for MDIO are being unlocked */
      lRet = NETANA_FUNCTION_FAILED;

    } else
    {
      NETANA_MDIO_BLOCK_T* ptMdio = &ptDevInst->ptBaseDPM->tMdioBlock;

      ptMdio->ulDirection  = ulDirection;
      ptMdio->ulPhyNum     = ulPhyNum;
      ptMdio->ulPhyReg     = ulPhyReg;

      if(NETANA_PHY_DIRECTION_WRITE == ulDirection)
        ptMdio->ulValue      = *pusValue;

      netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_MDIO_CMD);

      if(!netana_tkit_WaitForBitState(ptDevInst,
                                      NETANA_HSK_HOST_MDIO_CMD,
                                      NETANA_BITSTATE_EQUAL,
                                      ulTimeout))
      {
        /* Error waiting for MDIO access being finished */
        lRet = NETANA_FUNCTION_FAILED;
      } else
      {
        if(NETANA_PHY_DIRECTION_READ == ulDirection)
          *pusValue = (uint16_t)ptMdio->ulValue;

        lRet = NETANA_NO_ERROR;
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Get GPIO Mode
*   \param hDev      Device handle
*   \param ulGpio    Number of Gpio (0..3)
*   \param ptMode    Returned mode
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_get_gpio_mode(NETANA_HANDLE hDev, uint32_t ulGpio, NETANA_GPIO_MODE_T* ptMode)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(ulGpio < ptDevInst->ulGpioCnt)
  {
    NETANA_GPIO_BLOCK_T* ptBlock = ptDevInst->aptGPIOs[ulGpio];

    ptMode->ulMode = ptBlock->ulMode;

    OS_Memset(&ptMode->uData, 0, sizeof(ptMode->uData));

    switch(ptMode->ulMode)
    {
    default:
    case NETANA_GPIO_MODE_NONE:
    case NETANA_GPIO_MODE_RISING_EDGE:
    case NETANA_GPIO_MODE_FALLING_EDGE:

      ptMode->uData.tTrigger.ulCaptureTriggers = ptBlock->uData.tTrigger.ulCaptureTriggers;
      ptMode->uData.tTrigger.ulEndDelay        = ptBlock->uData.tTrigger.ulEndDelay;
      break;

    case NETANA_GPIO_MODE_OUTPUT:
      break;

    case NETANA_GPIO_MODE_OUTPUT_PWM:
      ptMode->uData.tPwm.ulHiPeriod            = ptBlock->uData.tPwm.ulHiPeriod;
      ptMode->uData.tPwm.ulLoPeriod            = ptBlock->uData.tPwm.ulLoPeriod;
      break;
    }

    lRet = NETANA_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Set GPIO Mode
*   \param hDev      Device handle
*   \param ulGpio    Number of Gpio (0..3)
*   \param ptMode    Gpio mode
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_set_gpio_mode(NETANA_HANDLE hDev, uint32_t ulGpio, NETANA_GPIO_MODE_T* ptMode)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_ACTIVE;

  if(ulGpio < ptDevInst->ulGpioCnt)
  {
    NETANA_GPIO_BLOCK_T* ptBlock = ptDevInst->aptGPIOs[ulGpio];

    switch(ptMode->ulMode)
    {
    default:
    case NETANA_GPIO_MODE_NONE:

      ptBlock->ulMode = ptMode->ulMode;

      lRet = NETANA_NO_ERROR;
      break;
    case NETANA_GPIO_MODE_RISING_EDGE:
    case NETANA_GPIO_MODE_FALLING_EDGE:

      ptBlock->ulMode = ptMode->ulMode;

      ptBlock->uData.tTrigger.ulCaptureTriggers = ptMode->uData.tTrigger.ulCaptureTriggers;
      ptBlock->uData.tTrigger.ulEndDelay        = ptMode->uData.tTrigger.ulEndDelay;

      lRet = NETANA_NO_ERROR;
      break;

    case NETANA_GPIO_MODE_OUTPUT:

      ptBlock->ulMode = ptMode->ulMode;
      /* NOTE: Level will be set later with a seperate function, as this is
               needed to be performed at runtime */
      ptBlock->uData.tOutput.ulLevel     = 0;

      lRet = NETANA_NO_ERROR;
      break;

    case NETANA_GPIO_MODE_OUTPUT_PWM:

      lRet = NETANA_INVALID_PARAMETER;

      /* check configuration parameter */
      if ( (ptMode->uData.tPwm.ulHiPeriod <= 100000000) &&
           (ptMode->uData.tPwm.ulHiPeriod >= 10)        &&
           (ptMode->uData.tPwm.ulLoPeriod <= 100000000) &&
           (ptMode->uData.tPwm.ulLoPeriod >= 10) )
      {
        NETANA_GPIO_BLOCK_T* ptTmpBlock     = ptDevInst->aptGPIOs[ulGpio];
        uint32_t             ulGPIOCnt      = 0;
        int                  fGPIOPWMActive = 0;

        /* check if a GPIO is already PWM configured  */
        for( ;ulGPIOCnt < ptDevInst->ulGpioCnt; ulGPIOCnt++)
        {
          ptTmpBlock = ptDevInst->aptGPIOs[ulGPIOCnt];
          if ((ptTmpBlock->ulMode == ptMode->ulMode) && (ulGpio != ulGPIOCnt))
          {
            fGPIOPWMActive = 1;
            break;
          }
        }
        /* only activate PWM if it is not allready configured */
        if (fGPIOPWMActive == 0)
        {
          ptBlock->ulMode = ptMode->ulMode;

          ptBlock->uData.tPwm.ulHiPeriod = ptMode->uData.tPwm.ulHiPeriod;
          ptBlock->uData.tPwm.ulLoPeriod = ptMode->uData.tPwm.ulLoPeriod;

          lRet = NETANA_NO_ERROR;
        } else
        {
          lRet = NETANA_FUNCTION_FAILED;
        }
      }
      break;

    case NETANA_GPIO_MODE_SYNC:

      ptBlock->ulMode = ptMode->ulMode;

      /* Relative time in ns used for offset compensation of trigger input signal */
      ptBlock->uData.tSync.ulPeriod = ptMode->uData.tSync.ulPeriod;

      lRet = NETANA_NO_ERROR;
      break;
    }
  }
  return lRet;
}

/*****************************************************************************/
/*! Set GPIO output
*   \param hDev      Device handle
*   \param ulGpio    Number of Gpio (0..3)
*   \param ulLevel
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_set_gpio_output(NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t ulLevel)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if (ptDevInst == NULL)
    return lRet;

  /* do not refuse the user here, otherwise it is not possible */
  /* to start the capture with a predefined state              */
  //if(!IsCaptureRunning(ptDevInst))
  //  return NETANA_CAPTURE_NOT_ACTIVE;

  if ( (ulGpio >= ptDevInst->ulGpioCnt) ||
       ((ulLevel != 0) && (ulLevel != 1)) ) /* ulLevel should be either 0 or 1 */
  {
    return lRet;
  } else
  {
    NETANA_GPIO_BLOCK_T* ptBlock = ptDevInst->aptGPIOs[ulGpio];

    if (ptBlock->ulMode == NETANA_GPIO_MODE_OUTPUT)
    {
      /* set GPIO level */
      ptBlock->uData.tOutput.ulLevel     = ulLevel;

      lRet = NETANA_NO_ERROR;
    } else
    {
      lRet = NETANA_FUNCTION_FAILED;
    }
  }
  return lRet;
}

/*****************************************************************************/
/*! Get current file capture state
*   \param hDev           Device handle
*   \param ulFileInfoSize Size of the file information structure
*   \param ptFileInfo     File information structure
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_file_info(NETANA_HANDLE hDev, uint32_t ulFileInfoSize, NETANA_FILE_INFORMATION_T* ptFileInfo)
{
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
  NETANA_FILE_INFORMATION_T    tFileInfo = {0};
  uint32_t                     ulFile;

  if(!IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_NOT_ACTIVE;

  if(ptDevInst->ulFileCount == 0)
    return NETANA_FILECAPTURE_NOT_ACTIVE;

  tFileInfo.ulActualFileNr        = ptDevInst->ulCurrentFile + 1;
  tFileInfo.ulMaxFileCnt          = ptDevInst->ulFileCount;

  for(ulFile = 0; ulFile < ptDevInst->ulFileCount; ++ulFile)
  {
    tFileInfo.ullTotalBytesWritten  += ptDevInst->atFiles[ulFile].ullDataWritten;
  }

  tFileInfo.ullMaxBytes           = ptDevInst->ullFileSize * ptDevInst->ulFileCount;

  tFileInfo.ulRingBufferMode      = ptDevInst->fRingBuffer ? 1 : 0;

  OS_Memcpy( (void*)ptFileInfo, (void*)&tFileInfo, min((size_t)ulFileInfoSize, sizeof(tFileInfo)));

  return NETANA_NO_ERROR;
}

/*****************************************************************************/
/*! API management function
*   \param ulCommandId            Command to execute
*   \param ptInputParameter       Pointer to the input information
*   \param ulInputParameterSize   Size of the input information structure
*   \param ptOutputParameter Size Pointer to the output information
*   \param ulOutputParameterSize  Size of the output information structure
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_mngmt_exec_cmd( uint32_t      ulCommandId,
                               void*         ptInputParameter,
                               uint32_t      ulInputParameterSize,
                               void*         ptOutputParameter,
                               uint32_t      ulOutputParameterSize)
{
  int32_t lRet = NETANA_INVALID_PARAMETER;

  switch (ulCommandId)
  {
    case NETANA_MNGMT_CMD_BLINK:
    {
      if ( (ptInputParameter == NULL) || (ulInputParameterSize == 0) )
      {
        lRet = NETANA_INVALID_PARAMETER;
      } else
      {
        char*                        szDevice = (char*)ptInputParameter;
        struct NETANA_DEVINSTANCE_T* ptDevice = STAILQ_FIRST(&g_tCardList);

        lRet = NETANA_DEVICE_NOT_FOUND;

        OS_EnterLock(g_pvCardLock);
        /* Search for device */
        while(ptDevice != NULL)
        {
          if(0 == OS_Strnicmp( szDevice, ptDevice->szAliasName, sizeof(ptDevice->szAliasName)))
          {
            /* Device found */
            netana_tkit_ToggleBit( ptDevice, NETANA_HSK_HOST_BLINK_CMD);

            lRet = NETANA_NO_ERROR;

            break;

          } else
          {
            ptDevice = STAILQ_NEXT(ptDevice, tList);
          }
        }
        OS_LeaveLock(g_pvCardLock);
      }

    }
    break;

    case NETANA_MNGMT_CMD_DEV_SCAN:
    {
      if ( (ptInputParameter == NULL)  || (ulInputParameterSize  != sizeof(NETANA_MNGMT_DEV_SCAN_IN_T)) ||
           (ptOutputParameter == NULL) || (ulOutputParameterSize == 0) )
      {
        lRet = NETANA_INVALID_PARAMETER;
      } else
      {
        PNETANA_MNGMT_DEV_SCAN_IN_T  ptDevScanIn  = (PNETANA_MNGMT_DEV_SCAN_IN_T)ptInputParameter;
        PNETANA_MNGMT_DEV_SCAN_OUT_T ptDevScanOut = (PNETANA_MNGMT_DEV_SCAN_OUT_T)ptOutputParameter;
        struct NETANA_DEVINSTANCE_T* ptDevice     = STAILQ_FIRST(&g_tCardList);

        UNREFERENCED_PARAMETER( ptDevice);
        UNREFERENCED_PARAMETER( ptDevScanIn);

        ptDevScanOut->ulNofDevices = g_ulCardCnt;

        lRet = NETANA_NO_ERROR;
      }
    }
    break;

    case NETANA_MNGMT_CMD_GET_DEV_FEATURE:
    {
      NETANA_MNGMT_GET_DEV_FEATURE_IN_T*  ptFeatureIn = (NETANA_MNGMT_GET_DEV_FEATURE_IN_T*)ptInputParameter;
      struct NETANA_DEVINSTANCE_T*        ptDevInst   = NULL;
      NETANA_MNGMT_GET_DEV_FEATURE_OUT_T  tFeatures   = {0};
      NETANA_MNGMT_GET_DEV_FEATURE_OUT_T* ptFeatures  = (NETANA_MNGMT_GET_DEV_FEATURE_OUT_T*)ptOutputParameter;
      NETANA_BASE_DPM_T*                  ptDpm       = NULL;

      if ((ptFeatureIn == NULL) || (ulInputParameterSize != sizeof(NETANA_MNGMT_GET_DEV_FEATURE_IN_T)) ||
        (ptFeatureIn->hDev == NULL))
        return NETANA_INVALID_PARAMETER;

      ptDevInst = (struct NETANA_DEVINSTANCE_T*)ptFeatureIn->hDev;
      ptDpm     = (NETANA_BASE_DPM_T*)ptDevInst->pvDPM;

      tFeatures.ulStructVersion              = 0x00000000;
      tFeatures.ulPhysType                   = ptDpm->tSystemInfoBlock.ulPhysType;
      tFeatures.ulNumPhysPorts               = ptDevInst->ulPortCnt;
      tFeatures.ulPhysTapPresent             = ptDpm->tSystemInfoBlock.ulPhysTapPresent;
      tFeatures.ulPhysForwardingSupport      = ptDpm->tSystemInfoBlock.ulPhysForwardingSupport;
      tFeatures.ulPhysPortSpeedSupport       = ptDpm->tSystemInfoBlock.ulPhysPortSpeedSupport;
      tFeatures.ulPhysTransparentModeSupport = ptDpm->tSystemInfoBlock.ulPhysTransparentModeSupport;
      tFeatures.ulNumGpios                   = ptDevInst->ulGpioCnt;
      tFeatures.ulGpioInputRisingSupport     = ptDpm->tSystemInfoBlock.ulGpioInputRisingSupport;
      tFeatures.ulGpioInputFallingSupport    = ptDpm->tSystemInfoBlock.ulGpioInputFallingSupport;
      tFeatures.ulGpioOutputModeSupport      = ptDpm->tSystemInfoBlock.ulGpioOutputModeSupport;
      tFeatures.ulGpioOutputPWMSupport       = ptDpm->tSystemInfoBlock.ulGpioOutputPWMSupport;
      tFeatures.ulGpioSyncInSupport          = ptDpm->tSystemInfoBlock.ulGpioSyncInSupport;
      tFeatures.ulGpioTriggerStartSupport    = ptDpm->tSystemInfoBlock.ulGpioTriggerStartSupport;
      tFeatures.ulGpioTriggerStopSupport     = ptDpm->tSystemInfoBlock.ulGpioTriggerStopSupport;
      tFeatures.ulGpioVoltage3VSupport       = ptDpm->tSystemInfoBlock.ulGpioVoltage3VSupport;
      tFeatures.ulGpioVoltage24VSupport      = ptDpm->tSystemInfoBlock.ulGpioVoltage24VSupport;
      tFeatures.ulSyncSupport                = ptDpm->tSystemInfoBlock.ulSyncSupport;

      if (ulOutputParameterSize>sizeof(tFeatures))
        ulOutputParameterSize = sizeof(tFeatures);

      OS_Memcpy( ptFeatures, &tFeatures, ulOutputParameterSize);

      lRet = NETANA_NO_ERROR;
    }
    break;

    case NETANA_MNGMT_CMD_SET_DEV_CLASS_FILTER:
    {
      NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T* ptFilterIn = (NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T*)ptInputParameter;
      uint32_t                                ulInvMask  = (uint32_t)~(NETANA_DEV_CLASS_NANL_500 | NETANA_DEV_CLASS_NSCP_100 | NETANA_DEV_CLASS_CIFX);

      if ((ptFilterIn == NULL) || (ulInputParameterSize != sizeof(NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T)))
        return NETANA_INVALID_PARAMETER;

      if ( (0 != (ptFilterIn->ulDeviceClass & ulInvMask)) || (ptFilterIn->ulDeviceClass == 0))
      {
        lRet = NETANA_INVALID_PARAMETER;
      } else
      {
        lRet = USER_SetDevClassFilter( ptFilterIn->ulDeviceClass);
      }
    }
    break;

    default:
     lRet = NETANA_INVALID_PARAMETER;
     break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Synchonizes the netX clock
*   \param hDev             Device handle
*   \param ullReferenceTime Reference time in nanaoseconds                   */
/*****************************************************************************/
int32_t netana_resync_time( NETANA_HANDLE hDev, uint64_t ullReferenceTime)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if (!netana_tkit_WaitForBitState( ptDevInst,
                                    NETANA_HSK_HOST_RESYNC_CMD,
                                    NETANA_BITSTATE_EQUAL,
                                    NETANA_DEFAULT_TIMEOUT))
  {
    lRet = NETANA_FUNCTION_FAILED;

  } else
  {
    NETANA_HOST_CONTROL_BLOCK_T* ptHostCtrl = &ptDevInst->ptBaseDPM->tHostControlBlock;

    ptHostCtrl->ulRefSeconds     = (uint32_t)(ullReferenceTime >> 32);
    ptHostCtrl->ulRefNanoseconds = (uint32_t)(ullReferenceTime & 0xFFFFFFFF);

    netana_tkit_ToggleBit( ptDevInst, NETANA_HSK_HOST_RESYNC_CMD);

    if(!netana_tkit_WaitForBitState( ptDevInst,
                                     NETANA_HSK_NETX_RESYNC_ACK,
                                     NETANA_BITSTATE_EQUAL,
                                     NETANA_DEFAULT_TIMEOUT))
    {
      lRet = NETANA_FUNCTION_FAILED;
    } else
    {
      lRet = NETANA_NO_ERROR;
    }
  }
  return lRet;
}


/*****************************************************************************/
/*! Resynchonizes the netX PI-controller
*   \param hDev             Device handle
*   \param ulP
*   \param ulI                                                               */
/*****************************************************************************/
int32_t netana_config_pi_controller( NETANA_HANDLE hDev, uint32_t ulP, uint32_t ulI)
{
  int32_t                      lRet       = NETANA_NO_ERROR;
  struct NETANA_DEVINSTANCE_T* ptDevInst  = (struct NETANA_DEVINSTANCE_T*)hDev;
  NETANA_HOST_CONTROL_BLOCK_T* ptHostCtrl = &ptDevInst->ptBaseDPM->tHostControlBlock;

  ptHostCtrl->ulResyncP = ulP;
  ptHostCtrl->ulResyncI = ulI;

  return lRet;
}


/*****************************************************************************/
/*! Function restarts device
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
int32_t netana_restart_device( NETANA_HANDLE hDev)
{
  int32_t                      lRet      = NETANA_NO_ERROR;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_ACTIVE;

  if (NETANA_NO_ERROR == (lRet = netana_tkit_deviceremove( ptDevInst, 1)))
  {
    /* add the device */
    lRet = netana_tkit_deviceadd( ptDevInst, ptDevInst->ulDevNo);
  }
  if (lRet)
  {
    netana_close_device( hDev);
  }
  return lRet;
}


/*****************************************************************************/
/*! Function update ident info
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
int32_t netana_set_device_ident( NETANA_HANDLE hDev, char* szFileName)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(IsCaptureRunning(ptDevInst))
    return NETANA_CAPTURE_ACTIVE;

  ptDevInst->tIdentInfoFile.fIdentUpdateRequest = 0;
  ptDevInst->tIdentInfoFile.lError              = NETANA_NO_ERROR;
  if (szFileName)
  {
    lRet = NETANA_OUT_OF_MEMORY;
    if ((OS_Strncpy( ptDevInst->tIdentInfoFile.szIdentInfoFileName, szFileName, NETANA_MAX_FILEPATH)))
    {
      lRet                                          = NETANA_NO_ERROR;
      ptDevInst->tIdentInfoFile.fIdentUpdateRequest = 1;
    }
  }
  return lRet;
}


/*****************************************************************************/
/*! Function return device ident information
*   \param ptDevInstance Device instance
*   \param size                                                             */
/*****************************************************************************/
int32_t netana_get_device_ident( NETANA_HANDLE hDev, uint32_t ulInfoSize, NETANA_DEVICE_IDENT_T *ptIdentInfo)
{
  int32_t                      lRet      = NETANA_INVALID_POINTER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  if (ptIdentInfo)
  {
    if (ulInfoSize>sizeof(NETANA_DEVICE_IDENT_T))
    {
      lRet = NETANA_INVALID_BUFFERSIZE;
    } else
    {
      NETANA_BASE_DPM_T*    ptDpm = (NETANA_BASE_DPM_T*)ptDevInst->pvDPM;
      NETANA_DEVICE_IDENT_T tInfo;

      /* TODO: setup ulIdentVersion */
      tInfo.ulIdentVersion = 0;
      tInfo.ulDeviceNr     = ptDpm->tSystemInfoBlock.ulDeviceNr;
      tInfo.ulSerialNr     = ptDpm->tSystemInfoBlock.ulSerialNr;
      tInfo.usManufacturer = ptDpm->tSystemInfoBlock.usManufacturer;
      tInfo.ulFlags1       = ptDpm->tSystemInfoBlock.ulLicenseFlags1;
      tInfo.ulFlags2       = ptDpm->tSystemInfoBlock.ulLicenseFlags2;
      tInfo.usNetXid       = ptDpm->tSystemInfoBlock.usNetXLicenseID;
      tInfo.usNetXflags    = ptDpm->tSystemInfoBlock.usNetXLicenseFlags;

      OS_Memcpy( ptIdentInfo, &tInfo, ulInfoSize);
      lRet = NETANA_NO_ERROR;
    }
  }

  return lRet;
}


/****************************************************************************/
/*! Function validates ident information
*   \param ptDevInstance   Device instance
*   \param ulInfoSize      size of pbDevIdentInfo
*   \param pbDevIdentInfo pointer to crypted information                     */
/*****************************************************************************/
int32_t netana_check_device_ident( NETANA_HANDLE hDev, uint32_t ulInfoSize, uint8_t* pbDevIdentInfo)
{
  int32_t                      lRet       = NETANA_INVALID_POINTER;
  struct NETANA_DEVINSTANCE_T* ptDevInst  = (struct NETANA_DEVINSTANCE_T*)hDev;

  if (pbDevIdentInfo)
  {
    if (ulInfoSize>IDENT_INFO_BLOCK_SIZE)
    {
      lRet = NETANA_FUNCTION_FAILED;
    } else
    {
      if(!netana_tkit_WaitForBitState(ptDevInst,
                                      NETANA_HSK_HOST_LICENSE_CMD,
                                      NETANA_BITSTATE_EQUAL,
                                      NETANA_DEFAULT_TIMEOUT))
      {
        lRet = NETANA_FUNCTION_FAILED;

      } else
      {
        OS_Memcpy( (void*)ptDevInst->tIdentInfoBlock.pbIdentInfoBlock, (void*)pbDevIdentInfo, ulInfoSize);

        /* signal updated license info */
        netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_LICENSE_CMD);

        if(!netana_tkit_WaitForBitState(ptDevInst,
                                        NETANA_HSK_HOST_LICENSE_CMD,
                                        NETANA_BITSTATE_EQUAL,
                                        NETANA_DEFAULT_TIMEOUT))
        {
          lRet = NETANA_FUNCTION_FAILED;
        } else
        {
          /* copy back the updated information */
          OS_Memcpy( (void*)pbDevIdentInfo, (void*)ptDevInst->tIdentInfoBlock.pbIdentInfoBlock, ulInfoSize);
          lRet = NETANA_NO_ERROR;
        }
      }
    }
  }
  return lRet;
}

/****************************************************************************/
/*! Function selects the different voltage settings
*   \param hDev             Handle to device
*   \param ulGpioSelMask    Mask of GPIOs to be affected
*   \param ulGpioVoltageSel Voltage selection
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_set_gpio_voltage( NETANA_HANDLE hDev, uint32_t ulGpioSelMask, uint32_t ulGpioVoltageSel)
{
  int32_t                      lRet            = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst       = (struct NETANA_DEVINSTANCE_T*)hDev;
  NETANA_BASE_DPM_T*           ptDpm           = (NETANA_BASE_DPM_T*)ptDevInst->pvDPM;
  uint32_t                     ulGpio          = 0;
  uint32_t                     ulSupportedMask = 0;

  /* parameter check */
  for (ulGpio=0;ulGpio<ptDevInst->ulGpioCnt;ulGpio++)
  {
    ulSupportedMask |= (1<<ulGpio);
  }
  if (ulGpioSelMask > ulSupportedMask)
    return NETANA_INVALID_PARAMETER;

  /* NANL only supports only 0x0F */
  if (ptDevInst->usDeviceClass == NANL_DEV_CLASS)
  {
    if (ulGpioSelMask != 0x0F)
      return NETANA_INVALID_PARAMETER;
  }
  switch (ulGpioVoltageSel)
  {
    case NETANA_GPIO_VOLTAGE_3V:
      if ((ptDpm->tSystemInfoBlock.ulGpioVoltage3VSupport & ulGpioSelMask) == ulGpioSelMask)
        lRet = NETANA_NO_ERROR;
      break;
    case NETANA_GPIO_VOLTAGE_24V:
      if ((ptDpm->tSystemInfoBlock.ulGpioVoltage24VSupport & ulGpioSelMask) == ulGpioSelMask)
        lRet = NETANA_NO_ERROR;
      break;
    default:
      break;
  }
  if (lRet == NETANA_NO_ERROR)
  {
    for (ulGpio=0;ulGpio<ptDevInst->ulGpioCnt;ulGpio++)
    {
      if (ulGpioSelMask & (1<<ulGpio))
      {
        NETANA_GPIO_BLOCK_T* ptBlock = ptDevInst->aptGPIOs[ulGpio];

        ptBlock->ulVoltage = ulGpioVoltageSel;
      }
      lRet = NETANA_NO_ERROR;
    }
  }
  return lRet;
}

/****************************************************************************/
/*! Function returns the selected voltage settings
*   \param hDev              Handle to device
*   \param ulGpio            Number of the requested GPIO
*   \param pulGpioVoltageSel Pointer to returned voltage setting
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_get_gpio_voltage( NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t* pulGpioVoltageSel)
{
  int32_t                      lRet      = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
  NETANA_GPIO_BLOCK_T*         ptBlock   = NULL;

  if ((pulGpioVoltageSel == NULL) || (ulGpio >= ptDevInst->ulGpioCnt))
    return lRet;

  ptBlock = ptDevInst->aptGPIOs[ulGpio];

  *pulGpioVoltageSel = ptBlock->ulVoltage;

  return NETANA_NO_ERROR;
}

/*****************************************************************************/
/*! Sends a Packet to the device
 *   \param hDev         Handle to device
 *   \param ptSendPkt    Packet to send
 *   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
 *   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t put_packet( NETANA_HANDLE hDev, NETANA_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  int32_t                      lRet      = NETANA_DEV_MAILBOX_FULL;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  /* Check if packet fits into the mailbox */
  if( (LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + NETANA_PACKET_HEADER_SIZE) > ptDevInst->tSendMbx.ulSendMailboxLength)
    return NETANA_DEV_MAILBOX_TOO_SHORT;

  if(netana_tkit_WaitForBitState(ptDevInst, ptDevInst->tSendMbx.bSendCMDBitoffset, NETANA_BITSTATE_EQUAL, ulTimeout))
  {
    /* Copy packet to mailbox */
    ++ptDevInst->tSendMbx.ulSendPacketCnt;
    OS_Memcpy( ptDevInst->tSendMbx.ptSendMailboxStart->abSendMailbox,
               ptSendPkt,
               LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + NETANA_PACKET_HEADER_SIZE);

    /* Signal new packet */
    netana_tkit_ToggleBit(ptDevInst, ptDevInst->tSendMbx.bSendCMDBitoffset);

    lRet = NETANA_NO_ERROR;
  }
  return lRet;
}

/*****************************************************************************/
/*! Retrieves a Packet from the device
 *   \param hDev             Handle to device
 *   \param ptRecvPkt        Pointer to place received Packet in
 *   \param ulRecvBufferSize Length of the receive buffer
 *   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
 *   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t get_packet( NETANA_HANDLE hDev, NETANA_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout)
{
  int32_t                      lRet       = NETANA_NO_ERROR;
  uint32_t                     ulCopySize = 0;
  NETANA_PACKET*               ptPacket   = NULL;
  struct NETANA_DEVINSTANCE_T* ptDevInst  = (struct NETANA_DEVINSTANCE_T*)hDev;

  if(!netana_tkit_WaitForBitState(ptDevInst, ptDevInst->tRecvMbx.bRecvACKBitoffset, NETANA_BITSTATE_UNEQUAL, ulTimeout))
    return NETANA_DEV_GET_NO_PACKET;

  ++ptDevInst->tRecvMbx.ulRecvPacketCnt;

  ptPacket   = (NETANA_PACKET*)ptDevInst->tRecvMbx.ptRecvMailboxStart->abRecvMailbox;
  ulCopySize = LE32_TO_HOST(ptPacket->tHeader.ulLen) + NETANA_PACKET_HEADER_SIZE;
  if(ulCopySize > ulRecvBufferSize)
  {
    /* We have to free the mailbox, read as much as possible */
    ulCopySize = ulRecvBufferSize;
    lRet = NETANA_BUFFER_TOO_SHORT;
  }
  OS_Memcpy(ptRecvPkt, ptPacket, ulCopySize);

  /* Signal read packet done */
  netana_tkit_ToggleBit(ptDevInst, ptDevInst->tRecvMbx.bRecvACKBitoffset);
  return lRet;
}

/*****************************************************************************/
/*! Returns the actual state of the device mailbox
 *   \param hDev           Handle to device
 *   \param pulRecvPktCnt  Number of pending packets to receive
 *   \param pulSendPktCnt  Number of packets that can be sent to the device
 *   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_get_mbx_state(NETANA_HANDLE hDev, uint32_t* pulRecvPktCnt, uint32_t* pulSendPktCnt)
{
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
  int32_t                      lRet      = NETANA_NO_ERROR;

  if(ptDevInst->tRecvMbx.ulRecvMailboxLength == 0)
  {
    lRet = NETANA_FUNCTION_NOT_AVAILABLE;
  } else
  {
    if (NULL != pulRecvPktCnt)
    {
      /* Get receive MBX state */
      *pulRecvPktCnt = LE16_TO_HOST(ptDevInst->tRecvMbx.ptRecvMailboxStart->usWaitingPackages);
    }
    if (NULL != pulSendPktCnt)
    {
      /* Get send MBX state */
      *pulSendPktCnt = LE16_TO_HOST(ptDevInst->tSendMbx.ptSendMailboxStart->usPackagesAccepted);
    }
  }
  return lRet;
}

/*****************************************************************************/
/*! Inserts a packet into the device mailbox
 *   \param hDev       Handle to device
 *   \param ptSendPkt  Packet to send to channel
 *   \param ulTimeout  Time in ms to wait for card to accept the packet
 *   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_put_packet(NETANA_HANDLE hDev, NETANA_PACKET*  ptSendPkt, uint32_t ulTimeout)
{
  int32_t                      lRet      = NETANA_NO_ERROR;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  /* Check if another command is active */
  if (0 == OS_WaitMutex( ptDevInst->tSendMbx.pvSendMBXMutex, ulTimeout))
    return NETANA_DRV_CMD_ACTIVE;

  lRet = put_packet(ptDevInst, ptSendPkt, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptDevInst->tSendMbx.pvSendMBXMutex);

  return lRet;
}

/*****************************************************************************/
/*! Gets a packet from the device mailbox
 *   \param hDev       Handle to device
 *   \param ulSize     Size of the return packet buffer
 *   \param ptRecvPkt  Returned packet
 *   \param ulTimeout  Time in ms to wait for available message
 *   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_get_packet(NETANA_HANDLE hDev, uint32_t ulSize, NETANA_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t                      lRet      = NETANA_NO_ERROR;
  struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;

  /* Check if another command is active */
  if (0 == OS_WaitMutex( ptDevInst->tRecvMbx.pvRecvMBXMutex, ulTimeout))
    return NETANA_DRV_CMD_ACTIVE;

  lRet = get_packet(ptDevInst, ptRecvPkt, ulSize, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptDevInst->tRecvMbx.pvRecvMBXMutex);

  return lRet;
}
