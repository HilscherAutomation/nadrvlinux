/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include "toolkit/netana_toolkit.h"
#include "netana.h"

extern struct NETANA_DEVINSTANCE_T* s_pdeviceinstance;

/*****************************************************************************/
/*! \file USER_Linux.c
*   Linux user specific implementation for finding firmware files, etc.      */
/*****************************************************************************/

typedef struct APP_PROCESS_Ttag
{
        TAILQ_ENTRY(APP_PROCESS_Ttag) tList;
        pid_t                         pid;
        uint32_t                      ulDeviceClass;

} APP_PROCESS_T;

TAILQ_HEAD(APP_PROCESS_LIST, APP_PROCESS_Ttag);

struct APP_PROCESS_LIST g_tAppProcessList = TAILQ_HEAD_INITIALIZER(g_tAppProcessList);



/*****************************************************************************/
/*! Print a trace message from cifX toolkit
*     \param ptDevInstance  Device instance the trace is coming from
*     \param ulTraceLevel   see TRACE_LVL_XXX defines
*     \param szFormat       printf style format string
*     \param ...            printf arguments                                 */
/*****************************************************************************/
void USER_Trace(struct NETANA_DEVINSTANCE_T* ptDevInst,
                uint32_t ulTraceLevel, char* szFormat, ...)
{
        va_list vaformat;
        int ulSize = 20;
        int strlen;
        char* szBuffer;

        szBuffer = kmalloc(ulSize, GFP_KERNEL | GFP_ATOMIC);

        while(1) {
                va_start( vaformat, szFormat);
                strlen = vsnprintf( szBuffer, ulSize, szFormat, vaformat);
                va_end( vaformat);

                if ((strlen > -1) && (strlen < ulSize) ) {
                        printk( "%s\n", szBuffer);
                        kfree(szBuffer);
                        return;
                }

                if (strlen > -1) {
                        szBuffer = krealloc(szBuffer, (ulSize+=1), GFP_KERNEL | GFP_ATOMIC);
                        if (szBuffer == NULL) {
                                kfree(szBuffer);
                                return;
                        }
                }  else {
                        kfree(szBuffer);
                        return;
                }
        }
}

#define PARSER_BUFFER_SIZE  1024

/*****************************************************************************/
/*! Returns the number of firmware files to be downloaded on the given
*   device/channel
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)
*     \return Number of files (used for USER_GetFirmwareFile()               */
/*****************************************************************************/
//uint32_t USER_GetFirmwareFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
//{

//return 0;
//}

/*****************************************************************************/
/*! Returns filename of the file to be downloaded on the given device/channel
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)
*     \param ulIdx          Number of the file (0..USER_GetFirmwareFileCount)
*     \param ptFileInfo     Full and short filename of the file
*     \return !=0 on success                                                 */
/*****************************************************************************/
void USER_GetFirmwareFile(struct NETANA_DEVINSTANCE_T* ptDevInst,
                        PNETANA_TKIT_FILE_INFO_T ptFileInfo)
{
        /* needs to be set to be able call OS_OpenFile() */
        s_pdeviceinstance = (struct NETANA_DEVINSTANCE_T*)ptDevInst;

        switch (s_pdeviceinstance->usDeviceClass)
        {
        case NANL_DEV_CLASS:
        {
                strcpy(ptFileInfo->szShortFileName, "NANL-500.NXF");
                /*!< Full filename (including path) to file */
                strcpy(ptFileInfo->szFullFileName,"netanalyzer/NANL-500.nxf");
        }
        break;
        case NSCP_DEV_CLASS:
        {
                strcpy(ptFileInfo->szShortFileName, "NSCP-100.NXF");
                /*!< Full filename (including path) to file */
                strcpy(ptFileInfo->szFullFileName,"netanalyzer/NSCP-100.nxf");
        }
        break;
        case CIFX_DEV_CLASS:
        {
                strcpy(ptFileInfo->szShortFileName, "NANLCIFX.NXF");
                /*!< Full filename (including path) to file */
                strcpy(ptFileInfo->szFullFileName,"netanalyzer/NANLCIFX.nxf");
        }
        break;
        default:
          if(0 != s_pdeviceinstance->usDeviceClass)
            USER_Trace(ptDevInst, NETANA_TRACELEVEL_ERROR,
              "Invalid device class %d (no firmware available)\n",
              s_pdeviceinstance->usDeviceClass);
        break;
        }
}

/*****************************************************************************/
/*! Retrieve the full and short filename of a bootloader file to download to
*   the given device
*     \param ptDevInstance Device Instance containing all device data
*     \param ptFileInfo    Pointer to returned file information              */
/*****************************************************************************/
void USER_GetBootloaderFile(struct NETANA_DEVINSTANCE_T* ptDevInst,
                        PNETANA_TKIT_FILE_INFO_T ptFileInfo)
{
        /* needs to be set to be able call OS_OpenFile() */
        s_pdeviceinstance = (struct NETANA_DEVINSTANCE_T*)ptDevInst;

        strcpy(ptFileInfo->szShortFileName, "NETX100-BSL.bin");
        /*!< Full filename (including path) to file */
        strcpy(ptFileInfo->szFullFileName,"netanalyzer/NETX100-BSL.bin");
}


/*****************************************************************************/
/*! Retrieve the full and short filename of a idenet info file to download to
*   the given device
*     \param ptDevInstance Device Instance containing all device data
*     \param ptFileInfo    Pointer to returned file information              */
/*****************************************************************************/
void USER_GetIdentInfoFile(struct NETANA_DEVINSTANCE_T* ptDevInst,
                        PNETANA_TKIT_FILE_INFO_T ptFileInfo)
{
        /* needs to be set to be able call OS_OpenFile() */
        s_pdeviceinstance = (struct NETANA_DEVINSTANCE_T*)ptDevInst;

        strcpy(ptFileInfo->szShortFileName, "lic.nxl");
        /*!< Full filename (including path) to file */
        strcpy(ptFileInfo->szFullFileName, ptDevInst->tIdentInfoFile.szIdentInfoFileName);
}

/*****************************************************************************/
/*! Sets the device filter.
*     \param ptDevInst      Device instance
*     \param ptFileInfo     Pointer to file information structure            */
/*****************************************************************************/
int32_t USER_SetDevClassFilter( uint32_t ulDeviceFilter)
{
        int32_t lRet = NETANA_INVALID_PARAMETER;
        APP_PROCESS_T*      ptProcessInfo = NULL;
        struct task_struct* curtask       = get_current();
        pid_t               pid           = curtask->group_leader->pid;
        uint32_t            ulInvMask     = (uint32_t)~(NETANA_DEV_CLASS_NANL_500 | NETANA_DEV_CLASS_NSCP_100| NETANA_DEV_CLASS_CIFX);

        if ( (0 != (ulDeviceFilter & ulInvMask)) || (ulDeviceFilter == 0))
        {
                return NETANA_INVALID_PARAMETER;
        }

        TAILQ_FOREACH( ptProcessInfo, &g_tAppProcessList, tList)
        {
                if ((ptProcessInfo != NULL) && (ptProcessInfo->pid == pid))
                {
                        ptProcessInfo->ulDeviceClass = ulDeviceFilter;
                        lRet = NETANA_NO_ERROR;
                        break;
                }
        }
        if (NETANA_NO_ERROR != lRet)
        {
                lRet = NETANA_FUNCTION_FAILED;
                if (NULL != (ptProcessInfo = OS_Memalloc(sizeof(APP_PROCESS_T))))
                {
                        ptProcessInfo->pid           = pid;
                        ptProcessInfo->ulDeviceClass = ulDeviceFilter;
                        TAILQ_INSERT_TAIL(&g_tAppProcessList, ptProcessInfo, tList);

                        lRet = NETANA_NO_ERROR;
                }
        }
        return lRet;
}

/*****************************************************************************/
/*! Retreives the device class which should be filtered.
*     \returns the class to filter                                           */
/*****************************************************************************/
uint32_t USER_GetDevClassFilter( void)
{
        APP_PROCESS_T*      ptProcessInfo = NULL;
        struct task_struct* curtask       = get_current();
        pid_t               pid           = curtask->group_leader->pid;
        uint32_t            ulRet         = 0;

                /* get device class of calling thread */
        TAILQ_FOREACH( ptProcessInfo, &g_tAppProcessList, tList)
        {
                if ((ptProcessInfo != NULL) && (ptProcessInfo->pid == pid))
                {
                        ulRet = ptProcessInfo->ulDeviceClass;
                        break;
                }
        }
        return ulRet;
}

/*****************************************************************************/
/*! Clears the device class which should be filtered.
*     \returns the class to filter                                           */
/*****************************************************************************/
void USER_ClearDevClassFilter( void)
{
        APP_PROCESS_T*      ptProcessInfo = NULL;
        struct task_struct* curtask       = get_current();
        pid_t               pid           = curtask->group_leader->pid;

        /* get device class of calling thread */
        TAILQ_FOREACH( ptProcessInfo, &g_tAppProcessList, tList)
        {
                if ((ptProcessInfo != NULL) && (ptProcessInfo->pid == pid))
                {
                        TAILQ_REMOVE(&g_tAppProcessList, ptProcessInfo, tList);
                        OS_Memfree( ptProcessInfo);
                        break;
                }
        }
}

/*****************************************************************************/
/*! Returns the alias name of the corresponding device class.
*     \param ulDeviceClass  device class
*     \param szAliasName    returned alias name
*     \param ulNameLen      length of buffer szAliasName
*     \returns 0 on success                                                  */
/*****************************************************************************/
int32_t USER_GetAliasName( uint32_t ulDeviceClass, char* szAliasName, uint32_t ulNameLen)
{
        int32_t lRet = NETANA_NO_ERROR;

        switch(ulDeviceClass)
        {
        case NSCP_DEV_CLASS:
                OS_Snprintf( szAliasName, ulNameLen, "%s", "netSCOPE");
        break;
        case NANL_DEV_CLASS:
                OS_Snprintf( szAliasName, ulNameLen, "%s", "netANALYZER");
        break;
        case CIFX_DEV_CLASS:
                OS_Snprintf( szAliasName, ulNameLen, "%s", "cifXANALYZER");
        break;
        default:
                lRet = NETANA_INVALID_PARAMETER;
        break;
        }
        return lRet;
}
