/**************************************************************************************

Copyright (c) Hilscher GmbH. All Rights Reserved.

**************************************************************************************

Filename:
$Workfile: netana_capture.c $
Last Modification:
$Author: sebastiand $
$Modtime: 30.09.09 14:10 $
$Revision: 3529 $

Targets:
Linux        : yes

Description:


Changes:

Version   Date        Author   Description
----------------------------------------------------------------------------------
1        17.09.2012   SD       - Initial version

**************************************************************************************/
#include <stdio.h>
#include <string.h>

#include "netana_private.h"

extern uint32_t g_ulTraceLevel;

int32_t netana_writefileheader(struct NETANA_DEVINSTANCE_T* ptDevInst, uint64_t ullSize)
{
        int32_t               lRet         = NETANA_FILE_WRITE_FAILED;
        uint32_t              ulFile       = ptDevInst->ulCurrentFile;
        NETANA_FILE_DATA_T*   ptFileData   = &ptDevInst->atFiles[ulFile];
        NETANA_FILE_HEADER_T* ptFileHeader = &ptDevInst->tFileHeader;

        if(ptFileData->ullOffset != 0){
                fseek(ptFileData->hFile, 0, 0);
                ptFileData->ullOffset = 0;
        }
        ptFileHeader->ullDataSize = ullSize;

        if(NETANA_FILE_HEADER_SIZE_INVALID == ullSize)
                ptFileData->ullDataWritten = 0;
        else
                ptFileData->ullDataWritten = ullSize;

        if( sizeof(*ptFileHeader) == fwrite( ptFileHeader, 1, sizeof(*ptFileHeader), ptFileData->hFile)){
                lRet = NETANA_NO_ERROR;
                ptFileData->ullOffset += sizeof(*ptFileHeader);
        }
        return lRet;
}

uint32_t netana_write_to_file(struct NETANA_DEVINSTANCE_T* ptDevInst, void* pvBuffer, uint32_t ulBufferSize)
{
        /* Capture to file is active */
        uint32_t              ulRet    = NETANA_NO_ERROR;
        uint32_t              ulFile   = ptDevInst->ulCurrentFile;
        NETANA_FILE_DATA_T* ptFileData = &ptDevInst->atFiles[ulFile];

        if( (ptFileData->ullOffset + ulBufferSize) > ptDevInst->ullFileSize)
        {
                /* Wrap around needed */
                /* Write File header into current file */
                netana_writefileheader(ptDevInst, ptFileData->ullOffset - sizeof(NETANA_FILE_HEADER_T));
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
                                netana_writefileheader(ptDevInst, NETANA_FILE_HEADER_SIZE_INVALID);

                                /* Write Data */
                                fwrite( pvBuffer, 1, ulBufferSize, ptFileData->hFile);
                                ptFileData->ullOffset       += ulBufferSize;
                                ptFileData->ullDataWritten  += ulBufferSize;


                        } else
                        {
                                ulRet = NETANA_CAPTURE_ERROR_DRIVER_FILE_FULL;

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
                        netana_writefileheader(ptDevInst, NETANA_FILE_HEADER_SIZE_INVALID);

                        /* Write Data */
                        fwrite( pvBuffer, 1, ulBufferSize, ptFileData->hFile);
                        ptFileData->ullOffset       += ulBufferSize;
                        ptFileData->ullDataWritten  += ulBufferSize;
                }

        } else
        {
                /* Normal file write */
                fwrite( pvBuffer, 1, ulBufferSize, ptFileData->hFile);
                ptFileData->ullOffset       += ulBufferSize;
                ptFileData->ullDataWritten  += ulBufferSize;
        }
        return ulRet;
}
