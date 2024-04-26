/*************************************************************************************

        Copyright (c) Hilscher GmbH. All Rights Reserved.

**************************************************************************************

        Filename:
                $Workfile: netana_functions.c $
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
                4        02.06.2015   SD       - add mailbox API functions:netana_put_packet(),
                                                 netana_get_packet(),netana_get_mbx_state()
                3        18.02.2014   SD       - netana_close_device() did not delete internal device handle
                2        05.02.2014   SD       - netana_set/get_filter(): validate size of binary file
                                                 before retrieving kernel driver error
                1        17.09.2012   SD       - Initial version

**************************************************************************************/
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <pthread.h>
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "netana_private.h"
#include "libnetana.h"

#define DRV_MAJOR 1
#define DRV_MINOR 0
#define DRV_BUILD 2
#define DRV_REV   0

int32_t CheckDeviceHandle(NETANA_HANDLE);
#define CHECK_HANDLE(x) if (CheckDeviceHandle(x) != NETANA_NO_ERROR) return CheckDeviceHandle(x);

void libinit(void);
__attribute__((constructor)) void libinit(void);

extern uint32_t         g_ulTraceLevel;
int32_t                 g_lTimezoneCorrection;
uint32_t                g_ulDMABufferSize  = NETANA_DEFAULT_DMABUFFER_SIZE;
uint32_t                g_ulDMABufferCount = NETANA_DEFAULT_DMABUFFERS;

struct NETANA_CARD_LIST g_tCardList     = STAILQ_HEAD_INITIALIZER(g_tCardList);
uint32_t                g_ulCardCnt     = 0;
pthread_mutex_t         g_tCardListLock = {{0}};


static void    CloseCaptureFiles    ( struct NETANA_DEVINSTANCE_T* ptDevInst);
static int32_t netana_get_state_poll( NETANA_HANDLE hDev, uint32_t* pulCaptureState, uint32_t* pulCaptureError);

int g_fDriverVersionMismatch = 0;
int g_fDriverRunning         = 0;
int g_libInitError           = 0;

extern NETANA_THREAD_PRIO_T g_tKThreadSetting;
extern NETANA_THREAD_PRIO_T g_tThreadSetting;

//TODO: insert more user traces

static void check_driver(void)
{
    NETANA_DRIVER_INFORMATION_T tDrvInfo;

    int32_t lRet = netana_driver_information(sizeof(tDrvInfo), &tDrvInfo);

    if(NETANA_DRIVER_NOT_RUNNING == lRet) {
            g_fDriverRunning = 0;
            g_fDriverVersionMismatch = 0;
    } else if(NETANA_NO_ERROR == lRet) {
            g_fDriverRunning = 1;
            g_ulDMABufferSize  = tDrvInfo.ulDMABufferSize;
            g_ulDMABufferCount = tDrvInfo.ulDMABufferCount;
    } else {
            g_fDriverRunning = 1;
            fprintf(stderr, "Error while retrieving driver's DMA " \
                    "configuration! Try using default. " \
                    "(DMA-Cnt: %d / DMA-Size: %d)\n",
                    g_ulDMABufferCount,
                    g_ulDMABufferSize);
    }

    if(g_fDriverRunning) {
            if ((tDrvInfo.ulVersionMajor<DRV_MAJOR) || (((tDrvInfo.ulVersionMajor==DRV_MAJOR)) && (tDrvInfo.ulVersionMinor<DRV_MINOR))) {
                    fprintf(stderr, "This is not the correct driver version! -> netAnalyzer Kernel-Mode Driver: %d.%d.%d.%d\n",
                            tDrvInfo.ulVersionMajor,
                            tDrvInfo.ulVersionMinor,
                            tDrvInfo.ulVersionBuild,
                            tDrvInfo.ulVersionRevision);
                    fprintf(stderr, "At least required: %d.%d.%d.%d\n", DRV_MAJOR, DRV_MINOR, DRV_BUILD, DRV_REV);
                    g_fDriverVersionMismatch = 1;
            }
    }
}

/*****************************************************************************/
/*! Initialization of gloabal driver information                             */
/*****************************************************************************/
void libinit(void)
{
        pthread_mutexattr_t          mta;
        int                          iRet;

        g_lTimezoneCorrection = 0;
        g_ulTraceLevel        = 0x00;

        pthread_mutexattr_init(&mta);
        if( (iRet = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE)) != 0 )
        {
                fprintf( stderr, "Mutex set attr: %s\n", strerror(iRet));
                g_libInitError = 1;
        } else if ((iRet = pthread_mutex_init( &g_tCardListLock, &mta)) != 0 )
        {
                fprintf( stderr, "Mutex set attr: %s\n", strerror(iRet));
                g_libInitError = 1;
        }
}

/*****************************************************************************/
/*! Function validates handle
*   \param hDev handle given by user
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t CheckDeviceHandle(NETANA_HANDLE hDev)
{
  int32_t                      ret   = NETANA_INVALID_PARAMETER;
  struct NETANA_DEVINSTANCE_T* ptDev = (struct NETANA_DEVINSTANCE_T*)hDev;
  struct NETANA_DEVINSTANCE_T* ptTmp = NULL;

  if (ptDev == NULL)
          return ret;

  ret = NETANA_INVALID_HANDLE;

  if (0 == pthread_mutex_lock(&g_tCardListLock)) {
          STAILQ_FOREACH( ptTmp, &g_tCardList, tList) {
                  if (ptTmp == ptDev) {
                          ret = NETANA_NO_ERROR;
                          break;
                  }
          }
          pthread_mutex_unlock(&g_tCardListLock);
  }
  return ret;
}

void* FileOpen(char* szFilename, uint64_t * pulFileSize, uint32_t ulFlags)
{
        int         fd;
        struct stat buf;

        fd = open(szFilename, ulFlags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if( fd == -1 )
                return NULL;

        if( fstat(fd, &buf) != 0 )
                return NULL;

        if (buf.st_size < *pulFileSize)
                posix_fallocate( fd, buf.st_size, *pulFileSize);

        return fdopen(fd, "w+");
}

/*****************************************************************************/
/*! Event-Thread
*   Polls the device and reads the incoming data or state changes.
*   The function calls the corresponding callbacks (pfnData/pfnStatus).
*   \param arg  pointer to device instance
*   \return NULL                                                             */
/*****************************************************************************/
static void* netana_event_thread(void *arg)
{
        struct pollfd                tpollfd;
        nfds_t                       nfds           = 1;
        struct NETANA_DEVINSTANCE_T* ptDevInst      = (struct NETANA_DEVINSTANCE_T*)arg;
        char*                        pbReadBuf      = malloc( ptDevInst->ulDMABufferSize);
        int                          iRet           = 0;
        int                          iTimeout       = 50;
        ssize_t                      buffersize     = 0;
        uint32_t                     ulCaptureError = 0;
        uint32_t                     ulCaptureState = NETANA_CAPTURE_STATE_OFF;
        uint32_t                     ulWriteError   = NETANA_NO_ERROR;

        /* check if user defined thread priority is set */
        if (ptDevInst->tEventThreadPrio.fThreadPrioSettings) {
                struct sched_param tSched_priority = {.sched_priority = ptDevInst->tEventThreadPrio.iPriority};

                if (0 != pthread_setschedparam( ptDevInst->hPollingThread, ptDevInst->tEventThreadPrio.iPolicy, &tSched_priority)) {
                        if(g_ulTraceLevel & NETANA_TRACELEVEL_INFO) {
                                USER_Trace( ptDevInst, NETANA_TRACELEVEL_INFO, "Error setting capture thread priority (error:%d)! Running with parent settings!\n", errno);
                        }
                }
        }
        tpollfd.fd     = ptDevInst->fd_Device;
        tpollfd.events = POLLPRI | POLLRDNORM;

        if (!pbReadBuf)
        {
                if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR) {
                        USER_Trace( ptDevInst, NETANA_TRACELEVEL_ERROR, "Stop cyclic thread! Not enough memory for reading notified data!\n");
                }
                ptDevInst->fRunEventThread = 0;
                ptDevInst->iEventError     = -ENOMEM;
        }
        while(ptDevInst->fRunEventThread || (ulCaptureState != NETANA_CAPTURE_STATE_OFF)) {
                if (0<=(iRet = poll( &tpollfd, nfds, iTimeout))) {
                        /* no timeout */
                        if (0 != iRet) {
                                if (tpollfd.revents & POLLPRI) {
                                        /* NOTE: The precession of this thread execution depends basicaly on two items: */
                                        /*       - system thread scheduling (algorithm and priority)                    */
                                        /*       - time consumption in the users callback (ptDevInst->pfnData)          */
                                        /*       Depending on the systems load it might be possible that the kernel     */
                                        /*       will signal more than one dma buffer to read.                          */
                                        /*       To minimize the over head of the user/kernel space switches always try */
                                        /*       to read as much as possible.                                           */
                                        /*       (Under heavy load a new data block will be available every ~10ms )     */
                                        do {
                                                if (0<(iRet = read( ptDevInst->fd_Device, pbReadBuf, ptDevInst->ulDMABufferSize))) {
                                                        buffersize = (ssize_t)iRet;
                                                        if (ptDevInst->fWriteToFile)
                                                                ulWriteError = netana_write_to_file( ptDevInst, pbReadBuf, buffersize);

                                                        if (ptDevInst->pfnData) {
                                                                ptDevInst->pfnData( pbReadBuf, buffersize, ptDevInst->pvUser);
                                                        }
                                                }
                                        /* in case of no data -ENODATA is returned                                     */
                                        /* in case of -EAGAIN try to read again since the data block is in preparation */
                                        } while((0 == iRet) || (-EAGAIN == iRet));
                                }
                                if (tpollfd.revents & POLLRDNORM) {
                                        if (NETANA_NO_ERROR == netana_get_state_poll( (NETANA_HANDLE)ptDevInst, &ulCaptureState, &ulCaptureError)) {
                                                if (ptDevInst->pfnStatus)
                                                        ptDevInst->pfnStatus( ulCaptureState, ulCaptureError, ptDevInst->pvUser);

                                                ptDevInst->ulCaptureState = ulCaptureState;
                                        }
                                }
                                if (ulWriteError)
                                        netana_signal_drv_error( ptDevInst, ulWriteError);
                        }
                } else
                {
                        if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR) {
                                USER_Trace( ptDevInst, NETANA_TRACELEVEL_ERROR, "Stop cyclic thread! Failed to poll netanalyzer device file (error %d)!\n", iRet);
                        }
                        /* error */
                        ptDevInst->fRunEventThread = 0;
                        ptDevInst->iEventError     = iRet;
                }

        }

        if (pbReadBuf)
                free( pbReadBuf);

        return NULL;
}

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
        {NETANA_FILECAPTURE_NOT_ACTIVE,       "Capturing to file is not enabled"},
        {NETANA_CAPTURE_NOT_ACTIVE,           "Capturing is currently stopped"},
/* cifX like Errors that may happen during firmware load/start */
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

/*****************************************************************************/
/*! Retrieves the latest error value. The errors are stored and returned in
*   a thread-safe context.
*   \return NULL                                                             */
/*****************************************************************************/
int32_t get_error( int devno)
{
        FILE*    ptFile;
        int32_t  lRet;
        char     errorpath[256];

        sprintf( errorpath, "/sys/class/netanalyzer/netanalyzer_%d/driver_information/driver_information/error", devno);

        ptFile = fopen(errorpath, "r");
        if (0 == fscanf(ptFile, "%X", &lRet))
                lRet = NETANA_FUNCTION_FAILED;
        fclose(ptFile);

        return lRet;
}

/*****************************************************************************/
/*! Returns a handle to file of the sysfs tree of the netAnalyzer
*   \param iDeviceNum  number of the netAnalyzer device
*   \param szAttr      pointer to a string of the attribute file
*   \param iflag       specifies read write mode
*   \return handle to the file                                               */
/*****************************************************************************/
FILE* get_attribute_handle(int iDeviceNum, char* szAttr, int iflag)
{
        char szAttPath[NETANA_MAX_FILEPATH];
        FILE *fd;

        sprintf( szAttPath, "%s%d/%s", SYSFS_DEVICE_PATH, iDeviceNum, szAttr);
        if (iflag & OPEN_WRITE) {
                fd = fopen(szAttPath,"w");
        } else if (iflag & OPEN_READ) {
                fd = fopen(szAttPath,"r");
        } else {
                return NULL;
        }
        /* set to unbuffered access */
        if (fd)
                setbuf(fd,NULL);

        return fd;
}

/*****************************************************************************/
/*! Returns the number of netAnalyzer devices
*   \return the number of netAnalyzer device                                 */
/*****************************************************************************/
uint32_t netana_get_devicecount(void)
{
        struct dirent** namelist;
        int             num_netana;
        uint32_t        ulCardCnt = 0;

        num_netana = scandir("/sys/class/netanalyzer", &namelist, 0, alphasort);
        if(num_netana > 0) {
                int current;
                for(current = 0; current < num_netana; ++current) {
                        int num;
                        if(0 == sscanf(namelist[current]->d_name, "netanalyzer_%u", &num)) {
                                /* Error extracting number */
                        } else {
                                ++ulCardCnt;
                        }
                        free(namelist[current]);
                }
                free(namelist);
        }
        return ulCardCnt;
}

static int32_t OpenCaptureFiles(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
        int32_t  lRet = NETANA_NO_ERROR;
        uint32_t ulFile;
        char     szFileName[NETANA_MAX_FILEPATH * 2];

        CloseCaptureFiles(ptDevInst);
        ptDevInst->ulCurrentFile = 0;

        for(ulFile = 0; ulFile < ptDevInst->ulFileCount; ++ulFile) {
                NETANA_FILE_DATA_T* ptFile = &ptDevInst->atFiles[ulFile];
                /* Open File */
                if(sizeof(szFileName) < snprintf(szFileName, sizeof(szFileName),
                                "%s%s%s_%u.hea",
                                ptDevInst->szPath,
                                "/",
                                ptDevInst->szBaseFilename,
                                ulFile)) {
                        /* Unable to format string correctly */
                        if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR) {
                                        USER_Trace(ptDevInst,
                                        NETANA_TRACELEVEL_ERROR,
                                        "Error formatting Filename. Internal buffer is too short!");
                        }
                        lRet = NETANA_FILE_CREATION_FAILED;
                        break;

                } else if(NULL == (ptFile->hFile = FileOpen(szFileName,
                                &ptDevInst->ullFileSize,
                                O_CREAT | O_RDWR ))) {
                        /* Error opening file */
                        lRet = NETANA_FILE_CREATION_FAILED;
                        break;
                } else {
                        ptFile->ullOffset      = 0;
                        ptFile->ullDataWritten = 0;
                }
        }
        return lRet;
}

static void CloseCaptureFiles(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
        uint32_t ulFile;

        for(ulFile = 0; ulFile < ptDevInst->ulFileCount; ++ulFile) {
                NETANA_FILE_DATA_T* ptFile = &ptDevInst->atFiles[ulFile];

                if(NULL != ptFile->hFile) {
                        fsync(fileno(ptFile->hFile));
                        fclose(ptFile->hFile);
                        close(fileno(ptFile->hFile));
                        ptFile->hFile = NULL;
                }
        }
}

static int IsCaptureRunning(struct NETANA_DEVINSTANCE_T* ptDevInst)
{
        int fRet = 0;

        if (NETANA_CAPTURE_STATE_OFF != ptDevInst->ulCaptureState) {
                fRet = 1;
        }
        return fRet;
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
        FILE*                       fd;
        int32_t                     lRet;

        if(NULL == ptDrvInfo)
                return NETANA_INVALID_PARAMETER;

        tDrvInfo.ulCardCnt      = netana_get_devicecount();
        tDrvInfo.ulMaxFileCount = NETANA_MAX_FILES;

        if (0>=(fd = fopen("/sys/class/netanalyzer/netanalyzer_0/driver_information/driver_information/dma_buffersize", "r"))) {
                lRet = NETANA_DRIVER_NOT_RUNNING;
                goto drv_error;
        } else if (1!=fscanf( fd, "0x%X", &tDrvInfo.ulDMABufferSize)) {
                goto get_err;
        } else if (fclose(fd) || (0>=(fd = fopen("/sys/class/netanalyzer/netanalyzer_0/driver_information/driver_information/dma_buffercnt", "r")))) {
                lRet = NETANA_INVALID_PARAMETER;
                goto drv_error;
        } else if (1!=fscanf(fd, "%d", &tDrvInfo.ulDMABufferCount)) {
                goto get_err;
        }
        fclose(fd);

        if (ulDrvInfoSize == sizeof(NETANA_DRIVER_INFORMATION_T) ) {
                if (0>=(fd = fopen("/sys/class/netanalyzer/netanalyzer_0/driver_information/driver_information/driver_version", "r"))){
                        lRet = NETANA_INVALID_PARAMETER;
                        goto drv_error;
                } else if (8!=fscanf( fd, "%d %d %d %d %d %d %d %d",
                                        &tDrvInfo.ulVersionMajor,
                                        &tDrvInfo.ulVersionMinor,
                                        &tDrvInfo.ulVersionBuild,
                                        &tDrvInfo.ulVersionRevision,
                                        &tDrvInfo.ulToolkitVersionMajor,
                                        &tDrvInfo.ulToolkitVersionMinor,
                                        &tDrvInfo.ulToolkitVersionBuild,
                                        &tDrvInfo.ulToolkitVersionRevision)) {
                        goto get_err;
                }
                fclose(fd);
        }
        memcpy(ptDrvInfo, &tDrvInfo, ulDrvInfoSize);
        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(0);
drv_error:
        return lRet;
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

        for(iIdx = 0; iIdx < (unsigned long)(sizeof(s_atErrorToDescrTable) / sizeof(s_atErrorToDescrTable[0])); ++iIdx) {
                if(s_atErrorToDescrTable[iIdx].lError == lError) {
                        strncpy( szBuffer, s_atErrorToDescrTable[iIdx].szErrorDescr, ulBufferSize);
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
        int32_t lRet = NETANA_DEVICE_NOT_FOUND;  /* Default to device not found */
        char    szDevice[NETANA_MAX_FILEPATH];
        FILE    *fd;

        sprintf( szDevice, "%s%d%s", SYSFS_DEVICE_PATH, ulCardNr, "/device_information/device_info/device_info");
        if (NULL == (fd = fopen( szDevice, "r"))) {
                goto err_drv;
        } else if (sizeof(NETANA_DEVICE_INFORMATION_T) != fread( (void*)ptDevInfo, 1, ulDevInfoSize, fd)) {
                goto get_err;
        }
        fclose(fd);
        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ulCardNr);
err_drv:
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
        struct NETANA_DEVINSTANCE_T *ptDevice;
        int32_t                     lRet = NETANA_DEVICE_NOT_FOUND;  /* Default to device not found */
        FILE                        *fd;

        CHECK_HANDLE( hDev);

        ptDevice = (struct NETANA_DEVINSTANCE_T*)hDev;

        if (!(fd = get_attribute_handle( ptDevice->iDeviceNum, "device_information/device_info/device_info", OPEN_READ) )) {
                goto err_drv;
        } else if (sizeof(NETANA_DEVICE_INFORMATION_T) != fread((void*)ptDevInfo, 1, ulDevInfoSize, fd)) {
                goto get_err;
        }
        fclose(fd);
        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevice->iDeviceNum);
err_drv:
        return lRet;
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
        int32_t lRet = NETANA_DEVICE_NOT_FOUND;
        char    dev_name[NETANA_MAX_FILEPATH];
        int     fd;
        int     iDeviceNum = 0;
        char*   szTmpName  = NULL;
        int     fNameValid = 1;

        if( (NULL == szDevice) || (NULL == phDev) )
                return NETANA_INVALID_PARAMETER;

        check_driver();

        if (!g_fDriverRunning) {
                fprintf(stderr, "Driver not running!\n");
                return NETANA_DRIVER_NOT_RUNNING;
        } else if (g_fDriverVersionMismatch) {
                fprintf(stderr, "Driver Version mismatch!\n");
                return NETANA_FUNCTION_FAILED;
        } else if (g_libInitError) {
                fprintf(stderr, "Library initialization error!\n");
                return NETANA_FUNCTION_FAILED;
        }

        /* validate given name and convert for internal usage */
        szTmpName = malloc(strlen(szDevice));
        sscanf( szDevice, "%[^0-9]", szTmpName);
        if ( (0!=strncmp( szTmpName,"netANALYZER_", strlen(szTmpName))) &&
             (0!=strncmp( szTmpName,"netSCOPE_", strlen(szTmpName))) &&
             (0!=strncmp( szTmpName,"cifXANALYZER_", strlen(szTmpName))) ) {
                fNameValid = 0;
        }
        free(szTmpName);
        if (fNameValid == 0)
                return lRet;

        /* filter device number */
        sscanf( szDevice, "%*[^0-9]%d", &iDeviceNum);
        sprintf(dev_name, "/dev/%s%d", "netanalyzer_", iDeviceNum);

        if (0<=(fd = open( dev_name, O_RDONLY, O_NONBLOCK))) {
                /* lock file access (non blocking) */
                if (0 != (lRet = flock( fd, LOCK_EX | LOCK_NB))) {
                        if (errno == EWOULDBLOCK) {
                                fprintf( stderr, "%s may be opened by another process!\n", szDevice);
                        }
                        close(fd);
                        fd = -1;
                        return NETANA_INVALID_PARAMETER;
                } else {
                        struct NETANA_DEVINSTANCE_T *ptDevInstance = (struct NETANA_DEVINSTANCE_T*)malloc(sizeof(struct NETANA_DEVINSTANCE_T));

                        memset(ptDevInstance, 0, sizeof(*ptDevInstance));
                        ptDevInstance->ulCaptureState = NETANA_CAPTURE_STATE_OFF;
                        ptDevInstance->ulFileCount    = 0;
                        ptDevInstance->fd_Device      = fd;
                        ptDevInstance->iDeviceNum     = iDeviceNum;
                        memcpy( ptDevInstance->szDeviceName, szDevice, strlen(szDevice));

                        if (0 == pthread_mutex_lock(&g_tCardListLock)) {
                                STAILQ_INSERT_TAIL(&g_tCardList, ptDevInstance, tList);
                                pthread_mutex_unlock(&g_tCardListLock);

                                *phDev = ptDevInstance;

                                ptDevInstance->tEventThreadPrio.fThreadPrioSettings = g_tThreadSetting.fThreadPrioSettings;
                                ptDevInstance->tEventThreadPrio.iPolicy             = g_tThreadSetting.iPolicy;
                                ptDevInstance->tEventThreadPrio.iPriority           = g_tThreadSetting.iPriority;

                                create_log_file( ptDevInstance);

                                if (0 != g_tKThreadSetting.fThreadPrioSettings) {
                                        FILE *TmpFile;

                                         /* update kernel thread priority */
                                        if ((TmpFile = get_attribute_handle( ptDevInstance->iDeviceNum, "driver_information/driver_information/dsr", OPEN_WRITE) )) {
                                                if (0>fprintf(TmpFile, "%d %d", g_tKThreadSetting.iPolicy,
                                                                                g_tKThreadSetting.iPriority)) {
                                                        if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR) {
                                                                USER_Trace(ptDevInstance,
                                                                NETANA_TRACELEVEL_ERROR,
                                                                "Error setting priority of DSR kernel thread! (Error: %d)", errno);
                                                        }
                                                }
                                                fclose(TmpFile);
                                        } else {
                                                if(g_ulTraceLevel & NETANA_TRACELEVEL_ERROR) {
                                                        USER_Trace(ptDevInstance,
                                                        NETANA_TRACELEVEL_ERROR,
                                                        "Error setting priority of DSR kernel thread!");
                                                }
                                        }
                                }
                                lRet = NETANA_NO_ERROR;
                        } else {
                                close(fd);
                                free( ptDevInstance);
                                return NETANA_FUNCTION_FAILED;
                        }
                }
        } else {
                switch(errno)
                {
                        case EBUSY:
                                fprintf( stderr, "Error opening device %s! (open() ret=%d)\n", dev_name, errno);
                                lRet = NETANA_DEVICE_STILL_OPEN;
                                break;
                        case EACCES:
                                fprintf( stderr, "Access to %s not allowed! (open() ret=%d)\n", dev_name, errno);
                                lRet = NETANA_ACCESS_DENIED;
                                break;
                        default:
                                fprintf( stderr, "Error opening device %s! (open() ret=%d)\n", dev_name, errno);
                                lRet = NETANA_FUNCTION_FAILED;
                                break;
                }
        }
        return lRet;
}

/*****************************************************************************/
/*! Close an open device
*   \param hDev      Returned device handle
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_close_device(NETANA_HANDLE hDev)
{
        int32_t                      lRet           = NETANA_DEVICE_NOT_OPEN;
        struct NETANA_DEVINSTANCE_T *ptDevInstance  = hDev;
        uint32_t                     ulCaptureState = 0;
        uint32_t                     ulCaptureError = 0;

        CHECK_HANDLE( hDev);

        /* first check if capturing is deactivated */
        lRet = netana_get_state( hDev, &ulCaptureState, &ulCaptureError);

        /* close device if it is not responsive or not in a capturing state */
        if ((lRet != NETANA_NO_ERROR) || (NETANA_CAPTURE_STATE_OFF == ulCaptureState))
        {
                close_log_file( ptDevInstance);

                if (-EBADF == close( ptDevInstance->fd_Device))
                        lRet = NETANA_DEVICE_NOT_OPEN;
                else
                        lRet = NETANA_NO_ERROR;

        } else {
                /* if capture is active return error */
                if (lRet == 0)
                        lRet = NETANA_CAPTURE_ACTIVE;
        }
        if (lRet == NETANA_NO_ERROR) {
                if (0 == pthread_mutex_lock(&g_tCardListLock)) {
                        STAILQ_REMOVE( &g_tCardList, ptDevInstance, NETANA_DEVINSTANCE_T,tList);
                        free( ptDevInstance);
                        pthread_mutex_unlock(&g_tCardListLock);
                }
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
        char                         szattname[NETANA_MAX_FILEPATH];
        FILE                         *fd;
        uint32_t                     ulFilterSizeA;
        uint32_t                     ulFilterSizeB;
        uint32_t                     ulRelationShip;

        CHECK_HANDLE( hDev);

        /* validate given parameter */
        if ((NULL != ptFilterA) && (ptFilterA->ulFilterSize>0))
        {
                if ((NULL == ptFilterA->pbMask) || (NULL == ptFilterA->pbValue))
                        return NETANA_INVALID_PARAMETER;
        }
        if ((NULL != ptFilterB) && (ptFilterB->ulFilterSize>0))
        {
                if ((NULL == ptFilterB->pbMask) || (NULL == ptFilterB->pbValue))
                        return NETANA_INVALID_PARAMETER;
        }

        sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/relation");
        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                lRet = NETANA_INVALID_PARAMETER;
                goto err_drv;
        } else if (3 != fscanf( fd, "%d %d %d", &ulRelationShip, &ulFilterSizeA, &ulFilterSizeB)) {
                lRet = NETANA_IOCTL_FAILED;
                fclose(fd);
        } else {
                fclose(fd);
                lRet = NETANA_NO_ERROR;

                if (ptFilterA) {
                        if (ptFilterA->ulFilterSize>ulFilterSizeA)
                                ptFilterA->ulFilterSize = ulFilterSizeA;

                        sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/maska");

                        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                                lRet = NETANA_INVALID_PARAMETER;
                                goto err_drv;
                        } else if (ptFilterA->ulFilterSize != fread( ptFilterA->pbMask, 1, ptFilterA->ulFilterSize, fd)) {
                                goto get_err;
                        } else {
                                fclose(fd);

                                sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/vala");
                                if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                                        lRet = NETANA_INVALID_PARAMETER;
                                        goto err_drv;
                                } else if (ptFilterA->ulFilterSize != fread( ptFilterA->pbValue, 1, ptFilterA->ulFilterSize, fd)) {
                                        goto get_err;
                                } else
                                        lRet = NETANA_NO_ERROR;

                                fclose(fd);
                        }
                }

                if (ptFilterB) {
                        if (ptFilterB->ulFilterSize>ulFilterSizeB)
                                ptFilterB->ulFilterSize = ulFilterSizeB;

                        sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/maskb");

                        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                                lRet = NETANA_INVALID_PARAMETER;
                                goto err_drv;
                        } else if (ptFilterB->ulFilterSize != fread( ptFilterB->pbMask, 1, ptFilterB->ulFilterSize, fd)) {
                                goto get_err;
                        } else {
                                fclose(fd);

                                sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/valb");
                                if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                                        lRet = NETANA_INVALID_PARAMETER;
                                        goto err_drv;
                                } else if (ptFilterB->ulFilterSize != fread( ptFilterB->pbValue, 1, ptFilterB->ulFilterSize, fd)) {
                                        goto get_err;
                                } else
                                        lRet = NETANA_NO_ERROR;

                                fclose(fd);
                        }
                }
        }
        if (NULL != pulRelationShip)
                *pulRelationShip = ulRelationShip;

        return lRet;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);

err_drv:
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
        int32_t                      lRet          = NETANA_INVALID_PORT;
        struct NETANA_DEVINSTANCE_T  *ptDevInst    = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE                         *fd           = NULL;
        uint32_t                     ulFilterSizeA = 0;
        uint32_t                     ulFilterSizeB = 0;
        uint32_t                     ulSize        = 0;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if(IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_ACTIVE;

        if ((NULL != ptFilterA) && (ptFilterA->ulFilterSize>0))
        {
                if ((NULL == ptFilterA->pbMask) || (NULL == ptFilterA->pbValue))
                        return NETANA_INVALID_PARAMETER;
        }
        if ((NULL != ptFilterB) && (ptFilterB->ulFilterSize>0))
        {
                if ((NULL == ptFilterB->pbMask) || (NULL == ptFilterB->pbValue))
                        return NETANA_INVALID_PARAMETER;
        }

        //TODO: check port
        lRet = NETANA_NO_ERROR;

        if ((NULL != ptFilterA) && (ptFilterA->ulFilterSize>0)) {
                ulFilterSizeA = ptFilterA->ulFilterSize;
                sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/maska");

                if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                        goto err_drv;
                } else if ((ptFilterA->ulFilterSize != fwrite( ptFilterA->pbMask, 1, ptFilterA->ulFilterSize, fd))) {
                        ulSize = ptFilterA->ulFilterSize;
                        goto check_size;
                } else {
                        fclose(fd);
                        sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/vala");

                        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                                goto err_drv;
                        } else if ((ptFilterA->ulFilterSize != fwrite( ptFilterA->pbValue, 1, ptFilterA->ulFilterSize, fd))) {
                                ulSize = ptFilterA->ulFilterSize;
                                goto check_size;
                        } else {
                                lRet = NETANA_NO_ERROR;
                        }
                        fclose(fd);
                }
        }
        if ((NULL != ptFilterB) && (ptFilterB->ulFilterSize>0)) {
                ulFilterSizeB = ptFilterB->ulFilterSize;
                sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/maskb");

                if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                        goto err_drv;
                } else if ((ptFilterB->ulFilterSize != fwrite( ptFilterB->pbMask, 1, ptFilterB->ulFilterSize, fd))) {
                        ulSize = ptFilterB->ulFilterSize;
                        goto check_size;
                } else {
                        fclose(fd);
                        sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/valb");

                        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                                goto err_drv;
                        } else if ((ptFilterB->ulFilterSize != fwrite(ptFilterB->pbValue, 1, ptFilterB->ulFilterSize, fd))) {
                                ulSize = ptFilterB->ulFilterSize;
                                goto check_size;
                        } else {
                                lRet = NETANA_NO_ERROR;
                        }
                        fclose(fd);
                }
        }
        sprintf( szattname, "%s%d%s", "hw_resources/filter/filter", ulPort, "/relation");
        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                lRet = NETANA_INVALID_PARAMETER;
                goto err_drv;
        } else if ((0>fprintf( fd, "%d %d %d", ulRelationShip, ulFilterSizeA, ulFilterSizeB))) {
                goto get_err;
        } else {
                lRet = NETANA_NO_ERROR;
        }

        fclose(fd);
        return lRet;

check_size:
{
        /* NOTE: Special case [ulFilterSizeA/B]>[size of device filter]            */
        /* The kernel driver will not recognize this error since reading/writing   */
        /* to the binary sysfs file will never be done with more bytes than the    */
        /* defined size of the file (size of file=device filter size).             */
        /* In case of a size mismatch, first we have to validate the size of file, */
        /* if everthing is ok retrieve kernel driver error                         */
        struct stat buf = {0};
        fstat(fileno(fd), &buf);
        if (ulSize > buf.st_size)
                lRet = NETANA_INVALID_BUFFERSIZE;
}
get_err:
        fclose(fd);
        if (NETANA_NO_ERROR == lRet)
                lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        FILE                         *fd       = NULL;
        char                         szattname[NETANA_MAX_FILEPATH];
        NETANA_PORT_STATE_T          tStatus;

        CHECK_HANDLE( hDev);

        sprintf( szattname, "%s%d%s", "hw_resources/port/port", ulPort, "/status");

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                goto err_drv;
        } else if (14 != fscanf(fd, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %d %llu",
                                &tStatus.ulLinkState,
                                (long long unsigned int*)&tStatus.ullFramesReceivedOk,
                                (long long unsigned int*)&tStatus.ullRXErrors,
                                (long long unsigned int*)&tStatus.ullAlignmentErrors,
                                (long long unsigned int*)&tStatus.ullFrameCheckSequenceErrors,
                                (long long unsigned int*)&tStatus.ullFrameTooLongErrors,
                                (long long unsigned int*)&tStatus.ullSFDErrors,
                                (long long unsigned int*)&tStatus.ullShortFrames,
                                (long long unsigned int*)&tStatus.ullFramesRejected,
                                (long long unsigned int*)&tStatus.ullLongPreambleCnt,
                                (long long unsigned int*)&tStatus.ullShortPreambleCnt,
                                (long long unsigned int*)&tStatus.ullBytesLineBusy,
                                &tStatus.ulMinIFG,
                                (long long unsigned int*)&tStatus.ullTime)) {
                goto get_err;
        } else {
                memcpy( ptStatus, &tStatus, ulSize);
        }
        fclose(fd);

        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);

err_drv:
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

        CHECK_HANDLE( hDev);

        if(IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_ACTIVE;

        if(ulFileCount > NETANA_MAX_FILES)
                return NETANA_INVALID_PARAMETER;

        /* A file must be at least as large as a single DMA buffer + file header,
        as we can only write complete DMA buffers to disk */
        if(ullFileSize < (g_ulDMABufferSize + sizeof(NETANA_FILE_HEADER_T)))
                return NETANA_INVALID_PARAMETER;

        strncpy( ptDevInst->szPath, szPath, sizeof(ptDevInst->szPath));
        strncpy( ptDevInst->szBaseFilename, szBaseFilename, sizeof(ptDevInst->szBaseFilename));
        ptDevInst->ulFileCount = ulFileCount;
        ptDevInst->ullFileSize = ullFileSize;

        if(NETANA_NO_ERROR != (lRet = OpenCaptureFiles(ptDevInst))){
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
        int32_t                      lRet          = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst     = (struct NETANA_DEVINSTANCE_T*)hDev;
        NETANA_DRIVER_INFORMATION_T  tDrvInfo      = {0};
        FILE                         *fd           = NULL;
        struct timespec              tSleepTime    = {0, (1 * 1000 * 1000)}; //1ms
        int                          fStartDataCB  = 0;
        int                          fStartStateCB = 0;

        CHECK_HANDLE( hDev);

        if(IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_ACTIVE;

        if( (ptDevInst->ulFileCount == 0) && (pfnData == NULL) )
                return NETANA_CONFIGURATION_ERROR;

        ptDevInst->pfnStatus = pfnStatus;
        ptDevInst->pfnData   = pfnData;
        ptDevInst->pvUser    = pvUser;

        if (NETANA_NO_ERROR != (lRet = netana_driver_information(sizeof(NETANA_DRIVER_INFORMATION_T), &tDrvInfo)) )
                return lRet;

        /* Fill Up DMA Buffers */
        ptDevInst->ulMaxFileCount   = 1;
        ptDevInst->ulDMABufferSize  = tDrvInfo.ulDMABufferSize;
        ptDevInst->ulDMABufferCount = tDrvInfo.ulDMABufferCount;

        if(ulCaptureMode & NETANA_CAPTUREMODE_RINGBUFFER) {
                ptDevInst->fRingBuffer = 1;
        } else{
                ptDevInst->fRingBuffer = 0;
        }
        if(ptDevInst->ulFileCount > 0){
                ptDevInst->ulCurrentFile  = 0;
                ptDevInst->fWriteToFile   = 1;

                ptDevInst->tFileHeader.ulCookie            = NETANA_FILE_HEADER_COOKIE;
                ptDevInst->tFileHeader.ulIdx               = 0;
                ptDevInst->tFileHeader.ullCaptureStart     = ullReferenceTime;
                ptDevInst->tFileHeader.lTimezoneCorrection = g_lTimezoneCorrection;
                ptDevInst->tFileHeader.ulReserved          = 0;

                /* Write file header */
                if(NETANA_NO_ERROR != netana_writefileheader(ptDevInst, NETANA_FILE_HEADER_SIZE_INVALID)) {
                        printf("errror writing file header\n");
                        lRet = NETANA_FILE_WRITE_FAILED;
                }

        } else {
                ptDevInst->fWriteToFile = 0;
        }
        /* we always need to start state callback for internal capture handling */
        fStartStateCB = 1;
        if (pfnData)
                fStartDataCB  = 1;

        ptDevInst->iEventError     = 0;
        ptDevInst->fRunEventThread = 1;
        if(0 != pthread_create( &ptDevInst->hPollingThread, NULL, netana_event_thread, (void*)ptDevInst))//TODO setup error
                goto err_drv;

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_control/capture_control/control", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%d %d %d %llu %i %i",
                ulCaptureMode,
                ulPortEnableMask,
                ulMacMode,
                (long long unsigned int)ullReferenceTime,
                fStartDataCB,
                fStartStateCB)) {
                goto get_err;
        }
        fclose(fd);

        lRet = NETANA_NO_ERROR;
        /* check if thread creation succeeds */
        while(0 == IsCaptureRunning(ptDevInst)){
                nanosleep(&tSleepTime, NULL);
                if (ptDevInst->iEventError != 0) {
                        USER_Trace( ptDevInst, NETANA_TRACELEVEL_INFO, "Failed to create cyclic thread! (-> stopping capture)\n");
                        netana_stop_capture( ptDevInst);
                        lRet = NETANA_THREAD_CREATION_FAILED;
                        break;
                }
        };

        return lRet;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);

err_drv:
        return lRet;
}

/*****************************************************************************/
/*! Stop current capture
*   \param hDev               Device handle
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_stop_capture(NETANA_HANDLE hDev)
{
        int32_t                      lRet       = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst  = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE                         *fd        = NULL;
        struct timespec              tSleepTime = {0, (1 * 1000 * 1000)}; //1ms

        CHECK_HANDLE( hDev);

        if(!IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_NOT_ACTIVE;

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_control/capture_control/control", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%d", 1)) {
                goto get_err;
        }
        fclose(fd);

        if(ptDevInst->ulFileCount > 0) {
                /* Only update header if it was not already stopped by DSR */
                if(ptDevInst->fWriteToFile) {
                        /* Write File header into last written file */
                        uint32_t            ulFile       = ptDevInst->ulCurrentFile;
                        NETANA_FILE_DATA_T* ptFileData   = &ptDevInst->atFiles[ulFile];

                        netana_writefileheader( ptDevInst, ptFileData->ullOffset - sizeof(NETANA_FILE_HEADER_T));
                }
                /* Release all open capture files */
                CloseCaptureFiles(ptDevInst);

                /* It's defined that all files are closed and user is expected to call
                netana_setfilelist before next capture */
                ptDevInst->ulFileCount = 0;
        }
        while(IsCaptureRunning(ptDevInst)){nanosleep(&tSleepTime, NULL);};
        ptDevInst->fRunEventThread = 0;
        pthread_join(ptDevInst->hPollingThread, NULL);

        ptDevInst->pfnStatus = NULL;
        ptDevInst->pfnData   = NULL;

        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        int32_t                      lRet      = NETANA_INVALID_HANDLE;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE                         *fd       = NULL;

        CHECK_HANDLE( hDev);

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/device_state/device_state", OPEN_READ))) {
                goto err_drv;
        } else if (2 != fscanf(fd, "0x%X 0x%X", pulCaptureState, pulCaptureError)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);

err_drv:
        return lRet;
}

/*****************************************************************************/
/*! Get current capture state. Called from cyclic thread, acknowledges notified state changes.
*   \param hDev             Device handle
*   \param pulCaptureState  Returned capture state
*   \param pulCaptureError  Returned capture error
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t netana_get_state_poll(NETANA_HANDLE hDev, uint32_t* pulCaptureState, uint32_t* pulCaptureError)
{
        int32_t                      lRet      = NETANA_INVALID_HANDLE;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE                         *fd       = NULL;

        CHECK_HANDLE( hDev);

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/device_state/device_state_poll", OPEN_READ))) {
                goto err_drv;
        } else if (2 != fscanf(fd, "0x%X 0x%X", pulCaptureState, pulCaptureError)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);

err_drv:
        return lRet;
}

static int32_t update_timeout( struct NETANA_DEVINSTANCE_T* ptDevInst, uint32_t ulTimeout)
{
        int32_t lRet = NETANA_INVALID_PARAMETER;
        FILE    *fd  = NULL;
        char    szattname[NETANA_MAX_FILEPATH];

        /* set timeout */
        sprintf( szattname, "%s", "device_config/timeout/config");
        fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE);
        if (!fd){
                lRet = NETANA_INVALID_HANDLE;
                goto err_drv;
        } else if (0>fprintf(fd, "%u", ulTimeout)) {
                goto get_err;
        }
        fclose(fd);
        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        FILE                         *fd       = NULL;
        char                         szattname[NETANA_MAX_FILEPATH];
        unsigned int                 tmp;

        CHECK_HANDLE( hDev);
        if(pusValue == NULL)
                return NETANA_INVALID_PARAMETER;

        if (NETANA_NO_ERROR != (lRet = update_timeout( ptDevInst, ulTimeout)))
                return lRet;

        /* read/write register */
        sprintf( szattname, "%s%d%s%d", "hw_resources/phy/phy", ulPhyNum, "/regval", ulPhyReg);

        if (ulDirection == NETANA_PHY_DIRECTION_READ)
                fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ);
        if (ulDirection == NETANA_PHY_DIRECTION_WRITE)
                fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE);

        if (!fd){
                goto err_drv;
        } else {
                if  (ulDirection == NETANA_PHY_DIRECTION_READ) {
                        if (1 != fscanf(fd, "%u", &tmp))
                                goto get_err;
                        else
                                *pusValue = tmp & 0xFFFF;

                } else if (ulDirection == NETANA_PHY_DIRECTION_WRITE) {
                                if (0>fprintf(fd, "%u", (unsigned int)*pusValue))
                                        goto get_err;
                } else {
                        fclose(fd);
                        lRet = NETANA_INVALID_PARAMETER;
                        goto err_drv;
                }
        }
        fclose(fd);

        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        FILE                         *fd       = NULL;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if(IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_ACTIVE;

        sprintf( szattname, "%s%d%s", "hw_resources/gpio/gpio", ulGpio, "/status");

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                goto err_drv;
        } else if (3 != fscanf(fd, "%d %d %d",
                &ptMode->ulMode,
                &ptMode->uData.tTrigger.ulCaptureTriggers,
                &ptMode->uData.tTrigger.ulEndDelay)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;

get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);

err_drv:
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
        FILE                         *fd       = NULL;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if(IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_ACTIVE;

        sprintf( szattname, "%s%d%s", "hw_resources/gpio/gpio", ulGpio, "/status");

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%d %d %d",
                ptMode->ulMode,
                ptMode->uData.tTrigger.ulCaptureTriggers,
                ptMode->uData.tTrigger.ulEndDelay)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        FILE                         *fd       = NULL;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        sprintf( szattname, "%s%d%s", "hw_resources/gpio/gpio", ulGpio, "/level");

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                goto err_drv;
        } else if (1 != fprintf(fd, "%d", ulLevel)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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

        CHECK_HANDLE( hDev);

        if(!IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_NOT_ACTIVE;

        if(ptDevInst->ulFileCount == 0)
                return NETANA_FILECAPTURE_NOT_ACTIVE;

        tFileInfo.ulActualFileNr        = ptDevInst->ulCurrentFile + 1;
        tFileInfo.ulMaxFileCnt          = ptDevInst->ulFileCount;

        for(ulFile = 0; ulFile < ptDevInst->ulFileCount; ++ulFile) {
                tFileInfo.ullTotalBytesWritten  += ptDevInst->atFiles[ulFile].ullDataWritten;
        }

        tFileInfo.ullMaxBytes      = ptDevInst->ullFileSize * ptDevInst->ulFileCount;
        tFileInfo.ulRingBufferMode = ptDevInst->fRingBuffer ? 1 : 0;

        memcpy(ptFileInfo, &tFileInfo, min(ulFileInfoSize, sizeof(tFileInfo)));

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
        FILE *fd;

        switch (ulCommandId)
        {
                case NETANA_MNGMT_CMD_BLINK:
                {
                        char*                       szDevice = (char*)ptInputParameter;
                        NETANA_DEVICE_INFORMATION_T tDevInfo;
                        uint32_t                    ulCardNumber = 0;

                        if (!szDevice)
                                goto err_drv;

                        lRet = NETANA_NO_ERROR;
                        while (lRet != NETANA_DEVICE_NOT_FOUND) {
                                lRet = netana_enum_device( ulCardNumber, sizeof(NETANA_DEVICE_INFORMATION_T), &tDevInfo);

                                if ((lRet == NETANA_NO_ERROR)) {
                                        if(0 == strncmp( szDevice, tDevInfo.szDeviceName, strlen(tDevInfo.szDeviceName))) {
                                                sscanf( szDevice, "%*[^0-9]%d", &ulCardNumber);
                                                if (!(fd = get_attribute_handle( ulCardNumber, "device_control/exec_cmd/identify", OPEN_WRITE))) {
                                                        lRet = NETANA_INVALID_PARAMETER;
                                                        goto err_drv;
                                                } else if (0>fprintf(fd, "%i", 1)) {
                                                        fclose(fd);
                                                        lRet = get_error(ulCardNumber);
                                                        goto err_drv;
                                                }
                                                fclose(fd);
                                                return NETANA_NO_ERROR;
                                        }
                                }
                                ulCardNumber++;
                        }
                        break;
                }

                case NETANA_MNGMT_CMD_DEV_SCAN:
                {
                        NETANA_MNGMT_DEV_SCAN_IN_T*  ptInputParam = (NETANA_MNGMT_DEV_SCAN_IN_T*)ptInputParameter;
                        NETANA_MNGMT_DEV_SCAN_OUT_T* ptReturnVal  = (NETANA_MNGMT_DEV_SCAN_OUT_T*)ptOutputParameter;
                        NETANA_DRIVER_INFORMATION_T  tDrvInfo     = {0};

                        if( (ptOutputParameter == NULL) || (sizeof(NETANA_MNGMT_DEV_SCAN_OUT_T) != ulOutputParameterSize) )
                                return lRet;

                        if ((lRet = netana_driver_information(sizeof(NETANA_DRIVER_INFORMATION_T), &tDrvInfo)))
                                return lRet;

                        if ( (ptInputParam) && (ptInputParam->pfnCallBack && tDrvInfo.ulCardCnt) ) {
                                uint32_t                    ulCardNumber = tDrvInfo.ulCardCnt;
                                while(ulCardNumber){
                                        NETANA_DEVICE_INFORMATION_T tDevInfo;
                                        /* retrieve the name of the last device and report it to the user */
                                        if ( NETANA_NO_ERROR == netana_enum_device( (tDrvInfo.ulCardCnt - ulCardNumber), sizeof(NETANA_DEVICE_INFORMATION_T), &tDevInfo) ) {
                                                ptInputParam->pfnCallBack( 0,
                                                        tDrvInfo.ulCardCnt - ulCardNumber+1,
                                                        tDevInfo.szDeviceName,
                                                        ptInputParam->pvUser);
                                        }
                                        ulCardNumber--;
                                }
                        }
                        ptReturnVal->ulNofDevices = tDrvInfo.ulCardCnt;
                        break;
                }

                case NETANA_MNGMT_CMD_GET_DEV_FEATURE:
                {
                        NETANA_MNGMT_GET_DEV_FEATURE_IN_T*  ptFeatureIn = (NETANA_MNGMT_GET_DEV_FEATURE_IN_T*)ptInputParameter;
                        struct NETANA_DEVINSTANCE_T*        ptDevInst   = NULL;
                        NETANA_MNGMT_GET_DEV_FEATURE_OUT_T* ptFeatures  = ptOutputParameter;
                        NETANA_MNGMT_GET_DEV_FEATURE_OUT_T  tFeatures   = {0};

                        if ((ptFeatureIn == NULL) || (ulInputParameterSize != sizeof(NETANA_MNGMT_GET_DEV_FEATURE_IN_T)) ||
                                (ptOutputParameter == NULL) || (ulOutputParameterSize == 0))
                                return NETANA_INVALID_PARAMETER;

                        if (NULL == (ptDevInst = (struct NETANA_DEVINSTANCE_T*)ptFeatureIn->hDev))
                                return NETANA_INVALID_PARAMETER;

                        CHECK_HANDLE( ptDevInst);

                        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/device_features/device_features", OPEN_READ))) {
                                goto err_drv;
                        } else if (0>fscanf(fd, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                                &tFeatures.ulStructVersion,
                                &tFeatures.ulPhysType,
                                &tFeatures.ulNumPhysPorts,
                                &tFeatures.ulPhysTapPresent,
                                &tFeatures.ulPhysForwardingSupport,
                                &tFeatures.ulPhysPortSpeedSupport,
                                &tFeatures.ulPhysTransparentModeSupport,
                                &tFeatures.ulNumGpios,
                                &tFeatures.ulGpioInputRisingSupport,
                                &tFeatures.ulGpioInputFallingSupport,
                                &tFeatures.ulGpioOutputModeSupport,
                                &tFeatures.ulGpioOutputPWMSupport,
                                &tFeatures.ulGpioSyncInSupport,
                                &tFeatures.ulGpioTriggerStartSupport,
                                &tFeatures.ulGpioTriggerStopSupport,
                                &tFeatures.ulGpioVoltage3VSupport,
                                &tFeatures.ulGpioVoltage24VSupport,
                                &tFeatures.ulSyncSupport)) {
                                fclose(fd);

                                lRet = get_error(ptDevInst->iDeviceNum);
                                goto err_drv;
                        } else
                        {
                                if (ulOutputParameterSize<sizeof(tFeatures))
                                        ulOutputParameterSize = sizeof(tFeatures);

                                memcpy( ptFeatures, &tFeatures, ulOutputParameterSize);
                                lRet = NETANA_NO_ERROR;
                        }
                }
                break;

                case NETANA_MNGMT_CMD_SET_DEV_CLASS_FILTER:
                {
                        FILE*                                   ptFile     = NULL;
                        NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T* ptFilterIn = (NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T*)ptInputParameter;

                        if ((ptFilterIn == NULL) || (ulInputParameterSize != sizeof(NETANA_MNGMT_SET_DEV_CLASS_FILTER_IN_T)))
                                return NETANA_INVALID_PARAMETER;

                        ptFile = fopen("/sys/class/netanalyzer/netanalyzer_0/driver_information/driver_information/device_class", "w");

                        if (ptFile == NULL)
                                goto err_drv;

                        setbuf( ptFile, NULL);
                        if (0>fprintf(ptFile, "%d", ptFilterIn->ulDeviceClass))
                                lRet = get_error(0);
                        else
                                lRet = NETANA_NO_ERROR;
                        fclose(ptFile);
                }
                break;

                default:
                        lRet = NETANA_INVALID_PARAMETER;
                        break;
        }
err_drv:
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
        FILE                         *fd       = NULL;

        CHECK_HANDLE( hDev);

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_control/time/config", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%llu", (long long unsigned int)ullReferenceTime)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE                         *fd       = NULL;

        CHECK_HANDLE( hDev);

        if(IsCaptureRunning(ptDevInst))
                return NETANA_CAPTURE_ACTIVE;

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_control/pi/config", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%d %d", ulP, ulI)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;
}

/*****************************************************************************/
/*! Set device ident info
* Note: szFi
*   \param hDev             Device handle
*   \param szFileName       Filename containing the ident information        */
/*****************************************************************************/
int32_t netana_set_device_ident(NETANA_HANDLE hDev, char* szFileName)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;
        struct stat                  buf;
        char                         filepath[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);
        if(NULL == szFileName)
                return NETANA_INVALID_PARAMETER;

        strcpy(filepath, "/lib/firmware/");
        strncat(filepath, szFileName, strlen(szFileName));

        if (0>(stat(filepath, &buf)))
                return NETANA_FILE_OPEN_ERROR;

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/ident_info/ident_info", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%s", szFileName)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;
}


/*****************************************************************************/
/*! Get device ident info
*   \param hDev             Device handle
*   \param ptIdentInfo      Ident Information                                */
/*****************************************************************************/
int32_t netana_get_device_ident(NETANA_HANDLE hDev, uint32_t ulDevIdentInfoSize, NETANA_DEVICE_IDENT_T *ptIdentInfo)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        NETANA_DEVICE_IDENT_T        tIdentInfo;
        FILE*                        fd        = 0;

        CHECK_HANDLE( hDev);
        if(NULL == ptIdentInfo)
                return NETANA_INVALID_PARAMETER;
        if (ulDevIdentInfoSize > sizeof(NETANA_DEVICE_IDENT_T))
                return NETANA_INVALID_BUFFERSIZE;

        memset( (void*)&tIdentInfo, 0, sizeof(NETANA_DEVICE_IDENT_T));

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/ident_info/ident_info", OPEN_READ))) {
                goto err_drv;
        } else if (8!=fscanf( fd, "%u %u %u %u 0x%X 0x%X %u %u",
                        &tIdentInfo.ulIdentVersion,
                        &tIdentInfo.ulDeviceNr,
                        &tIdentInfo.ulSerialNr,
                        (unsigned int*)&tIdentInfo.usManufacturer,
                        &tIdentInfo.ulFlags1,
                        &tIdentInfo.ulFlags2,
                        (unsigned int*)&tIdentInfo.usNetXid,
                        (unsigned int*)&tIdentInfo.usNetXflags)) {
                goto get_err;
        }
        memcpy( ptIdentInfo, &tIdentInfo, ulDevIdentInfoSize);
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;
}


/*****************************************************************************/
/*! Check device ident information
*   \param hDev               Device handle
*   \param ulDevIdentInfoSize size of pbDevIdentInfo
*   \param pbDevIdentInfo     crypted ident information                      */
/*****************************************************************************/
int32_t netana_check_device_ident(NETANA_HANDLE hDev, uint32_t ulDevIdentInfoSize, uint8_t* pbDevIdentInfo)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;

        CHECK_HANDLE( hDev);

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/ident_info/ident_check", OPEN_WRITE))) {
                goto err_drv;
        } else if (ulDevIdentInfoSize != fwrite( (void*)pbDevIdentInfo, 1, ulDevIdentInfoSize, fd)) {
                goto get_err;
        } else {
                fclose(fd);
                if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "device_information/ident_info/ident_check", OPEN_READ)))
                        goto err_drv;
                else if (ulDevIdentInfoSize != fread( (void*)pbDevIdentInfo, 1, ulDevIdentInfoSize, fd))
                        goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;
}

/****************************************************************************/
/*! Function selects the different voltage settings
*   \param hDev             Handle to device
*   \param ulGpioSelMask    Mask of GPIOs to be affected
*   \param ulGpioVoltageSel Voltage selection
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_set_gpio_voltage( NETANA_HANDLE hDev, uint32_t ulGpioSelMask, uint32_t ulGpioVoltageSel)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;

        CHECK_HANDLE( hDev);

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "hw_resources/gpio/voltage/set_voltage", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%d %d\n", ulGpioSelMask, ulGpioVoltageSel)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;

return lRet;
}

/****************************************************************************/
/*! Function returns the selected voltage settings
*   \param hDev              Handle to device
*   \param ulGpio            Number of the requested GPIO
*   \param pulGpioVoltageSel Pointer to returned voltage setting
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_get_gpio_voltage( NETANA_HANDLE hDev, uint32_t ulGpio, uint32_t* pulGpioVoltageSel)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if (pulGpioVoltageSel == NULL)
                return NETANA_INVALID_PARAMETER;

        sprintf( szattname, "%s%d%s", "hw_resources/gpio/gpio", ulGpio, "/voltage");
        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                goto err_drv;
        } else if (1!=fscanf(fd, "%d", pulGpioVoltageSel)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;

return lRet;
}

/****************************************************************************/
/*! Linux specific function: Required to for Marshaller to signal driver errors
*   \param hDev              Handle to device
*   \param ulError           Error to signal to firmware
*   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_signal_drv_error( void* hDev, uint32_t ulError)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;

        CHECK_HANDLE( hDev);

        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, "fw/fw_error", OPEN_WRITE))) {
                goto err_drv;
        } else if (0>fprintf(fd, "%d", ulError)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        int32_t                      lRet         = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst    = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd           = NULL;
        uint32_t                     ulRecvPktCnt = 0;
        uint32_t                     ulSendPktCnt = 0;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if ((NULL == pulRecvPktCnt) && (NULL == pulSendPktCnt))
                return NETANA_INVALID_PARAMETER;

        sprintf( szattname, "%s", "device_control/mailbox/status");
        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                goto err_drv;
        } else if (2!=fscanf(fd, "%d %d", &ulRecvPktCnt, &ulSendPktCnt)) {
                goto get_err;
        }
        if (NULL != pulSendPktCnt)
                *pulSendPktCnt = ulSendPktCnt;
        if (NULL != pulRecvPktCnt)
                *pulRecvPktCnt = ulRecvPktCnt;

        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
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
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if (ulSize<NETANA_PACKET_HEADER_SIZE) {
                return NETANA_BUFFER_TOO_SHORT;
        } else if (NETANA_NO_ERROR != (lRet = update_timeout( ptDevInst, ulTimeout))) {
                return lRet;
        }
        sprintf( szattname, "%s", "device_control/mailbox/mailbox");
        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_READ))) {
                goto err_drv;
        } else {
                size_t readlen = fread( ptRecvPkt, 1, ulSize, fd);

                if (readlen != ulSize) {
                        /* In case of success fread returns always the number of requested bytes. Anyway */
                        /* we can safely access the buffer since we checked the min packet size (header) */
                        /* and the kernel driver checks the buffer size and may return an error.         */
                        goto get_err;
                }
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;
}

/*****************************************************************************/
/*! Inserts a packet into the device mailbox
 *   \param hDev       Handle to device
 *   \param ptSendPkt  Packet to send to channel
 *   \param ulTimeout  Time in ms to wait for card to accept the packet
 *   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t APIENTRY netana_put_packet(NETANA_HANDLE hDev, NETANA_PACKET* ptSendPkt, uint32_t ulTimeout)
{
        int32_t                      lRet      = NETANA_INVALID_PARAMETER;
        struct NETANA_DEVINSTANCE_T* ptDevInst = (struct NETANA_DEVINSTANCE_T*)hDev;
        FILE*                        fd        = 0;
        uint32_t                     ulLen     = 0;
        char                         szattname[NETANA_MAX_FILEPATH];

        CHECK_HANDLE( hDev);

        if (NULL == ptSendPkt) {
                return NETANA_INVALID_PARAMETER;
        } else if (ptSendPkt->tHeader.ulLen>NETANA_MAX_DATA_SIZE) {
                return NETANA_INVALID_BUFFERSIZE;
        } else if (NETANA_NO_ERROR != (lRet = update_timeout( ptDevInst, ulTimeout))) {
                return lRet;
        }
        /* add size of header */
        ulLen = ptSendPkt->tHeader.ulLen + NETANA_PACKET_HEADER_SIZE;

        sprintf( szattname, "%s", "device_control/mailbox/mailbox");
        if (!(fd = get_attribute_handle( ptDevInst->iDeviceNum, szattname, OPEN_WRITE))) {
                goto err_drv;
        } else if (ulLen != fwrite( ptSendPkt, 1, ulLen, fd)) {
                goto get_err;
        }
        fclose(fd);

        return NETANA_NO_ERROR;
get_err:
        fclose(fd);
        lRet = get_error(ptDevInst->iDeviceNum);
err_drv:
        return lRet;
}

/*****************************************************************************/
/*! Exchanges a packet with the device
 *   ATTENTION: This function will poll for receive packet, and will discard
 *              any packets that do not match the send packet. So don't use
 *              it during active data transfers
 *   \param hDev             Handle to device
 *   \param ptSendPkt        Send packet pointer
 *   \param ptRecvPkt        Pointer to place received Packet in
 *   \param ulRecvBufferSize Length of the receive buffer
 *   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
 *   \param pvPktCallback    Packet callback for unhandled receive packets
 *   \param pvUser           User data for callback function
 *   \return NETANA_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t netana_transfer_packet( NETANA_HANDLE         hDev,
                                NETANA_PACKET*        ptSendPkt,
                                NETANA_PACKET*        ptRecvPkt,
                                uint32_t              ulRecvBufferSize,
                                uint32_t              ulTimeout,
                                PFN_RECV_PKT_CALLBACK pvPktCallback,
                                void*                 pvUser)
{
        int32_t lCount = 0;
        int32_t lRet   = NETANA_NO_ERROR;

        if( (lRet = netana_put_packet(hDev, ptSendPkt, ulTimeout)) == NETANA_NO_ERROR)
        {
                do
                {
                        if( (lRet = netana_get_packet(hDev, ulRecvBufferSize, ptRecvPkt, ulTimeout)) == NETANA_NO_ERROR)
                        {
                                /* Check if we got the answer */
                                if((le32toh(ptRecvPkt->tHeader.ulCmd) & ~1) ==
                                    le32toh(ptSendPkt->tHeader.ulCmd) )
                                {
                                        /* Check rest of packet data */
                                        if ( (ptRecvPkt->tHeader.ulSrc   == ptSendPkt->tHeader.ulSrc)    &&
                                             (ptRecvPkt->tHeader.ulId    == ptSendPkt->tHeader.ulId)     &&
                                             (ptRecvPkt->tHeader.ulSrcId == ptSendPkt->tHeader.ulSrcId)  )
                                        {
                                                /* We got the answer message */
                                                /* lRet = ptRecvPkt->tHeader.ulState; */
                                                /* Do not deliever back this information */
                                                break;
                                        } else
                                        {
                                                /* This is not our packet, check if the user wants it */
                                                if( NULL != pvPktCallback)
                                                {
                                                        pvPktCallback(ptRecvPkt, pvUser);
                                                }
                                        }
                                }
                                /* Reset error, in case we might drop out of the loop, with no proper answer,
                                *          returning a "good" state */
                                lRet = NETANA_TRANSFER_TIMEOUT;
                                lCount++;
                        }else
                        {
                          /* Error during packet receive */
                          break;
                        }
                } while ( lCount < 10);
        }
        return lRet;
}
