/* SPDX-License-Identifier: MIT */

#include "netana_toolkit.h"

#ifndef min
  #define min(a,b)  ((a < b) ? a : b)
#endif

int32_t netana_signal_driver_error(struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulError)
{
  int32_t            lRet  = NETANA_NO_ERROR;
  NETANA_BASE_DPM_T* ptDPM = ptDevInst->ptBaseDPM;

   /* Signal firmware that we had a driver error and need to stop */
  if(!netana_tkit_WaitForBitState(ptDevInst,
                                  NETANA_HSK_HOST_DRV_ERROR_CMD,
                                  NETANA_BITSTATE_EQUAL,
                                  NETANA_DEFAULT_TIMEOUT))
  {
    /* Fatal Error, as we cannot signal firmware, we ran into a driver problem (e.g. file full) */
    if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
    {
      USER_Trace(ptDevInst,
                 NETANA_TRACELEVEL_ERROR,
                 "Unable to signal driver error (0x%08X) to firmware. Flags did not become equal during timeout!",
                 ulError);
    }
    lRet = NETANA_FUNCTION_FAILED;
  } else
  {
    ptDPM->tHostControlBlock.ulHostError = ulError;
    netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_DRV_ERROR_CMD);
  }
  return lRet;
}

int32_t netana_tkit_writefileheader(struct NETANA_DEVINSTANCE_T* ptDevInst, uint64_t ullSize)
{
  int32_t               lRet         = NETANA_FILE_WRITE_FAILED;
  uint32_t              ulFile       = ptDevInst->ulCurrentFile;
  NETANA_FILE_DATA_T*   ptFileData   = &ptDevInst->atFiles[ulFile];
  NETANA_FILE_HEADER_T* ptFileHeader = &ptDevInst->tFileHeader;

  if(ptFileData->ullOffset != 0)
  {
    OS_FileSeek(ptFileData->hFile, 0);
    ptFileData->ullOffset = 0;
  }

  ptFileHeader->ullDataSize = ullSize;

  if(NETANA_FILE_HEADER_SIZE_INVALID == ullSize)
    ptFileData->ullDataWritten = 0;
  else
    ptFileData->ullDataWritten = ullSize;

  if( sizeof(*ptFileHeader) == OS_FileWrite(ptFileData->hFile,
                                            sizeof(*ptFileHeader),
                                            ptFileHeader))
  {
    lRet = NETANA_NO_ERROR;
    ptFileData->ullOffset += sizeof(*ptFileHeader);
  }

  return lRet;
}

void netana_tkit_process(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
  uint32_t           ulUnequalFlags = ptDevInst->ulNetxFlags ^ ptDevInst->ulHostFlags;
  NETANA_BASE_DPM_T* ptDPM          = ptDevInst->ptBaseDPM;

  if(ulUnequalFlags & NETANA_HSK_HOST_NEWDATA_IND_MSK)
  {
    /* New data is available */
    uint32_t ulBufferMask  = (1 << ptDevInst->ulCurrentBuffer);
    uint32_t ulUsedBuffers;

    /* Free new Data Indication again */
    netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_NEWDATA_IND);

    ulUsedBuffers = ptDPM->tHostControlBlock.ulBufferFlags ^
                    ptDPM->tNetxStatusBlock.ulBufferFlags;

    while(ulUsedBuffers & ulBufferMask)
    {
      uint32_t            ulBuffer         = ptDevInst->ulCurrentBuffer;
      NETANA_DMABUFFER_T* ptDMABuffer      = &ptDevInst->atDMABuffers[ulBuffer];
      uint32_t            ulUsedBufferSize = ptDPM->tNetxStatusBlock.aulUsedBufferSize[ulBuffer];

      if(ulUsedBufferSize > ptDMABuffer->ulBufferSize)
      {
        /* ATTENTION: This must be a firmware bug as it indicates more data as we can
                      handle. We can only reject this buffer */

        netana_signal_driver_error(ptDevInst, NETANA_CAPTURE_ERROR_DRIVER_INVALID_BUFFERSIZE);

        if(ptDevInst->fWriteToFile)
        {
          uint32_t            ulFile     = ptDevInst->ulCurrentFile;
          NETANA_FILE_DATA_T* ptFileData = &ptDevInst->atFiles[ulFile];

          /* Write File header into current file */
          netana_tkit_writefileheader(ptDevInst, ptFileData->ullOffset -
                                                 sizeof(NETANA_FILE_HEADER_T));
          ptDevInst->fWriteToFile = 0;
        }

        if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR)
        {
          USER_Trace(ptDevInst,
                     NETANA_TRACELEVEL_ERROR,
                     "Firmware passed illegal buffer size of %u bytes, for buffer %u. Rejecting buffer and aborting capture!",
                     ulUsedBufferSize, ulBuffer);
        }

      } else
      {
        /* Capture to file is active */
        if(ptDevInst->fWriteToFile)
        {
          uint32_t              ulFile     = ptDevInst->ulCurrentFile;
          NETANA_FILE_DATA_T* ptFileData = &ptDevInst->atFiles[ulFile];

          if( (ptFileData->ullOffset + ulUsedBufferSize) > ptDevInst->ullFileSize)
          {
            /* Wrap around needed */

            /* Write File header into current file */
            netana_tkit_writefileheader(ptDevInst, ptFileData->ullOffset -
                                                   sizeof(NETANA_FILE_HEADER_T));
            ++ptDevInst->tFileHeader.ulIdx;

            if(++ptDevInst->ulCurrentFile >= ptDevInst->ulFileCount)
            {
              if(ptDevInst->fRingBuffer)
              {
                /* Ring buffer mode starts again at first file */
                ptDevInst->ulCurrentFile = 0;

                /* Switch and rewind file */
                ptFileData = &ptDevInst->atFiles[ptDevInst->ulCurrentFile];

                /* Write header to new file */
                netana_tkit_writefileheader(ptDevInst, NETANA_FILE_HEADER_SIZE_INVALID);

                /* Write Data */
                OS_FileWrite(ptFileData->hFile, ulUsedBufferSize, ptDMABuffer->pvBufferAddress);
                ptFileData->ullOffset       += ulUsedBufferSize;
                ptFileData->ullDataWritten  += ulUsedBufferSize;


              } else
              {
                netana_signal_driver_error(ptDevInst, NETANA_CAPTURE_ERROR_DRIVER_FILE_FULL);

                ptDevInst->fWriteToFile = 0;

                /* Adjust current capture file for netana_file_info to return correct file number */
                --ptDevInst->ulCurrentFile;
              }
            } else
            {
              /* Next file has been automatically started */
              ulFile     = ptDevInst->ulCurrentFile;
              ptFileData = &ptDevInst->atFiles[ulFile];

              /* Write header to new file */
              netana_tkit_writefileheader(ptDevInst, NETANA_FILE_HEADER_SIZE_INVALID);

              /* Write Data */
              OS_FileWrite(ptFileData->hFile, ulUsedBufferSize, ptDMABuffer->pvBufferAddress);
              ptFileData->ullOffset       += ulUsedBufferSize;
              ptFileData->ullDataWritten  += ulUsedBufferSize;
            }

          } else
          {
            /* Normal file write */
            OS_FileWrite(ptFileData->hFile, ulUsedBufferSize, ptDMABuffer->pvBufferAddress);
            ptFileData->ullOffset       += ulUsedBufferSize;
            ptFileData->ullDataWritten  += ulUsedBufferSize;
          }
        }

        /* Callback user */
        if(ptDevInst->pfnData)
        {
          ptDevInst->pfnData(ptDMABuffer->pvBufferAddress,
                             ulUsedBufferSize,
                             ptDevInst->pvUser);
        }
      }

      /* Free buffer for firmware and driver again */
      ptDPM->tHostControlBlock.ulBufferFlags ^= ulBufferMask;
      ulUsedBuffers &= ~ulBufferMask;

      if(++ptDevInst->ulCurrentBuffer >= ptDevInst->ulDMABufferCount)
        ptDevInst->ulCurrentBuffer = 0;

      ulBufferMask = 1 << ptDevInst->ulCurrentBuffer;
    }
  }

  if(ulUnequalFlags & NETANA_HSK_HOST_NEWSTATUS_IND_MSK)
  {
    uint32_t ulCaptureState = ptDPM->tNetxStatusBlock.ulCaptureState;
    uint32_t ulCaptureError = ptDPM->tNetxStatusBlock.ulCaptureError;

    if(ptDevInst->pfnStatus)
    {
      /* Status callback */
      ptDevInst->pfnStatus(ulCaptureState, ulCaptureError, ptDevInst->pvUser);
    }

    netana_tkit_ToggleBit(ptDevInst, NETANA_HSK_HOST_NEWSTATUS_IND);

    if(ulCaptureState == NETANA_CAPTURE_STATE_OFF)
      OS_SetEvent(ptDevInst->hStopComplete);
  }
}
