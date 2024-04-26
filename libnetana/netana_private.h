/**************************************************************************************

Copyright (c) Hilscher GmbH. All Rights Reserved.

**************************************************************************************

Filename:
$Workfile: netana_private.h $
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
#ifndef __NETANA_PRIV__H
#define __NETANA_PRIV__H

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <string.h>
//#include <dirent.h>
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "sys/queue.h"
#include "netana_user.h"
#include "netana_errors.h"

#ifndef NULL
#define NULL  ((void*)0)
#endif

#ifndef min
#define min(a,b)  ((a < b) ? a : b)
#endif

#define UNREFERENCED_PARAMETER(a)  (a = a)

#define NETANA_LIB_VERSION_MAJOR    0
#define NETANA_LIB_VERSION_MINOR    9
#define NETANA_LIB_VERSION_BUILD    0
#define NETANA_LIB_VERSION_REVISION 0

#define NETANA_MAX_FILES              256
#define NETANA_MAX_FILEPATH           256

#define SYSFS_PATH        "/sys/class/netanalyzer/"
#define SYSFS_DEVICE_PATH "/sys/class/netanalyzer/netanalyzer_"
#define SYSFS_ERROR_PATH  "/sys/class/netanalyzer/driver_information/driver_information/error" //TODO: set to driver path not to device path

#define NETANA_TRACELEVEL_ERROR   1
#define NETANA_TRACELEVEL_WARNING 2
#define NETANA_TRACELEVEL_INFO    3
#define NETANA_TRACELEVEL_DEBUG   4

#define OPEN_READ      1
#define OPEN_WRITE     2
#define OPEN_READWRITE 3


#define NETANA_DEFAULT_DMABUFFERS       8
#define NETANA_DEFAULT_DMABUFFER_SIZE   (512 * 1024)

typedef struct NETANA_FILE_DATA_Ttag
{
        void*    hFile;
        uint64_t ullOffset;
        uint64_t ullDataWritten;

} NETANA_FILE_DATA_T;

typedef struct NETANA_THREAD_PRIO_Ttag
{
        int fThreadPrioSettings;
        int iPolicy;
        int iPriority;

} NETANA_THREAD_PRIO_T;

struct NETANA_DEVINSTANCE_T
{
        STAILQ_ENTRY(NETANA_DEVINSTANCE_T) tList;
        char                               szDeviceName[NETANA_MAX_DEVICENAMESIZE];
        int                                fd_Device;
        int                                iDeviceNum;
        uint32_t                           ulMaxFileCount;
        uint32_t                           ulDMABufferSize;
        uint32_t                           ulDMABufferCount;
        pthread_t                          hPollingThread;
        int                                fRunEventThread;
        int                                iEventError;
        NETANA_THREAD_PRIO_T               tEventThreadPrio;
        PFN_STATUS_CALLBACK                pfnStatus;
        PFN_DATA_CALLBACK                  pfnData;
        void*                              pvUser;
        uint32_t                           ulCaptureState;
        int                                fRingBuffer;
        uint32_t                           ulFileCount;
        uint64_t                           ullFileSize;
        uint32_t                           ulCurrentBuffer;
        uint32_t                           ulCurrentFile;
        int                                fWriteToFile;
        NETANA_FILE_HEADER_T               tFileHeader;
        NETANA_FILE_DATA_T                 atFiles[NETANA_MAX_FILES];

        char                               szPath[NETANA_MAX_FILEPATH];
        char                               szBaseFilename[NETANA_MAX_FILEPATH];
        FILE                               *log_file;
};

STAILQ_HEAD(NETANA_CARD_LIST, NETANA_DEVINSTANCE_T);

int32_t  netana_writefileheader(struct NETANA_DEVINSTANCE_T* ptDevInst, uint64_t ullSize);
uint32_t netana_write_to_file  (struct NETANA_DEVINSTANCE_T* ptDevInst, void*    buffer, uint32_t buffersize);

void USER_Trace( struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulTraceLevel, const char* szFormat, ...);
int  create_log_file( struct NETANA_DEVINSTANCE_T* ptDevInst);
void close_log_file ( struct NETANA_DEVINSTANCE_T* ptDevInst);

#endif //__NETANA_PRIV__H
