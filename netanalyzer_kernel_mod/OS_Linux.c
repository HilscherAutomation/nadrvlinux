/**************************************************************************************

Copyright (c) Hilscher GmbH. All Rights Reserved.

**************************************************************************************

        Filename:
        $Workfile: OS_Linux.c $
        Last Modification:
        $Author: sebastiand $
        $Modtime: 30.09.09 14:58 $
        $Revision: 3111 $

        Targets:
        Linux        : yes

        Description:
        Linux O/S abstraction for toolkit

        Changes:

        Version   Date        Author       Description
        ----------------------------------------------------------------------------------
        3        02.06.15     SD       - Added mutex functions (lock with timeout)
        2        24.04.14     SD       - Bugfix: OS_WaitEvent() may not block, since
                                         timeout results to 0 if timeout < 1000ms
                                         (now use kernel macro instead)
        1        -/-          SD       - initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file OS_Linux.c
*   Linux O/S abstraction for toolkit                                        */
/*****************************************************************************/

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <linux/spinlock.h>

#include <linux/uaccess.h>

#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>

#include "netana_errors.h"
#include "toolkit/OS_Dependent.h"
#include "netana.h"


#define IRQ_CFG_REG_OFFSET          0xfff0
#define IRQ_ENABLE_MASK             0x80000000

#define IRQ_STACK_MIN_SIZE          0x1000 /* Stack size needed by IRQ Thread
                                        calling Toolkit's ISR/DSR Handler*/

#define BLOCK64 sizeof(uint64_t)
#define BLOCK32 sizeof(uint32_t)

const struct firmware* ptBootLoader = NULL;
const struct firmware* ptFirmware   = NULL;
const struct firmware* ptIdentInfo  = NULL;

struct NETANA_DEVINSTANCE_T* s_pdeviceinstance;

extern irqreturn_t netana_isr(int irq, void *dev_info);

int ReadPCIConfig( struct pci_dev* pcidevice, int size, char* buffer)
{
        int      tempSize;
        uint32_t ulRet;

        tempSize = size;

        while(tempSize)
        {
                ulRet = pci_read_config_byte( pcidevice, (size-tempSize),
                                                &buffer[size-tempSize]);
                tempSize--;

                if (ulRet)
                        printk("error reading pci config %X\n", ulRet);
        }
        return 0;
}

int WritePCIConfig( struct pci_dev* pcidevice, int size, char* buffer)
{
        int      tempSize;
        uint32_t ulRet;

        tempSize = size;

        while(tempSize) {
                ulRet = pci_write_config_byte( pcidevice, size-tempSize,
                                                buffer[size-tempSize]);

                if (ulRet)
                        printk("error writing pci config %X\n", ulRet);

                tempSize--;
        }
        return 0;
}


/*****************************************************************************/
/*! O/S Specific initialization
*     \return NETANA_NO_ERROR on success                                      */
/*****************************************************************************/
int32_t OS_Init(void)
{
        return 1;
}

/*****************************************************************************/
/*! O/S Specific de-initialization                                           */
/*****************************************************************************/
void OS_Deinit(void)
{
}

void* OS_Memalloc(uint32_t ulSize)
{
        return kzalloc( ulSize, GFP_KERNEL);
}

void  OS_Memfree(void* pvMem)
{
        kfree( pvMem);
}

/*****************************************************************************/
/*! Memset wrapper
*     \param pvMem   Memory to set
*     \param bFill   Fill byte
*     \param ulSize  Size of the fill block                                  */
/*****************************************************************************/
void OS_Memset(void* pvMem, unsigned char bFill, uint32_t ulSize)
{
        memset(pvMem, bFill, ulSize);
}

/*****************************************************************************/
/*! Memcopy wrapper
*     \param pvDest  Destination pointer
*     \param pvSrc   Source pointer
*     \param ulSize  Size to copy                                            */
/*****************************************************************************/
void OS_Memcpy(void* pvDest, const void* pvSrc, unsigned long ulSize)
{
        uint32_t ulDestAlignment = (uint32_t)(unsigned long)pvDest & 0x03;
        uint32_t ulSrcAlignment  = (uint32_t)(unsigned long)pvSrc & 0x03;
        uint8_t *pDest8 = (uint8_t*)pvDest;
        uint8_t *pSrc8 = (uint8_t*)pvSrc;

        if ( (ulDestAlignment == 0) &&
                (ulSrcAlignment == 0) )
        {
                uint32_t *pDest32 = (uint32_t*)pvDest;
                uint32_t *pSrc32  = (uint32_t*)pvSrc;

        while(ulSize>=BLOCK64) {
                *(pDest32)++ = *(pSrc32)++;
                *(pDest32)++ = *(pSrc32)++;
                ulSize-=BLOCK64;
        }
        while(ulSize>=BLOCK32) {
                *(pDest32)++ = *(pSrc32)++;
                ulSize-=BLOCK32;
        }
        pDest8 = (uint8_t*)pDest32;
        pSrc8 = (uint8_t*)pSrc32;
        }
        while(ulSize--)
                *(pDest8++) = *(pSrc8++);
}

/*****************************************************************************/
/*! Memcompare wrapper
*     \param pvBuf1  First compare buffer
*     \param pvBuf2  Second compare buffer
*     \param ulSize  Size to compare
*     \return 0 if blocks are equal                                          */
/*****************************************************************************/
int OS_Memcmp(const void* pvBuf1, const void* pvBuf2, unsigned long ulSize)
{
        return memcmp(pvBuf1, pvBuf2, ulSize);
}

/*****************************************************************************/
/*! Read PCI configuration area of specified card
*     \param pvOSDependent OS Dependent parameter to identify card
*     \return Pointer to configuration data (passed to WritePCIConfig)       */
/*****************************************************************************/
void* OS_ReadPCIConfig(void* pvOSDependent)
{
        struct netana_info *ptDeviceInfo = (struct netana_info*)pvOSDependent;
        int                pci_ret;
        void               *pci_buf;

        if(!pvOSDependent)
                return NULL;

        if(!ptDeviceInfo->pcidevice)
                return NULL;

        pci_buf = kzalloc(256, GFP_KERNEL);
        if(!pci_buf) {
                //perror("pci_buf malloc failed");
                return NULL;
        }

        pci_ret = ReadPCIConfig(ptDeviceInfo->pcidevice, 256, pci_buf);

        return pci_buf;
}

/*****************************************************************************/
/*! Restore PCI configuration
*     \param pvOSDependent OS Dependent parameter to identify card
*     \param pvPCIConfig   Pointer returned from ReadPCIConfig               */
/*****************************************************************************/
void OS_WritePCIConfig(void* pvOSDependent, void* pvPCIConfig)
{
        int                pci_ret;
        struct netana_info *ptDeviceInfo = (struct netana_info*)pvOSDependent;

        if(!pvOSDependent)
                return;

        if(!ptDeviceInfo->pcidevice)
                return;

        pci_ret = WritePCIConfig(ptDeviceInfo->pcidevice, 256, pvPCIConfig);

        kfree(pvPCIConfig);
}

struct file_info
{
        struct file  *pfile;
        uint64_t     ullWriteOffset;
        uint64_t     ullReadOffset;
        int          origin;
};

/* !!! NOTE !!! */
/* for every call to FileOpen() a prior call to USER_Get[XXX]File is required */
/* !!! NOTE !!! */
/*****************************************************************************/
/*! Open file for reading
*     \param szFilename   File to open (including path)
*     \param pulFileSize  Returned size of the file in bytes
*     \return Handle to the file, NULL on failure                            */
/*****************************************************************************/
void* OS_FileOpen(char* szFilename, uint64_t * pulFileSize, uint32_t ulFlags)
{
        struct netana_info        *netAnaInfo    = NULL;
        int32_t                   iRet           = -1;
        const struct              firmware* ptFW = NULL;
        NETANA_TKIT_FILE_INFO_T   tBootFileInfo  = {{0}};
        NETANA_TKIT_FILE_INFO_T   tFWFileInfo    = {{0}};
        NETANA_TKIT_FILE_INFO_T   tIdentFileInfo = {{0}};

        /* !!! NOTE !!! */
        /* In kernel mode we are not allowed to do normal file operations!                */
        /* OS_FileOpen() can only be used for firmware requests (see request_firmware()). */
        /* Due to the the firmware request parameter, the device information is reuired.  */
        /* So for every call to FileOpen() a prior call to USER_Get[XXX]File is required  */
        /* to make corresponding device known.                                            */
        /* !!! NOTE !!! */
        if (NULL == s_pdeviceinstance)
                return NULL;

        netAnaInfo = (struct netana_info*)s_pdeviceinstance->pvOSDependent;

        USER_GetBootloaderFile( s_pdeviceinstance, &tBootFileInfo);
        USER_GetFirmwareFile( s_pdeviceinstance, &tFWFileInfo);
        USER_GetIdentInfoFile( s_pdeviceinstance, &tIdentFileInfo);

        if (0 == strcmp( tBootFileInfo.szFullFileName, szFilename)) {
#ifdef NETANA_PM_AWARE
                /* in case of system resume check if bootloader is available in memory, */
                /* because we can not call the user process reuqest_firmware() yet      */
                if (netAnaInfo->resumephase && netAnaInfo->drv_info->bl_copy.data) {
                        ptFW         = &netAnaInfo->drv_info->bl_copy;
                        *pulFileSize = netAnaInfo->drv_info->bl_copy.size;
                        return (void*)ptFW;
                }
#endif
                if (0 == (iRet = request_firmware( &ptBootLoader,
                                                        tBootFileInfo.szFullFileName,
                                                        netAnaInfo->device))) {
                        ptFW         = ptBootLoader;
                        *pulFileSize = ptBootLoader->size;
                }
        } else if (0 == strcmp(tFWFileInfo.szFullFileName, szFilename)) {
#ifdef NETANA_PM_AWARE
                if (netAnaInfo->resumephase && netAnaInfo->drv_info->fw_copy.data) {
                        ptFW         = &netAnaInfo->drv_info->fw_copy;
                        *pulFileSize = netAnaInfo->drv_info->fw_copy.size;
                        return (void*)ptFW;
                }
#endif
                if (0 == (iRet = request_firmware( &ptFirmware,
                                                tFWFileInfo.szFullFileName,
                                                netAnaInfo->device))) {
                        ptFW         = ptFirmware;
                        *pulFileSize = ptFirmware->size;
                }
        } else if (0 == strcmp(tIdentFileInfo.szFullFileName, szFilename)) {
                if (0 == (iRet = request_firmware( &ptFirmware,
                                        tIdentFileInfo.szFullFileName,
                                        netAnaInfo->device))) {
                        ptFW         = ptFirmware;
                        *pulFileSize = ptFirmware->size;
                }
        }
#ifdef NETANA_PM_AWARE
        /* if the driver is pm aware we need to store the bootloader and the */
        /* firmware, when the system resumes we are not able to call any     */
        /* user process like request_firmware() to retrieve the files        */
        if (ptBootLoader) {
                netAnaInfo->drv_info->bl_copy.size = ptBootLoader->size;
                netAnaInfo->drv_info->bl_copy.data = kzalloc( ptBootLoader->size, GFP_KERNEL);
                memcpy( (void*)netAnaInfo->drv_info->bl_copy.data, ptFW->data, ptBootLoader->size);
                ptBootLoader = NULL;
        }
        if (ptFirmware) {
                netAnaInfo->drv_info->fw_copy.size = ptFirmware->size;
                netAnaInfo->drv_info->fw_copy.data = kzalloc( ptFirmware->size, GFP_KERNEL);
                memcpy( (void*)netAnaInfo->drv_info->fw_copy.data, ptFW->data, ptFirmware->size);
                ptFirmware = NULL;
        }
#endif
        return (void*)ptFW;
}

/*****************************************************************************/
/*! Read data from file
*     \param pvFile    Handle to the file (acquired by OS_FileOpen)
*     \param ulOffset  Offset to read from
*     \param ulSize    Size to read
*     \param pvBuffer  Buffer to read data into
*     \return number of bytes read                                           */
/*****************************************************************************/
unsigned long OS_FileRead(void* pvFile,  unsigned long ulOffset, unsigned long ulSize, void* pvBuffer)
{
        const struct firmware* ptFW = pvFile;

        if (ptFW) {
                OS_Memcpy(pvBuffer, &ptFW->data[ulOffset], ulSize);
        } else {
                ulSize = 0;
        }

        return ulSize;
}

unsigned long OS_FileWrite(void* pvFile, unsigned long ulSize, void* pvBuffer)
{
        return 0;
}

unsigned long OS_FileSeek(void* pvFile,  unsigned long ulOffset)
{
        return 0;
}


/*****************************************************************************/
/*! Close open file
*     \param pvFile    Handle to the file (acquired by OS_FileOpen)          */
/*****************************************************************************/
void OS_FileClose(void* pvFile)
{
        const struct  firmware* ptFW       = pvFile;
#ifdef NETANA_PM_AWARE
        struct netana_info*     netAnaInfo = (struct netana_info*)s_pdeviceinstance->pvOSDependent;

        if (netAnaInfo->resumephase) {
                return;
        } else {
#else
        {
#endif
                if (ptFW)
                        release_firmware(ptFW);
        }
        s_pdeviceinstance = NULL;
}

/*****************************************************************************/
/*! Get Millisecond counter value (used for timeout handling)
*     \return Counter value with a resolution of 1ms                         */
/*****************************************************************************/
uint32_t OS_GetMilliSecCounter(void)
{
        uint32_t msec_count;
        uint32_t curjiffies = jiffies;

        msec_count = curjiffies * 1000 / HZ;

        return msec_count;
}

/*****************************************************************************/
/*! Sleep for the given time
*     \param ulSleepTimeMs Time in ms to sleep (0 will sleep for 50us)       */
/*****************************************************************************/
void OS_Sleep(unsigned long ulSleepTimeMs)
{
        if (ulSleepTimeMs == 0)
                udelay(50);
        else
                msleep( ulSleepTimeMs);
}

/*****************************************************************************/
/*! Create Lock (Usually same as mutex, but does not support timed waiting)
*     \return Handle to created lock                                         */
/*****************************************************************************/
void* OS_CreateLock(void)
{
        struct mutex* ptlock = kzalloc( sizeof(struct mutex), GFP_KERNEL);
        if (ptlock)
                mutex_init(ptlock);

        return ptlock;
}

/*****************************************************************************/
/*! Acquire a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_EnterLock(void* pvLock)
{
        struct mutex* ptlock = (struct mutex*)pvLock;
        mutex_lock(ptlock);
}

/*****************************************************************************/
/*! Release a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_LeaveLock(void* pvLock)
{
        struct mutex* ptlock = (struct mutex*)pvLock;
        mutex_unlock(ptlock);
}

/*****************************************************************************/
/*! Delete a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_DeleteLock(void* pvLock)
{
        kfree(pvLock);
}

/*****************************************************************************/
/*! Compare strings
*     \param pszBuf1  String buffer 1
*     \param pszBuf2  String buffer 2
*     \return 0 if strings are equal                                         */
/*****************************************************************************/
int OS_Strcmp(const char* pszBuf1, const char* pszBuf2)
{
        return strcmp( pszBuf1, pszBuf2);
}

/*****************************************************************************/
/*! Compare strings case insensitive
*     \param pszBuf1  String buffer 1
*     \param pszBuf2  String buffer 2
*     \param ulLen    Maximum length to compare
*     \return 0 if strings are equal                                         */
/*****************************************************************************/
int OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen)
{
        return strncmp(pszBuf1, pszBuf2, ulLen);
}

/*****************************************************************************/
/*! Get length of string
*     \param szText  Text buffer
*     \return Length of given string                                         */
/*****************************************************************************/
int OS_Strlen(const char* szText)
{
        return strlen(szText);
}

/*****************************************************************************/
/*! Copy string to destination buffer
*     \param szText   Destination string
*     \param szSource Source string
*     \param ulLen    Maximum length to copy
*     \return Pointer to szDest                                              */
/*****************************************************************************/
char* OS_Strncpy(char* szDest, const char* szSource, uint32_t ulLen)
{
        memcpy(szDest, szSource,ulLen);
        return szDest;
}

/*****************************************************************************/
/*! Map driver pointer to user space
*     \param pvDriverMem  Pointer to driver memory
*     \param ulMemSize    Size of the memory to map
*     \param ppvMappedMem Returned mapped pointer
*     \param os_dependent OS Dependent parameter in DEVICEINSTANCE
*     \return Handle to mapping, NULL on error                               */
/*****************************************************************************/
void* OS_MapUserPointer(void* pvDriverMem, uint32_t ulMemSize,
                        void** ppvMappedMem, void *os_dependent)
{
        return NULL;
}

/*****************************************************************************/
/*! Unmap previously mapped user space pointer
*     \param phMapping  Handle returned from OS_MapUserPointer
*     \param os_dependent OS Dependent parameter in DEVICEINSTANCE
*     \return 0 on error                                                     */
/*****************************************************************************/
int OS_UnmapUserPointer(void* phMapping, void *os_dependent)
{
        return 1;
}

typedef struct twaitstruct {
        wait_queue_head_t ptwait_head;
        int               fEventSet;

} twaitstruct_T;

/*****************************************************************************/
/*! Create event
*     \return Handle to created event                                        */
/*****************************************************************************/
void* OS_CreateEvent(void)
{
        twaitstruct_T* ptwait = NULL;

        ptwait = kzalloc(sizeof(twaitstruct_T), GFP_KERNEL | GFP_ATOMIC);

        if (!ptwait)
                printk("error allocating event\n");


        if (ptwait)
        {
                ptwait->fEventSet = 0;
                init_waitqueue_head( &ptwait->ptwait_head);
        } else {
                return NULL;
        }
        return (void*)ptwait;
}

/*****************************************************************************/
/*! Signal event
*     \param pvEvent Handle to event                                         */
/*****************************************************************************/
void OS_SetEvent(void* pvEvent)
{
        twaitstruct_T* ptwait = (twaitstruct_T*) pvEvent;

        ptwait->fEventSet = 1;
        /* call wake up with '_sync' since this variant will prevent the currently running task from beeing scheduled */
        wake_up_interruptible_sync( &ptwait->ptwait_head);
}

/*****************************************************************************/
/*! Reset event
*     \param pvEvent Handle to event                                         */
/*****************************************************************************/
void OS_ResetEvent(void* pvEvent)
{
        twaitstruct_T* ptwait = (twaitstruct_T*) pvEvent;
        ptwait->fEventSet = 0;
}

/*****************************************************************************/
/*! Delete event
*     \param pvEvent Handle to event                                         */
/*****************************************************************************/
void OS_DeleteEvent(void* pvEvent)
{
        twaitstruct_T* ptwait = (twaitstruct_T*) pvEvent;

        if (ptwait)
                kfree(ptwait);
}

/*****************************************************************************/
/*! Wait for event
*     \param pvEvent   Handle to event
*     \param ulTimeout Timeout in ms to wait for event
*     \return CIFX_EVENT_SIGNALLED if event was set, CIFX_EVENT_TIMEOUT otherwise */
/*****************************************************************************/
uint32_t OS_WaitEvent(void* pvEvent, uint32_t ulTimeout)
{
        uint32_t       ret = 0;
        twaitstruct_T* ptwait = (twaitstruct_T*) pvEvent;

        ulTimeout = wait_event_interruptible_timeout( ptwait->ptwait_head,
                                                        (ptwait->fEventSet == 1),
                                                        msecs_to_jiffies(ulTimeout));

        if (ptwait->fEventSet == 0) {
                ret = NETANA_EVENT_TIMEOUT ;
        } else {
                ret               = NETANA_EVENT_SIGNALLED;
                ptwait->fEventSet = 0;
        }
        return ret;
}

void  OS_EnableDeviceInterrupts(void* pvOSDependent)
{
        struct netana_info *ptDeviceInfo = (struct netana_info*)pvOSDependent;

        /* register irq handler */
        if (request_irq(ptDeviceInfo->irq, netana_isr, IRQF_SHARED, ptDeviceInfo->devname, (void*)ptDeviceInfo))
                printk(KERN_ERR "error requesting IRQ\n");
        else /* enable interrupts */
                netana_tkit_enable_hwinterrupts(&ptDeviceInfo->tkdevice->deviceinstance);
}

void  OS_DisableDeviceInterrupts(void* pvOSDependent)
{
        struct netana_info *ptDeviceInfo = (struct netana_info*)pvOSDependent;

        netana_tkit_disable_hwinterrupts(&ptDeviceInfo->tkdevice->deviceinstance);
        /* free irq */
        free_irq(ptDeviceInfo->irq, (void*)ptDeviceInfo);
}

int OS_Snprintf(char* szBuffer, uint32_t ulSize, const char* szFormat, ...)
{
        va_list vaformat;
        int     strlen;

        va_start( vaformat, szFormat);

        strlen = vsnprintf( szBuffer, ulSize, szFormat, vaformat);

        va_end(vaformat);

        return strlen;
}

/* private helper structure */
struct priv_mutex
{
        wait_queue_head_t waitqueue;
        int               flag;
        struct mutex      lock;
};

/*****************************************************************************/
/*! Create an Mutex object for locking code sections
 *   \return handle to the mutex object                                      */
/*****************************************************************************/
void* OS_CreateMutex(void)
{
        struct priv_mutex* tmp_mutex = kzalloc( sizeof(struct priv_mutex), GFP_KERNEL);

        if (tmp_mutex) {
                init_waitqueue_head(&tmp_mutex->waitqueue);
                mutex_init(&tmp_mutex->lock);
                tmp_mutex->flag = 0;
        }
        return tmp_mutex;
}

/*****************************************************************************/
/*! Wait for mutex
 *   \param pvMutex    Handle to the Mutex locking object
 *   \param ulTimeout  Wait timeout
 *   \return !=0 on succes                                                   */
/*****************************************************************************/
int OS_WaitMutex(void* pvMutex, uint32_t ulTimeout)
{
        struct priv_mutex* tmp_mutex = (struct priv_mutex*)pvMutex;
        uint32_t           timeout   =  msecs_to_jiffies(ulTimeout);
        int                ret       = 0;
        int                exclusive = 0;

        if (!tmp_mutex)
                return 0;

        if (0 == timeout) {
                exclusive = mutex_trylock(&tmp_mutex->lock);
        } else {
                do
                {
                        timeout = wait_event_timeout(tmp_mutex->waitqueue, (tmp_mutex->flag == 0), timeout);
                        /* mutex was released so try to aquire the lock */
                        if ((exclusive = mutex_trylock(&tmp_mutex->lock)) && (tmp_mutex->flag == 0))
                                break;

                        if (exclusive) {
                                mutex_unlock(&tmp_mutex->lock);
                                exclusive = 0;
                        }
                } while (timeout > 0);
        }
        /* we got exclusive access */
        if (exclusive) {
                if (tmp_mutex->flag == 0) {
                        ret             = 1;
                        tmp_mutex->flag = 1;
                }
                mutex_unlock(&tmp_mutex->lock);
        }
        return ret;
}

/*****************************************************************************/
/*! Release a mutex section section
 *   \param pvMutex Handle to the locking object                             */
/*****************************************************************************/
void OS_ReleaseMutex(void* pvMutex)
{
        struct priv_mutex* tmp_mutex = (struct priv_mutex*)pvMutex;

        mutex_lock(&tmp_mutex->lock);
        tmp_mutex->flag = 0;
        wake_up_interruptible(&tmp_mutex->waitqueue);
        mutex_unlock(&tmp_mutex->lock);
}

/*****************************************************************************/
/*! Delete a Mutex object
 *   \param pvMutex Handle to the mutex object being deleted                 */
/*****************************************************************************/
void OS_DeleteMutex(void* pvMutex)
{
        kfree(pvMutex);
}
