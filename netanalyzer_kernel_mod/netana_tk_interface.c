#include <linux/sched.h>

#include "netana_tk_interface.h"

#define DEFAULT_TIMEOUT 10

extern void GenerateDeviceInformation(struct NETANA_DEVINSTANCE_T* ptDevice,
                                uint32_t ulDevInfoSize,
                                NETANA_DEVICE_INFORMATION_T* ptDevInfo);
/**
* task_info - contains device specific information
*/
struct task_info {
        struct list_head list;
        pid_t            pid;
        int32_t          lasterror;
        uint32_t         timeout;
};

void initlist(struct netana_info *ptdeviceinfo)
{
        INIT_LIST_HEAD(&ptdeviceinfo->act_task_list);
        mutex_init(&ptdeviceinfo->task_list_lock);
}

/**
* cleanlist - cleans list of device specific run-time information
* (error, timout, ...)
* @dev_info:    Pointer to the devices info structure
*/
void cleanlist(struct netana_info *ptdeviceinfo)
{
        struct list_head *task_list = &ptdeviceinfo->act_task_list;
        struct list_head *pos;
        struct list_head *next;

        mutex_lock(&ptdeviceinfo->task_list_lock);
        list_for_each_safe(pos, next,task_list) {
                struct task_info *tmp;
                if (list_empty(task_list))
                        break;

                tmp = list_entry(pos, struct task_info, list);
                list_del(pos);
                kfree(tmp);
        }
        mutex_unlock(&ptdeviceinfo->task_list_lock);
}

/**
* get_err - returns the last error of the device
* errors are associated by the process id
* @dev_info:    Pointer to the devices info structure
*/
int32_t get_err( struct netana_info *ptdeviceinfo)
{
        struct task_struct *curtask   = get_current();
        pid_t              pid        = curtask->pid;
        struct list_head   *task_list = &ptdeviceinfo->act_task_list;
        int                found      = 0;
        struct list_head   *pos;
        int32_t            ret        = NETANA_NO_ERROR;

        mutex_lock(&ptdeviceinfo->task_list_lock);
        list_for_each(pos, task_list) {
                struct task_info *tmp;
                tmp = list_entry(pos, struct task_info, list);
                if (tmp->pid == pid) {
                        found = 1;
                        ret   = tmp->lasterror;
                        break;
                }
        }
        mutex_unlock(&ptdeviceinfo->task_list_lock);
        return ret;
}

/**
* store_err - stores the last error of the device
* errors are associated by the process id
* @dev_info:    Pointer to the devices info structure
*/
ssize_t store_err( struct netana_info *ptdeviceinfo, int32_t lerror)
{
        /* store the error in the context of the calling process */
        if (!lerror) {
                return 0;
        } else {
                struct task_struct *curtask = get_current();
                pid_t               pid     = curtask->pid;
                int                 found   = 0;
                struct list_head    *pos;
                struct list_head    *task_list = &ptdeviceinfo->act_task_list;
                // store the error in the context of the calling process
                mutex_lock(&ptdeviceinfo->task_list_lock);
                if (!list_empty(task_list)) {
                        list_for_each(pos, task_list) {
                                struct task_info *tmp;
                                tmp = list_entry(pos, struct task_info, list);
                                if (tmp->pid == pid) {
                                        found = 1;
                                        tmp->lasterror = lerror;
                                }
                        }
                }
                if (!found) {
                        struct task_info *next_entry;
                        if ((next_entry = kzalloc(sizeof(struct task_info), GFP_KERNEL))) {
                                list_add_tail( &(next_entry->list), task_list);
                                next_entry->pid       = pid;
                                next_entry->lasterror = lerror;
                        }
                }
                mutex_unlock(&ptdeviceinfo->task_list_lock);
        }
        return -EPERM;
}

uint32_t get_timeout( struct netana_info *ptdeviceinfo)
{
        struct task_struct *curtask   = get_current();
        pid_t              pid        = curtask->group_leader->pid; /* get process id since this is a global setting */
        struct list_head   *task_list = &ptdeviceinfo->act_task_list;
        int                found      = 0;
        struct list_head   *pos;
        uint32_t           ret        = 0;

        mutex_lock(&ptdeviceinfo->task_list_lock);
        list_for_each(pos, task_list) {
                struct task_info *tmp;
                tmp = list_entry(pos, struct task_info, list);
                if (tmp->pid == pid) {
                        found = 1;
                        ret = tmp->timeout;
                }
        }
        mutex_unlock(&ptdeviceinfo->task_list_lock);

        return ret;
}

void store_timeout( struct netana_info *ptdeviceinfo, uint32_t ultimeout)
{
        /* store the error in the context of the calling process */
        struct task_struct *curtask = get_current();
        pid_t               pid     = curtask->group_leader->pid; /* store process id since this is a global setting */
        int                 found   = 0;
        struct list_head    *pos;
        struct list_head    *task_list = &ptdeviceinfo->act_task_list;
        // store the error in the context of the calling process
        mutex_lock(&ptdeviceinfo->task_list_lock);
        if (!list_empty(task_list)) {
                list_for_each(pos, task_list) {
                        struct task_info *tmp;
                        tmp = list_entry(pos, struct task_info, list);
                        if (tmp->pid == pid) {
                                found = 1;
                                tmp->timeout = ultimeout;
                        }
                }
        }
        if (!found) {
                struct task_info *next_entry;
                if ((next_entry = kzalloc(sizeof(struct task_info), GFP_KERNEL))) {
                        list_add_tail( &(next_entry->list), task_list);
                        next_entry->pid       = pid;
                        next_entry->timeout = ultimeout;
                }
        }
        mutex_unlock(&ptdeviceinfo->task_list_lock);
}

/**
* ATTRIBUTE SPECIFIC STORE/SHOW FUNCTIONS
* ATTRIBUTE SPECIFIC STORE/SHOW FUNCTIONS
* ATTRIBUTE SPECIFIC STORE/SHOW FUNCTIONS
*/
ssize_t netana_get_last_error( struct netana_info *ptdeviceinfo,
                        char *buf, void *pvuser_data)
{
        return sprintf( buf, "0x%X\n", get_err(ptdeviceinfo));
}

ssize_t netana_show_drvvers( struct netana_info *ptdeviceinfo,
                        char *buf, void *pvuser_data)
{
        return sprintf( buf, "%d %d %d %d %d %d %d %d\n",
                        NETANA_DRIVER_VERSION_MAJOR,
                        NETANA_DRIVER_VERSION_MINOR,
                        NETANA_DRIVER_VERSION_BUILD,
                        NETANA_DRIVER_VERSION_REVISION,
                        NETANA_TOOLKIT_VERSION_MAJOR,
                        NETANA_TOOLKIT_VERSION_MINOR,
                        NETANA_TOOLKIT_VERSION_BUILD,
                        NETANA_TOOLKIT_VERSION_REVISION);
}

ssize_t netana_show_dma_buffercnt( struct netana_info *ptdeviceinfo,
                                char *buf, void *pvuser_data)
{
        return sprintf( buf, "%d\n", g_ulDMABufferCount);
}

ssize_t netana_show_dma_buffersize( struct netana_info *ptdeviceinfo,
                                char *buf, void *pvuser_data)
{
        return sprintf( buf, "0x%X\n", g_ulDMABufferSize);
}

ssize_t netana_set_device_class( struct netana_info *ptdeviceinfo,
        const char *buf, size_t tsize, void *pvuser_data)
{
        ssize_t            ret   = tsize;
        int32_t            netana_error;
        uint32_t           ulDeviceClass;

        if (1 == sscanf( buf, "%d", &ulDeviceClass)) {
                if (NETANA_NO_ERROR != (netana_error = USER_SetDevClassFilter( ulDeviceClass)))
                        return store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}

ssize_t netana_get_device_class( struct netana_info *ptdeviceinfo,
        char *buf, void *pvuser_data)
{
        uint32_t ulDeviceClass;

        ulDeviceClass = USER_GetDevClassFilter();

        return sprintf( buf, "%d\n", ulDeviceClass);
}

/**
* netana_set_dsr_prio - changes priority of the dsr
* @dev_info:    Pointer to the devices info structure
*/
ssize_t netana_set_dsr_prio( struct netana_info *ptdeviceinfo, const char *buf, size_t tsize, void *pvuser_data)
{
        ssize_t ret     = tsize;
        int     iPolicy = 0;
        int     iPrio   = 0;
        int     iret    = 0;

        if (2 == sscanf( buf, "%d %d", &iPolicy, &iPrio)) {
                iret = netana_update_dsr_thread_prio( ptdeviceinfo, iPolicy, iPrio);
                if (iret < 0)
                        ret = iret;
        } else {
                return -EINVAL;
        }
        return ret;
}

/**
* devstate_show - returns current firmware state
* @dev_info:    Pointer to the devices info structure
*/
ssize_t devstate_show( struct netana_info *ptdeviceinfo,
                char *buf, void *pvuser_data)
{
        ssize_t  ret            = -EPERM;
        int32_t  netana_error   = NETANA_NO_ERROR;
        uint32_t ulcapturestate = 0;
        uint32_t ulcaptureerror = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NETANA_NO_ERROR != (netana_error = netana_get_state( &ptdeviceinfo->tkdevice->deviceinstance, &ulcapturestate, &ulcaptureerror)))
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "0x%X 0x%X\n", ulcapturestate, ulcaptureerror);

        return ret;
}

/**
* devstate_show - needs to be called after user notification to deliver last
* device state. The function wakes the waiting DSR to signal user recognition.
* @dev_info:    Pointer to the devices info structure
*/
ssize_t devstate_show_poll( struct netana_info *ptdeviceinfo,
                char *buf, void *pvuser_data)
{
        ssize_t  ret = -EPERM;
        int32_t  netana_error;
        uint32_t ulcapturestate;
        uint32_t ulcaptureerror;
        int      cnt;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NETANA_NO_ERROR != (netana_error = netana_get_state( &ptdeviceinfo->tkdevice->deviceinstance, &ulcapturestate, &ulcaptureerror)))
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "0x%X 0x%X\n", ulcapturestate, ulcaptureerror);

        cnt = atomic_read(&ptdeviceinfo->status_event_counter);
        if (ptdeviceinfo->status_read_counter != cnt) {
                ptdeviceinfo->status_read_counter = cnt;
                /* TODO: check if everything is read */
                ptdeviceinfo->status_read_sig = 1;
                wake_up_interruptible(&ptdeviceinfo->waitqueue_status_notification);
        }

        return ret;
}

ssize_t devfeature_show( struct netana_info *ptdeviceinfo,
               char *buf, void *pvuser_data)
{
        ssize_t                            ret = -EPERM;
        int32_t                            netana_error;
        NETANA_MNGMT_GET_DEV_FEATURE_IN_T  tFeaturesIn  = {0};
        NETANA_MNGMT_GET_DEV_FEATURE_OUT_T tFeaturesOut = {0};

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        tFeaturesIn.hDev = &ptdeviceinfo->tkdevice->deviceinstance;

        if (NETANA_NO_ERROR != (netana_error = netana_mngmt_exec_cmd(  NETANA_MNGMT_CMD_GET_DEV_FEATURE, &tFeaturesIn, sizeof(tFeaturesIn), &tFeaturesOut, sizeof(tFeaturesOut))))
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\n",
                tFeaturesOut.ulStructVersion,
                tFeaturesOut.ulPhysType,
                tFeaturesOut.ulNumPhysPorts,
                tFeaturesOut.ulPhysTapPresent,
                tFeaturesOut.ulPhysForwardingSupport,
                tFeaturesOut.ulPhysPortSpeedSupport,
                tFeaturesOut.ulPhysTransparentModeSupport,
                tFeaturesOut.ulNumGpios,
                tFeaturesOut.ulGpioInputRisingSupport,
                tFeaturesOut.ulGpioInputFallingSupport,
                tFeaturesOut.ulGpioOutputModeSupport,
                tFeaturesOut.ulGpioOutputPWMSupport,
                tFeaturesOut.ulGpioSyncInSupport,
                tFeaturesOut.ulGpioTriggerStartSupport,
                tFeaturesOut.ulGpioTriggerStopSupport,
                tFeaturesOut.ulGpioVoltage3VSupport,
                tFeaturesOut.ulGpioVoltage24VSupport,
                tFeaturesOut.ulSyncSupport);

        return ret;
}

ssize_t fwerror_store( struct netana_info *ptdeviceinfo,
                                const char *buf, size_t tsize, void *pvuser_data)
{
        ssize_t       ret           = tsize;
        int32_t       netana_error  = 0;
        uint32_t      ulDeviceError = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (1 == sscanf( buf, "%d", &ulDeviceError)) {
                  if (NETANA_NO_ERROR != (netana_error = netana_signal_driver_error( &ptdeviceinfo->tkdevice->deviceinstance, ulDeviceError)))
                        return store_err( ptdeviceinfo, netana_error);
        }
        return ret;

}

ssize_t show_device_info(struct file *ptfile, struct kobject *ptkobj, struct bin_attribute *bin_attr, char* buf, loff_t toff, size_t tcount)
{
        struct netana_sysfs_dir   *dir_entry = container_of( ptkobj, struct netana_sysfs_dir, kobj);
        NETANA_DEVICE_INFORMATION_T tDevInfo;
        int32_t                     netana_error;

        if (toff > sizeof(tDevInfo))
                return 0;

        if ((toff+tcount)> sizeof(tDevInfo))
                tcount = sizeof(tDevInfo) - toff;

        if (NETANA_NO_ERROR != (netana_error = netana_enum_device( dir_entry->deviceinfo->minor, sizeof(tDevInfo), &tDevInfo)))
                return store_err( dir_entry->deviceinfo, netana_error);

        memcpy( (void*)buf, (void*)&tDevInfo+toff, tcount);
        return tcount;
}

ssize_t netana_store_gpio_mode( struct netana_info *ptdeviceinfo,
                                const char *buf, size_t tsize, void *pvuser_data)
{
        struct attrid      *ptid = (struct attrid*)pvuser_data;
        ssize_t            ret   = tsize;
        int32_t            netana_error;
        NETANA_GPIO_MODE_T tmode;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (3 == sscanf( buf, "%d %d %d",
                                &tmode.ulMode,
                                &tmode.uData.tTrigger.ulCaptureTriggers,
                                &tmode.uData.tTrigger.ulEndDelay)) {
                if (NETANA_NO_ERROR != (netana_error = netana_set_gpio_mode( &ptdeviceinfo->tkdevice->deviceinstance, ptid->objnum, &tmode)))
                        return store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}

ssize_t netana_store_gpio_level ( struct netana_info *ptdeviceinfo,
                                const char *buf, size_t tsize, void *pvuser_data)
{
        struct attrid      *ptid = (struct attrid*)pvuser_data;
        ssize_t            ret   = tsize;
        int32_t            netana_error;
        uint32_t           ullevel;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (1 == sscanf( buf, "%d", &ullevel)) {
                if (NETANA_NO_ERROR != (netana_error = netana_set_gpio_output( &ptdeviceinfo->tkdevice->deviceinstance, ptid->objnum, ullevel)))
                        return store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}

ssize_t netana_gpio_get_reg_val( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data)
{
        struct attrid      *ptid = (struct attrid*)pvuser_data;
        ssize_t            ret = 0;
        int32_t            netana_error;
        NETANA_GPIO_MODE_T tmode;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NETANA_NO_ERROR != (netana_error = netana_get_gpio_mode( &ptdeviceinfo->tkdevice->deviceinstance, ptid->objnum, &tmode)))
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "%d %d %d\n",
                                tmode.ulMode,
                                tmode.uData.tTrigger.ulCaptureTriggers,
                                tmode.uData.tTrigger.ulEndDelay);

        return ret;
}

ssize_t netana_set_gpiovoltage( struct netana_info *ptdeviceinfo,
        const char *buf, size_t tsize, void *pvuser_data)
{
        ssize_t            ret   = tsize;
        int32_t            netana_error;
        uint32_t           ulVoltage     = 0;
        uint32_t           ulGpioSelMask = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (2 == sscanf( buf, "%d %d", &ulGpioSelMask, &ulVoltage)) {
                switch(ulVoltage)
                {
                case 0:
                case 1:
                        if (NETANA_NO_ERROR !=
                        (netana_error = netana_set_gpio_voltage( &ptdeviceinfo->tkdevice->deviceinstance, ulGpioSelMask, ulVoltage)))
                                return store_err( ptdeviceinfo, netana_error);
                break;
                default:
                        return store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
                break;
                }
        } else {
                return store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
        }
        return ret;
}

ssize_t netana_get_gpiovoltage( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data)
{
        struct attrid      *ptid = (struct attrid*)pvuser_data;
        ssize_t            ret = 0;
        int32_t            netana_error;
        uint32_t           ulGpioVoltageSel;

        if (!ptdeviceinfo->tkdevice->hdevice)
        return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NETANA_NO_ERROR !=
                (netana_error = netana_get_gpio_voltage( &ptdeviceinfo->tkdevice->deviceinstance, ptid->objnum, &ulGpioVoltageSel)))
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "%d\n", ulGpioVoltageSel);

        return ret;
}

ssize_t netana_port_set_reg_val( struct netana_info *ptdeviceinfo, const char *buf, void *user_data)
{
        return 0;
}

ssize_t netana_port_get_reg_val( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data)
{
        struct attrid       *ptid        = (struct attrid*)pvuser_data;
        ssize_t             ret          = 0;
        int32_t             netana_error = 0;
        NETANA_PORT_STATE_T tstatus;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NETANA_NO_ERROR != (netana_error = netana_get_portstat( &ptdeviceinfo->tkdevice->deviceinstance,
                                                                        ptid->objnum, sizeof(NETANA_PORT_STATE_T),
                                                                        &tstatus))) {
                ret = store_err( ptdeviceinfo, netana_error);
        } else {
                ret = sprintf( buf, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %d %llu\n",
                                tstatus.ulLinkState,                     /*!< Link state (Up/Down and Speed) */
                                tstatus.ullFramesReceivedOk,             /*!< Received ok                    */
                                tstatus.ullRXErrors,                     /*!< Phy RX Errors                  */
                                tstatus.ullAlignmentErrors,              /*!< Alignment error                */
                                tstatus.ullFrameCheckSequenceErrors,     /*!< check sequence error           */
                                tstatus.ullFrameTooLongErrors,           /*!< Frame too long                 */
                                tstatus.ullSFDErrors,                    /*!< SFD errors                     */
                                tstatus.ullShortFrames,                  /*!< Short frames                   */
                                tstatus.ullFramesRejected,               /*!< Rejected Frame                 */
                                tstatus.ullLongPreambleCnt,              /*!< Preamble to long               */
                                tstatus.ullShortPreambleCnt,             /*!< Preamble to short              */
                                tstatus.ullBytesLineBusy,                /*!<                                */
                                tstatus.ulMinIFG,                        /*!< Min IFG                        */
                                tstatus.ullTime);                         /*!< Time of last counter update    */
        }
        return ret;
}

ssize_t netana_phy_get_reg_val( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data)
{
        struct attrid *ptid        = (struct attrid*)pvuser_data;
        ssize_t       ret          = 0;
        int32_t       netana_error = 0;
        uint16_t      usValue;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NETANA_NO_ERROR != (netana_error = netana_access_phy_reg(&ptdeviceinfo->tkdevice->deviceinstance,
                                                                        NETANA_PHY_DIRECTION_READ,
                                                                        ptid->objnum,
                                                                        ptid->devnum,
                                                                        &usValue,
                                                                        get_timeout(ptdeviceinfo))))
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "%u\n", usValue);

        return ret;
}

ssize_t netana_phy_set_reg_val( struct netana_info *ptdeviceinfo,
                                const char *buf, size_t size, void *pvuser_data)
{
        struct attrid *ptid        = (struct attrid*)pvuser_data;
        uint32_t      netana_error = 0;
        ssize_t       ret          = size;
        unsigned int  tmp;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (1 == sscanf( buf, "%u", &tmp)) {
                uint16_t usvalue = 0xFFFF & tmp;
                if (NETANA_NO_ERROR != (netana_error = netana_access_phy_reg(&ptdeviceinfo->tkdevice->deviceinstance,
                                                                                NETANA_PHY_DIRECTION_WRITE,
                                                                                ptid->objnum,
                                                                                ptid->devnum,
                                                                                &usvalue,
                                                                                get_timeout(ptdeviceinfo))) ) {
                        ret = store_err( ptdeviceinfo, netana_error);
                }
        }
        return ret;
}

ssize_t get_relation( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data)
{
        struct attrid   *ptid = (struct attrid*)pvuser_data;
        ssize_t         ret;
        int32_t         netana_error   = 0;
        uint32_t        ulrelationship = 0;
        uint32_t        ulfiltersize   = 0;
        NETANA_FILTER_T tfiltera       = {0};
        NETANA_FILTER_T tfilterb       = {0};

        tfiltera.pbMask       = ptdeviceinfo->tkdevice->maska;
        tfiltera.pbValue      = ptdeviceinfo->tkdevice->vala;
        tfiltera.ulFilterSize = 0;
        tfilterb.pbMask       = ptdeviceinfo->tkdevice->maskb;
        tfilterb.pbValue      = ptdeviceinfo->tkdevice->valb;
        tfilterb.ulFilterSize = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        ulfiltersize = ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize;

        if (NETANA_NO_ERROR != (netana_error = netana_get_filter( &ptdeviceinfo->tkdevice->deviceinstance,
                                                                        ptid->objnum,
                                                                        &tfiltera,
                                                                        &tfilterb,
                                                                        &ulrelationship)) )
                ret = store_err( ptdeviceinfo, netana_error);
        else
                ret = sprintf( buf, "%d %d %d\n", ulrelationship, ulfiltersize, ulfiltersize);

        return ret;
}

/**
* set_filter - activates filter on the device.Filter information need to be
* previously set by calling store_filter()
* @dev_info:    Pointer to the devices info structure
*/
ssize_t set_filter( struct netana_info *ptdeviceinfo,
                const char *buf, size_t size, void *pvuser_data)
{
        struct attrid   *ptid = (struct attrid*)pvuser_data;
        ssize_t         ret = size;
        int32_t         netana_error = 0;
        NETANA_FILTER_T tfiltera;
        NETANA_FILTER_T tfilterb;
        uint32_t        ulrelationship = 0;

        tfiltera.pbMask       = ptdeviceinfo->tkdevice->maska;
        tfiltera.pbValue      = ptdeviceinfo->tkdevice->vala;
        tfiltera.ulFilterSize = ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize;
        tfilterb.pbMask       = ptdeviceinfo->tkdevice->maskb;
        tfilterb.pbValue      = ptdeviceinfo->tkdevice->valb;
        tfilterb.ulFilterSize = ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (3 == sscanf( buf, "%d %d %d", &ulrelationship, &tfiltera.ulFilterSize, &tfilterb.ulFilterSize)) {
                if (NETANA_NO_ERROR != (netana_error = netana_set_filter( &ptdeviceinfo->tkdevice->deviceinstance,
                                                                                ptid->objnum,
                                                                                &tfiltera,
                                                                                &tfilterb,
                                                                                ulrelationship)))
                        ret = store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}

ssize_t show_filter(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count)
{
        int32_t                   netana_error    = 0;
        struct netana_sysfs_dir  *kobjentry      = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_info        *ptdeviceinfo   = kobjentry->deviceinfo;
        ssize_t                   ret             = count;
        uint32_t                  ulrelationship;
        NETANA_FILTER_T           *ptfiltera      = NULL;
        NETANA_FILTER_T           *ptfilterb      = NULL;
        NETANA_FILTER_T           tfilter;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        tfilter.pbMask       = kzalloc( ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize, GFP_KERNEL);
        tfilter.pbValue      = kzalloc( ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize, GFP_KERNEL);
        tfilter.ulFilterSize = ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize;

        if ((0 == strncmp( bin_attr->attr.name, "maska", strlen("maska"))) ||
                (0 == strncmp( bin_attr->attr.name, "vala", strlen("vala")))) {
                ptfiltera = &tfilter;
        } else if ((0 == strncmp( bin_attr->attr.name, "maskb", strlen("maskb"))) ||
                (0 == strncmp( bin_attr->attr.name, "valb", strlen("valb")))) {
                ptfilterb = &tfilter;
        } else {
                return -EIO;
        }

        if (NETANA_NO_ERROR != (netana_error = netana_get_filter( &ptdeviceinfo->tkdevice->deviceinstance,
                                                                        kobjentry->objnum,
                                                                        ptfiltera,
                                                                        ptfilterb,
                                                                        &ulrelationship)) ) {
                ret = store_err( ptdeviceinfo, netana_error);
        } else {
                if (0 == strncmp( bin_attr->attr.name, "mask", strlen("mask")))
                        memcpy ( buf, tfilter.pbMask, count);
                else
                        memcpy ( buf, tfilter.pbValue, count);
        }
        kfree(tfilter.pbMask);
        kfree(tfilter.pbValue);

        return ret;
}

ssize_t store_filter(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count)
{
        struct netana_sysfs_dir *dir_entry    = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_info        *ptdeviceinfo = dir_entry->deviceinfo;
        ssize_t                   ret           = count;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if ((count == 0) || (count>ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize))
                return store_err( ptdeviceinfo, NETANA_INVALID_BUFFERSIZE);

        if (0 == strncmp( bin_attr->attr.name, "maska", strlen("maska"))) {
                memset( ptdeviceinfo->tkdevice->maska, 0, ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize);
                memcpy ( ptdeviceinfo->tkdevice->maska, buf, count);
        } else if ( 0 == strncmp( bin_attr->attr.name, "vala", strlen("vala"))) {
                memset( ptdeviceinfo->tkdevice->vala, 0, ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize);
                memcpy ( ptdeviceinfo->tkdevice->vala, buf, count);
        } else if ( 0 == strncmp( bin_attr->attr.name, "maskb", strlen("maskb"))) {
                memset( ptdeviceinfo->tkdevice->maskb, 0, ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize);
                memcpy ( ptdeviceinfo->tkdevice->maskb, buf, count);
        } else if ( 0 == strncmp( bin_attr->attr.name, "valb", strlen("valb"))) {
                memset( ptdeviceinfo->tkdevice->valb, 0, ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize);
                memcpy ( ptdeviceinfo->tkdevice->valb, buf, count);
        } else {
                ret = store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
        }
        return ret;
}

/**
* netana_config_control - enables/disables capture process and/or status
* notification.
* @dev_info:    Pointer to the devices info structure
*/
ssize_t netana_config_control( struct netana_info *ptdeviceinfo,
                        const char *buf, size_t size, void *pvuser_data)
{
        int32_t  netana_error = 0;
        ssize_t  ret          = size;
        uint32_t ulcapturemode;
        uint32_t ulportmask;
        uint32_t ulmacmode;
        uint64_t ullreftime;
        int      fdatacallback;
        int      fstatuscallback;
        uint32_t ulcapturedisable;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (6 == sscanf( buf, "%d %d %d %llu %i %i", &ulcapturemode, &ulportmask, &ulmacmode, &ullreftime, &fdatacallback, &fstatuscallback)) {
                PFN_STATUS_CALLBACK pfnStatus = NULL;
                if (fstatuscallback)
                        pfnStatus = netana_notify_state;

                if (NETANA_NO_ERROR != (netana_error = netana_start_capture( &ptdeviceinfo->tkdevice->deviceinstance,
                                                                                ulcapturemode,
                                                                                ulportmask,
                                                                                ulmacmode,
                                                                                ullreftime,
                                                                                pfnStatus,
                                                                                netana_notify_data,
                                                                                ptdeviceinfo)) ) {
                        ssize_t tmp_ret;
                        if ((tmp_ret = store_err( ptdeviceinfo, netana_error)))
                                ret = tmp_ret;
                } else
                {
                        ptdeviceinfo->captureactive = 1;
                        if (fstatuscallback)
                                ptdeviceinfo->statuscallback = 1;
                }
        } else if (1 == sscanf( buf, "%d", &ulcapturedisable)) {
                if (1 == ulcapturedisable)
                {
                        //ptdeviceinfo->netAnaStatusSig   = 1;
                        if (NETANA_NO_ERROR != (netana_error = netana_stop_capture( &ptdeviceinfo->tkdevice->deviceinstance)) ) {
                                ssize_t tmp_ret;
                                if ((tmp_ret = store_err( ptdeviceinfo, netana_error)))
                                        ret = tmp_ret;
                        } else {
                                ptdeviceinfo->captureactive  = 0;
                                ptdeviceinfo->statuscallback = 0;
                        }
                }
        }
        return ret;
}

ssize_t netana_blink_device( struct netana_info *ptdeviceinfo,
                        const char *buf, size_t size, void *pvuser_data)
{
        int     blink        = 0;
        size_t  ret          = size;
        int32_t netana_error = 0;

        if (sscanf( buf, "%i", &blink)){
                if (blink) {
                        if (NETANA_NO_ERROR != (netana_error = netana_mngmt_exec_cmd( NETANA_MNGMT_CMD_BLINK,
                                                ptdeviceinfo->tkdevice->deviceinstance.szDeviceName,
                                                strlen(ptdeviceinfo->tkdevice->deviceinstance.szDeviceName),
                                                NULL,
                                                0)))
                                ret = store_err( ptdeviceinfo, netana_error);
                }
        }
        return ret;
}

ssize_t netana_pi_config( struct netana_info *ptdeviceinfo,
                        const char *buf, size_t size, void *pvuser_data)
{
        uint32_t ulp          = 0;
        uint32_t uli          = 0;
        int32_t  netana_error = 0;
        size_t   ret          = size;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (2 == sscanf( buf, "%d %d", &ulp, &uli)){
                        if (NETANA_NO_ERROR != (netana_error = netana_config_pi_controller( &ptdeviceinfo->tkdevice->deviceinstance, ulp, uli))){
                                ssize_t tmp_ret;

                                if ((tmp_ret = store_err( ptdeviceinfo, netana_error)))
                                        ret = tmp_ret;
                        }
        } else{
                ret = store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
        }
        return ret;
}

ssize_t netana_time_config( struct netana_info *ptdeviceinfo,
                        const char *buf, size_t size, void *pvuser_data)
{
        int32_t  netana_error     = 0;
        size_t   ret              = size;
        uint64_t ullreferencetime = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (1 == sscanf( buf, "%llu", &ullreferencetime)){
                        if (NETANA_NO_ERROR != (netana_error = netana_resync_time( &ptdeviceinfo->tkdevice->deviceinstance, ullreferencetime))){
                                ssize_t tmp_ret;

                                if ((tmp_ret = store_err( ptdeviceinfo, netana_error)))
                                        ret = tmp_ret;
                        }
        } else{
                ret = store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
        }
        return ret;
}

ssize_t netana_timeout_config( struct netana_info *ptdeviceinfo,
                        const char *buf, size_t size, void *pvuser_data)
{
        size_t   ret       = size;
        uint32_t ultimeout = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (1 == sscanf( buf, "%d", &ultimeout)){
                store_timeout( ptdeviceinfo, ultimeout);
        } else{
                ret = store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
        }
        return ret;
}

ssize_t netana_show_ident_info( struct netana_info *ptdeviceinfo,
                char *buf, void *pvuser_data)
{
        ssize_t            ret          = 0;
        NETANA_BASE_DPM_T* ptDpm = NULL;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        ptDpm = (NETANA_BASE_DPM_T*)ptdeviceinfo->tkdevice->deviceinstance.pvDPM;

        ret = sprintf( buf, "%lu %lu %lu %u 0x%X 0x%X %u %u\n",
                0UL,
                (long unsigned int)ptDpm->tSystemInfoBlock.ulDeviceNr,
                (long unsigned int)ptDpm->tSystemInfoBlock.ulSerialNr,
                ptDpm->tSystemInfoBlock.usManufacturer,
                ptDpm->tSystemInfoBlock.ulLicenseFlags1,
                ptDpm->tSystemInfoBlock.ulLicenseFlags2,
                ptDpm->tSystemInfoBlock.usNetXLicenseID,
                ptDpm->tSystemInfoBlock.usNetXLicenseFlags);
        return ret;
}

ssize_t netana_ident_update( struct netana_info *ptdeviceinfo,
                        const char *buf, size_t size, void *pvuser_data)
{
        size_t   ret       = size;
        char*    ident_info_file;
        int32_t  netana_error = 0;
        struct NETANA_DEVINSTANCE_T *dev = &ptdeviceinfo->tkdevice->deviceinstance;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if (NULL == (ident_info_file = kmalloc(size, GFP_KERNEL)))
                return store_err( ptdeviceinfo, NETANA_OUT_OF_MEMORY);

        if (1 == sscanf( buf, "%s", ident_info_file)){
                if (NETANA_NO_ERROR == (netana_error = netana_set_device_ident( dev,
                        ident_info_file)))
                {
                        if ((netana_error = netana_restart_device( dev)))
                        {
                                ptdeviceinfo->tkdevice->addsucceded = 0;
                                ptdeviceinfo->tkdevice->hdevice     = NULL;
                                ret = store_err( ptdeviceinfo, NETANA_HWRESET_ERROR);
                        } else {
                                if (dev->tIdentInfoFile.lError) {
                                        ret = store_err( ptdeviceinfo,
                                                        dev->tIdentInfoFile.lError);
                                }
                        }
                } else {
                        ret = store_err( ptdeviceinfo, netana_error);
                }
        } else{
                ret = store_err( ptdeviceinfo, NETANA_INVALID_PARAMETER);
        }
        kfree(ident_info_file);
        return ret;
}

ssize_t netana_write_ident(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count)
{
        struct netana_sysfs_dir *dir_entry    = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_info        *ptdeviceinfo = dir_entry->deviceinfo;
        ssize_t                   ret           = count;
        int32_t                   netana_error  = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);
        //TODO:check size
        memcpy ( ptdeviceinfo->identinfoblock, buf, count);
        if ((netana_error = netana_check_device_ident( &ptdeviceinfo->tkdevice->deviceinstance,
                (uint32_t)count,
                ptdeviceinfo->identinfoblock))) {
                ret = store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}

ssize_t netana_read_ident(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count)
{
        struct netana_sysfs_dir *dir_entry    = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_info        *ptdeviceinfo = dir_entry->deviceinfo;
        ssize_t                   ret           = count;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        memcpy( buf, ptdeviceinfo->identinfoblock+off, count);
        memset( ptdeviceinfo->identinfoblock, 0, count);

        return ret;
}

ssize_t netana_read_mbx_status( struct netana_info *ptdeviceinfo,
                                char *buf, void *pvuser_data)
{
        ssize_t  ret          = 0;
        uint32_t recv_cnt     = 16;
        uint32_t send_cnt     = 0;
        int32_t  netana_error = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if ((netana_error = netana_get_mbx_state( &ptdeviceinfo->tkdevice->deviceinstance, &recv_cnt, &send_cnt))) {
                ret = store_err( ptdeviceinfo, netana_error);
        } else {
                ret = sprintf( buf, "%u %u\n", recv_cnt, send_cnt);
        }
        return ret;
}

ssize_t netana_read_mbx_packet_bin(struct file *filp, struct kobject *kobj,
                                   struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count)
{
        struct netana_sysfs_dir *dir_entry    = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_info        *ptdeviceinfo = dir_entry->deviceinfo;
        ssize_t                   ret           = count;
        int32_t                   netana_error  = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        /* in case of success do not return packet size (netana_get_packet() checked the size), just return the number of requested bytes to satisfy read call */
        if ((netana_error = netana_get_packet( &ptdeviceinfo->tkdevice->deviceinstance, count, (NETANA_PACKET*)buf, get_timeout(ptdeviceinfo)))) {
                ret = store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}

ssize_t netana_write_mbx_packet_bin(struct file *filp, struct kobject *kobj,
                                    struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count)
{
        struct netana_sysfs_dir *dir_entry    = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_info        *ptdeviceinfo = dir_entry->deviceinfo;
        ssize_t                   ret           = count;
        int32_t                   netana_error  = 0;

        if (!ptdeviceinfo->tkdevice->hdevice)
                return store_err( ptdeviceinfo, NETANA_INVALID_HANDLE);

        if ((netana_error = netana_put_packet( &ptdeviceinfo->tkdevice->deviceinstance, (NETANA_PACKET*)buf, get_timeout(ptdeviceinfo)))) {
                ret = store_err( ptdeviceinfo, netana_error);
        }
        return ret;
}
