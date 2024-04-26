#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/idr.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/kthread.h>
#include <linux/poll.h>
#include <linux/version.h>

#include <linux/sched.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <uapi/linux/sched/types.h>
#endif

#include "netana.h"
#include "netana_sysfs.h"

/* module parameter */
unsigned long ulDMABufferCount   = 0;
unsigned long ulDMABufferSize    = 0;
unsigned long ulDMABufferTimeout = 0;
unsigned long ulTraceLevel       = 0;

module_param(ulDMABufferCount, ulong, 0);
MODULE_PARM_DESC(ulDMABufferCount, "Number of DMA-buffers to use.");
module_param(ulDMABufferSize, ulong, 0);
MODULE_PARM_DESC(ulDMABufferSize, "Size of the DMA-buffers.");
module_param(ulDMABufferTimeout, ulong, 0);
MODULE_PARM_DESC(ulDMABufferTimeout, "Timeout of DMA-buffer indication.");
module_param(ulTraceLevel, ulong, 0);
MODULE_PARM_DESC(ulTraceLevel, "Trace-Level of debug messages (0-255).");

/* global variables */
extern uint32_t     g_ulTraceLevel;
static              DEFINE_IDR(netAnaIDR);
static int          major = 0;

DEFINE_MUTEX(minor_mutex);

/* netanalyzer class definition */
static struct netana_class {
        struct kref               kref;
        struct class              *class;
        struct netana_driver_info drv_info;
} *netana_class = NULL;

/* function declaration */
static int          netana_open    (struct inode *, struct file *);
static int          netana_release (struct inode *, struct file *);
static ssize_t      netana_read    (struct file *, char *, size_t, loff_t *);
static unsigned int netana_poll    (struct file *, struct poll_table_struct *);

static struct file_operations fops = {
        .read    = netana_read,
        .open    = netana_open,
        .release = netana_release,
        .poll    = netana_poll,
};

/**
* netana_poll - Called when poll() is called
* returns POLLPRI in case of available data
* returns POLLRDNORM in case of state change
*/
static unsigned int netana_poll(struct file *ptfile, struct poll_table_struct *ptpolltable)
{
        struct netana_info *ptdeviceinfo = (struct netana_info*)ptfile->private_data;
        unsigned int       ret           = 0;

        poll_wait(ptfile, &ptdeviceinfo->waitqueue_usr_notification, ptpolltable);

        if (ptdeviceinfo->data_read_counter != atomic_read(&ptdeviceinfo->data_event_counter))
                ret = POLLPRI | POLLIN;
        if (ptdeviceinfo->status_read_counter != atomic_read(&ptdeviceinfo->status_event_counter))
                ret |= POLLRDNORM | POLLIN;

        return ret;
}

/**
* netana_notify_data - Called in case of new data
* called during capturing process (see tk_interface.c -> netana_config_control())
* @pvData    : pointer to the availalbe data
* @ulDataLen : length of available data area
* @pvUser    : pointer to the devices info structure
*/
void netana_notify_data(void* pvdmabuffer, uint32_t uldatalen, void* pvuser)
{
        struct netana_info *ptdeviceinfo = (struct netana_info*)pvuser;

        if (0 == ptdeviceinfo->captureactive)
                return;

        /* check if user application is waiting for the data */
        if (ptdeviceinfo->notify_interrupts == 1) {
                /* validate buffer */
                ptdeviceinfo->cur_dma_buf.data        = pvdmabuffer;
                ptdeviceinfo->cur_dma_buf.datalen     = uldatalen;
                ptdeviceinfo->cur_dma_buf.buffervalid = 1;

                atomic_inc(&ptdeviceinfo->data_event_counter);
                wake_up_interruptible(&ptdeviceinfo->waitqueue_usr_notification);

                /* NOTE: interrupted waits will lead to lost data */
                wait_event_interruptible(ptdeviceinfo->waitqueue_data_notification,
                                ptdeviceinfo->data_read_sig);
                ptdeviceinfo->data_read_sig = 0;
        }
}

/**
* netana_notify_state - Called in case of a state change
* called during capturing process (see tk_interface.c -> netana_config_control())
* @ulCaptureState : not needed here
* @ulCaptureError : not needed here
* @pvUser         : pointer to the devices info structure
*/
void netana_notify_state(uint32_t ulcapturestate, uint32_t ulcaputererror,
                        void* pvuser)
{
        struct netana_info *ptdeviceinfo = (struct netana_info*)pvuser;

        /* check if user application is waiting for the data */
        if (ptdeviceinfo->notify_interrupts == 1) {
                atomic_inc(&ptdeviceinfo->status_event_counter);
                wake_up_interruptible(&ptdeviceinfo->waitqueue_usr_notification);
                //NOTE: interrupted waits will lead to lost data
                wait_event_interruptible(ptdeviceinfo->waitqueue_status_notification,
                                        ptdeviceinfo->status_read_sig);
                ptdeviceinfo->status_read_sig = 0;
        }
}

/**
* netana_update_dsr_thread_prio - Changes priority of dsr during runtime
* @param ptdeviceinfo: pointer to the devices info structure
* @param iPolicy: policy to be set
* @param iPrio:   priority to be set
*/
int netana_update_dsr_thread_prio( struct netana_info* ptdeviceinfo, int iPolicy, int iPrio)
{
        struct sched_param  tSchedParam = {0};
        int                 iret        = 0;

        tSchedParam.sched_priority = iPrio;

        if ((NULL != ptdeviceinfo) && (NULL != ptdeviceinfo->dsrthread)) {
                if ((iret = sched_setscheduler( ptdeviceinfo->dsrthread, iPolicy, &tSchedParam))) {
                        printk( KERN_INFO "DSR priority settings invalid!\n");
                        printk( KERN_INFO "Changing DSR thread priority returned %d\n", iret);
                }
        } else {
                printk( KERN_INFO "Error no DSR thread running!\n");
                iret = -EINVAL;
        }
        return iret;
}

/**
* netana_dsrhandler - Processes long time consuming jobs
* @param : pointer to the devices info structure
*/
int netana_dsrhandler(void* pvparam)
{
        int32_t             ret          = 0;
        struct netana_info* ptdeviceinfo = (struct netana_info*)pvparam;

        while(1) {
                //NOTE: interrupted waits will lead to lost data
                wait_event_interruptible(ptdeviceinfo->waitqueue_dsr,
                                        ptdeviceinfo->dsr_sig);
                ptdeviceinfo->dsr_sig = 0;

                if(kthread_should_stop())
                        break;

                ret = netana_tkit_dsr_handler(&ptdeviceinfo->tkdevice->deviceinstance);

                if (NETANA_TKIT_DSR_PROCESSING_REQUEST == ret) {
                        netana_tkit_process( &ptdeviceinfo->tkdevice->deviceinstance);
                }
                netana_tkit_enable_hwinterrupts( &ptdeviceinfo->tkdevice->deviceinstance);
        }
        return 0;
}

void stop_dsr_thread(struct netana_info *ptdeviceinfo)
{
        if (ptdeviceinfo->dsrthread) {
                ptdeviceinfo->dsr_sig = 1;
                kthread_stop(ptdeviceinfo->dsrthread);
                ptdeviceinfo->dsrthread = NULL;
        }
}

/**
* netana_isr - netAnalyzer interrupt handler
* @irq     : IRQ number
* @dev_info: Pointer to the devices info structure
*/
irqreturn_t netana_isr(int irq, void *pvparam)
{
        struct netana_info *ptdeviceinfo = (struct netana_info*)pvparam;
        irqreturn_t        ret           = IRQ_HANDLED;
        int32_t            isr_ret;

        /* check if it was our device and confirm the interrupt */
        isr_ret = netana_tkit_isr_handler(&ptdeviceinfo->tkdevice->deviceinstance, 1);
        /* if it was our device an the more work is requested call DSR handler */
        switch(isr_ret) {
                case NETANA_TKIT_ISR_IRQ_DSR_REQUESTED:
                        netana_tkit_disable_hwinterrupts( &ptdeviceinfo->tkdevice->deviceinstance);
                        ptdeviceinfo->dsr_sig = 1;
                        wake_up_interruptible(&ptdeviceinfo->waitqueue_dsr);
                        break;

                case NETANA_TKIT_ISR_IRQ_HANDLED:
                        /* Everything was done by ISR, no need to call DSR */
                        break;

                case NETANA_TKIT_ISR_IRQ_OTHERDEVICE:
                default:
                        /* this is not our device */
                        ret = IRQ_NONE;
                        break;
        }
        return ret;
}

/**
* netana_getminor - Generates minor number
* @dev_info: Pointer to the devices info structure
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
static int netana_getminor(struct netana_info *ptdeviceinfo)
{
        int ret = 0;
        int iid;

        mutex_lock(&minor_mutex);
        if (idr_pre_get(&netAnaIDR, GFP_KERNEL) == 0)
                goto exit;

        if (0 > (ret = idr_get_new(&netAnaIDR, ptdeviceinfo, &iid))) {
                if (ret == -EAGAIN)
                        ret = -ENOMEM;

                goto exit;
        }
        ptdeviceinfo->minor = iid;

exit:
        mutex_unlock(&minor_mutex);
        return ret;
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)
static int netana_getminor(struct netana_info *ptdeviceinfo)
{
        int ret;

        mutex_lock(&minor_mutex);
        ret = idr_alloc(&netAnaIDR, ptdeviceinfo, 0, 0, GFP_KERNEL);

        if (ret < 0)
                goto exit;

        ptdeviceinfo->minor = ret;
        ret = 0;

exit:
        mutex_unlock(&minor_mutex);
        return ret;
}
#endif


/**
* netana_getminor - Frees minor number
* @ptdeviceinfo: Pointer to the devices info structure
*/
static void netana_freeminor(struct netana_info *ptdeviceinfo)
{
        idr_remove(&netAnaIDR, ptdeviceinfo->minor);
}

/**
* init_netana_class - Initilaizes netanalyzer class
*/
static int init_netana_class(void)
{
        int ret = SUCCESS;

        if (netana_class != 0) {
                kref_get(&netana_class->kref);
                goto exit;
        }
        mutex_init(&minor_mutex);

        if (0>(major = register_chrdev(0, DEVICE_NAME, &fops))) {
                printk( KERN_ERR "Failed to register %s\n", DEVICE_NAME);
                return major;
        }
        if (!(netana_class = kzalloc(sizeof(struct netana_class), GFP_KERNEL))) {
                ret = -ENOMEM;
                goto err_kzalloc;
        }
        kref_init(&netana_class->kref);
        netana_class->class = class_create(THIS_MODULE, DEVICE_NAME);
        if ((ret = IS_ERR(netana_class->class))) {
                printk(KERN_ERR "Could not create class for %s", DEVICE_NAME);
                goto err_create_class;
        }
        if (NETANA_NO_ERROR == netana_tkit_init()) {
                memset((void*)&netana_class->drv_info, 0, sizeof(netana_class->drv_info));
                return SUCCESS;
        }
        ret = -ENOMEM;
        class_destroy(netana_class->class);
err_create_class:
        kfree(netana_class);
        netana_class = NULL;
err_kzalloc:
        unregister_chrdev( major, DEVICE_NAME);
exit:
        return ret;
}

/**
* netana_release_class - De-initializes netanalyzer class
* ptkref: Pointer to the ref counter
*/
static void netana_release_class(struct kref* ptkref)
{
#ifdef NETANA_PM_AWARE
        if (netana_class->drv_info.fw_copy.data)
                kfree(netana_class->drv_info.fw_copy.data);

        if (netana_class->drv_info.bl_copy.data)
                kfree(netana_class->drv_info.bl_copy.data);
#endif

        class_destroy(netana_class->class);
        kfree(netana_class);
        unregister_chrdev(major, DEVICE_NAME);
        netana_class = NULL;
}

static void netana_classdestroy(void)
{
        if (netana_class)
                kref_put(&netana_class->kref, netana_release_class);
}

/**
* netana_create_sysfsentries - Generates the required sysfs-entries
* |->+netanalyzer0
* |  |->+driver_information = driver specific information
* |  |->+device_information = device specific information
* |  |->+hw_resources       = device hardware resource information
* |  |->+device_config      = device global configuration
* |  |->+device_control     = device control
* |  |->+fw                 = firmware related information
* ...
* @ptdeviceinfo: Pointer to the devices info structure
*/
static int netana_create_sysfsentries(struct netana_info *ptdeviceinfo)
{
        /* initialize task information, required for some sysfs attributes */
        init_task_list( ptdeviceinfo);

        /* global driver information */
        if (ptdeviceinfo->kobj_drv_info) {
                if (nasysfs_create_drv_info(ptdeviceinfo))
                        goto err_sysfs;
        } else {
                goto err_sysfs;
        }
        /* device information */
        if ((ptdeviceinfo->kobj_dev_info_dir = kobject_create_and_add("device_information", &ptdeviceinfo->device->kobj))) {
                if (nasysfs_create_dev_state_info(ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_dev_info(ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_ident_info( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_dev_features( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_fw_error( ptdeviceinfo)) {
                        goto err_sysfs;
                }
        } else {
                goto err_sysfs;
        }
        /* create hardware resources */
        if ((ptdeviceinfo->kobj_hw_dir = kobject_create_and_add("hw_resources", &ptdeviceinfo->device->kobj))) {
                if (nasysfs_create_filter_dir(ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_gpio_dir( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_port_dir( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_phy_dir( ptdeviceinfo)) {
                        goto err_sysfs;
                }
        } else {
                goto err_sysfs;
        }
        /* create device config */
        if ((ptdeviceinfo->kobj_conf_dir = kobject_create_and_add("device_config", &ptdeviceinfo->device->kobj))) {
                if (nasysfs_create_timeout_cfg( ptdeviceinfo)) {
                        goto err_sysfs;
                }
        } else {
                goto err_sysfs;
        }
        /* create device control */
        if ((ptdeviceinfo->kobj_ctl_dir = kobject_create_and_add("device_control", &ptdeviceinfo->device->kobj))) {
                if (nasysfs_create_capture_ctl( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_exec_cmd( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_pi_cfg( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_time_cfg( ptdeviceinfo)) {
                        goto err_sysfs;
                } else if (nasysfs_create_mbx_dir( ptdeviceinfo)) {
                        goto err_sysfs;
                }
        } else {
                goto err_sysfs;
        }
        return 0;

err_sysfs:
        clean_up_sysfs( ptdeviceinfo);

        /* delete basic kernel objects */
        kobject_put(ptdeviceinfo->kobj_ctl_dir);
        kobject_put(ptdeviceinfo->kobj_conf_dir);
        kobject_put(ptdeviceinfo->kobj_hw_dir);
        kobject_put(ptdeviceinfo->kobj_dev_info_dir);

        printk(KERN_ERR "Error while creating sysfs files!\n");
        return -ENOMEM;
}

extern struct kobj_type dma_map_attr_type;
/**
* netana_create_dma - Creates dma buffer for the netAnalyzer device
* size and count of the DMA buffers depend on the module arguments
* (or default)
* @dev_info: Pointer to the devices info structure
*/
static int netana_create_dma(struct netana_info *ptdeviceinfo)
{
        int32_t           lidx;
        uint32_t          ret           = -ENOMEM;
        struct netana_mem *dma_mem_info = NULL;

        if (!(ptdeviceinfo->dmainfo = kzalloc(sizeof(struct netana_mem*)*(g_ulDMABufferCount+1), GFP_KERNEL)))
                return -ENOMEM;

        if (!(ptdeviceinfo->kobj_drv_info = kobject_create_and_add("driver_information", &ptdeviceinfo->device->kobj)))
                goto err_kobj;

        if (!(ptdeviceinfo->kobj_dma_dir = kobject_create_and_add("dma", ptdeviceinfo->kobj_drv_info))) {
                printk(KERN_ERR "Error creating dma object\n");
                goto err_kobj;
        }
        for(lidx = 0; lidx < g_ulDMABufferCount; ++lidx) {
                void*               pDMA     = NULL;
                NETANA_DMABUFFER_T* ptBuffer = &ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[lidx];
                dma_addr_t          dma_handle;

                /* allocate dma buffer */
                if (!(pDMA = dma_alloc_coherent((ptdeviceinfo->pcidevice) ? &ptdeviceinfo->pcidevice->dev : NULL, g_ulDMABufferSize, &dma_handle, GFP_KERNEL))) {
                        printk(KERN_ERR "DMA allocation failed\n");
                        goto free_dma;
                }
                memset( pDMA, 0, g_ulDMABufferSize);

                /* setup dma information */
                ptBuffer->hBuffer           = pDMA;
                ptBuffer->ulBufferSize      = g_ulDMABufferSize;
                ptBuffer->ulPhysicalAddress = 0xFFFFFFFF & dma_handle;
                ptBuffer->pvBufferAddress   = pDMA;
        }
        /* setup dma info in sysfs */
        for(lidx = 0; lidx < g_ulDMABufferCount; ++lidx) {
                if (!(dma_mem_info = kzalloc(sizeof(struct netana_mem), GFP_KERNEL)))
                        goto err_dma_mem_info;

                dma_mem_info->addr = (unsigned long)ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[lidx].pvBufferAddress;
                dma_mem_info->size = ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[lidx].ulBufferSize;

                kobject_init(&dma_mem_info->kobj, &dma_map_attr_type);
                if (0>(ret = kobject_add(&dma_mem_info->kobj, ptdeviceinfo->kobj_dma_dir, "dma%d", lidx))) {
                        kfree(dma_mem_info);
                        goto err_dma_mem_info;
                }
                ptdeviceinfo->dmainfo[lidx]   = dma_mem_info;
                ptdeviceinfo->dmainfo[lidx+1] = NULL;
        }
        ptdeviceinfo->tkdevice->deviceinstance.ulDMABufferCount = g_ulDMABufferCount;
        return SUCCESS;

err_dma_mem_info:
        while((lidx--)>0) {
                if (ptdeviceinfo->dmainfo[lidx]) {
                        kobject_put(&ptdeviceinfo->dmainfo[lidx]->kobj);
                        kfree(ptdeviceinfo->dmainfo[lidx]);
                }
        }
        lidx = g_ulDMABufferCount;
free_dma:
        while((lidx--)>0)
                dma_free_coherent((ptdeviceinfo->pcidevice) ? &ptdeviceinfo->pcidevice->dev : NULL,
                                g_ulDMABufferSize,
                                ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[lidx].hBuffer,
                                ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[lidx].ulPhysicalAddress);

        kobject_put(ptdeviceinfo->kobj_dma_dir);
err_kobj:
        kfree(ptdeviceinfo->dmainfo);
        return ret;
}

/**
* netana_free_dma - Frees the allocted dma regions
* @ptdeviceinfo: Pointer to the devices info structure
*/
static void netana_free_dma(struct netana_info *ptdeviceinfo)
{
        uint32_t ulidx;

        if (!ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers)
                return;

        for(ulidx = 0; ulidx < g_ulDMABufferCount; ++ulidx) {
                if (ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[ulidx].hBuffer) {
                        dma_free_coherent((ptdeviceinfo->pcidevice) ? &ptdeviceinfo->pcidevice->dev : NULL,
                                        g_ulDMABufferSize,
                                        ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[ulidx].hBuffer,
                                        ptdeviceinfo->tkdevice->deviceinstance.atDMABuffers[ulidx].ulPhysicalAddress);
                }
                if (ptdeviceinfo->dmainfo) {
                        if (ptdeviceinfo->dmainfo[ulidx]) //{
                                kobject_put(&ptdeviceinfo->dmainfo[ulidx]->kobj);
                }
        }
        if (ptdeviceinfo->dmainfo)
                kfree(ptdeviceinfo->dmainfo);

        kobject_put(ptdeviceinfo->kobj_dma_dir);
        kobject_put(ptdeviceinfo->kobj_drv_info);
}

/**
* netana_read - Serves the DMA-buffer content if available
* When the dma-buffer is read, signal DSR to wake up and free buffer for reuse.
*/
static ssize_t netana_read(struct file* ptfile, char* pbbuf, size_t tsize, loff_t* ptoffset)
{
        struct  netana_info *ptdeviceinfo = (struct netana_info*)ptfile->private_data;
        ssize_t             tbytes_read   = 0;
        int                 data_event    = atomic_read(&ptdeviceinfo->data_event_counter);

        /* TODO: check if everything is read in the prior call*/
        if (ptdeviceinfo->data_read_counter == data_event)
                return -ENODATA;

        ptdeviceinfo->data_read_counter = data_event;

        if (ptdeviceinfo->cur_dma_buf.data == NULL ||
                ptdeviceinfo->cur_dma_buf.buffervalid == 0)
                return -EAGAIN;

        if (tsize>ptdeviceinfo->cur_dma_buf.datalen)
                tsize = ptdeviceinfo->cur_dma_buf.datalen;

        tbytes_read = copy_to_user(pbbuf, ptdeviceinfo->cur_dma_buf.data, tsize);
        ptdeviceinfo->cur_dma_buf.data = NULL;

        /* TODO: check if everything is read */
        ptdeviceinfo->data_read_sig = 1;
        wake_up_interruptible(&ptdeviceinfo->waitqueue_data_notification);

        return tsize-tbytes_read;
}

/**
* netana_reset_event_counter - Resets the device event counter. Called when device
* is opended.
* When the dma-buffer is read, signal DSR to wake up and free buffer for reuse.
* @ptdeviceinfo: Pointer to the devices info structure
*/
void netana_reset_event_counter(struct netana_info *ptdeviceinfo)
{
        ptdeviceinfo->data_read_sig       = 0;
        ptdeviceinfo->data_read_counter   = 0;
        ptdeviceinfo->status_read_sig     = 0;
        ptdeviceinfo->status_read_counter = 0;
        atomic_set(&ptdeviceinfo->data_event_counter, 0);
        atomic_set(&ptdeviceinfo->status_event_counter, 0);
}

/**
* netana_open - Called when device file is opened
*/
static int netana_open(struct inode* ptinode, struct file* ptfile)
{
        struct netana_info *ptdeviceinfo;
        int                ret = -ENODEV;

        try_module_get(THIS_MODULE);
        if ((ptdeviceinfo = idr_find(&netAnaIDR, iminor(ptinode))) ) {
                if (ptdeviceinfo->tkdevice && (ptdeviceinfo->tkdevice->addsucceded) ) {
                        mutex_lock(&ptdeviceinfo->tkdevice->lock);
                        if (ptdeviceinfo->tkdevice->hdevice){
                                ret  = -EBUSY;
                        } else {
                                if (NETANA_NO_ERROR == (netana_open_device(ptdeviceinfo->tkdevice->deviceinstance.szDeviceName,
                                                                &ptdeviceinfo->tkdevice->hdevice)) ) {
                                        /* enable userspace interrupt notifications */
                                        ptdeviceinfo->notify_interrupts = 1;
                                        ptfile->private_data = (void*)ptdeviceinfo;
                                        netana_reset_event_counter(ptdeviceinfo);
                                        ret = SUCCESS;
                                } else {
                                        ptdeviceinfo->tkdevice->hdevice = NULL;
                                }
                        }
                        mutex_unlock(&ptdeviceinfo->tkdevice->lock);
                }
        }
        if (ret) module_put(THIS_MODULE);

        return ret;
}

/**
* netana_open - Called when device file is closed
*/
static int netana_release(struct inode* ptinode, struct file* ptfile)
{
        struct netana_info *ptdeviceinfo = (struct netana_info*)ptfile->private_data;

        if(ptdeviceinfo) {
                if (ptdeviceinfo->tkdevice->addsucceded) {
                        netana_stop_capture(ptdeviceinfo->tkdevice->hdevice);
                }
                /* disable userspace interrupt notifications */
                ptdeviceinfo->notify_interrupts = 0;
                ptdeviceinfo->captureactive  = 0;
                ptdeviceinfo->data_read_sig  = 1;
                wake_up_interruptible(&ptdeviceinfo->waitqueue_data_notification);
                ptdeviceinfo->statuscallback  = 0;
                ptdeviceinfo->status_read_sig = 1;
                wake_up_interruptible(&ptdeviceinfo->waitqueue_status_notification);
                netana_close_device(ptdeviceinfo->tkdevice->hdevice);
                ptdeviceinfo->tkdevice->hdevice = NULL;
                clean_up_task_list( ptdeviceinfo);
        }
        module_put(THIS_MODULE);

        return SUCCESS;
}

/**
* netana_registerdevice - Called during pci-enumeration process
* @parent       :    Pointer to parent pci device
* @ptdeviceinfo :    Pointer to the devices info structure
*/
int netana_registerdevice(struct device *parent, struct netana_info *ptdeviceinfo)
{
        int                     ret       = -ENOMEM;
        struct netana_tk_device *tkdevice = NULL;

        g_ulTraceLevel = ulTraceLevel;

        if (ulDMABufferSize != 0) {
                g_ulDMABufferSize = ulDMABufferSize;

        } else if (ulDMABufferCount != 0) {
                g_ulDMABufferCount = ulDMABufferCount;

        } else if (ulDMABufferTimeout != 0) {
                g_ulDMABufferTimeout = ulDMABufferTimeout;
        }
        if(g_ulTraceLevel & NETANA_TRACELEVEL_INFO) {
                printk(KERN_INFO "DMA-buffer configuration\n");
                printk(KERN_INFO "Counter: %lu\n", (long unsigned int)g_ulDMABufferCount);
                printk(KERN_INFO "Size   : 0x%X\n", (unsigned int)g_ulDMABufferSize);
                printk(KERN_INFO "Timeout: %lu\n", (long unsigned int)g_ulDMABufferTimeout);
        }

        if ((init_netana_class()))
                return ret;

        /* internal device structure */
        if (!(tkdevice = kzalloc(sizeof(struct netana_tk_device), GFP_KERNEL)))
                goto err_devmem;

        mutex_init(&tkdevice->lock);
        /* now we can add some informtion and other attributes of the device */
        ptdeviceinfo->tkdevice = tkdevice;
        if ((ret = netana_getminor(ptdeviceinfo)))
                goto err_get_minor;

        ptdeviceinfo->device = device_create(netana_class->class,
                                        parent,
                                        MKDEV(major, ptdeviceinfo->minor),
                                        NULL, DEVICE_NAME"_%d",
                                        ptdeviceinfo->minor);

        if ((ret = IS_ERR(ptdeviceinfo->device))) {
                printk(KERN_ERR "Failed to create device!\n");
                goto err_create_device;
        }

        /* setup device instance information, requried to initialize the device */
        snprintf(ptdeviceinfo->devname,
                NETANA_MAX_DEVICENAMESIZE,
                "%s_%d", DEVICE_NAME,
                ptdeviceinfo->minor);

        if (ptdeviceinfo->pcidevice) {
                switch (ptdeviceinfo->pcidevice->subsystem_device)
                {
                        case PCI_SUBDEVICE_ID_C100:
                                snprintf(ptdeviceinfo->tkdevice->deviceinstance.szDeviceName,
                                       NETANA_MAX_DEVICENAMESIZE,
                                        "%s_%d", "netSCOPE",
                                        ptdeviceinfo->minor);

                                break;
                        case PCI_SUBDEVICE_ID_CIFX:
                                snprintf(ptdeviceinfo->tkdevice->deviceinstance.szDeviceName,
                                        NETANA_MAX_DEVICENAMESIZE,
                                        "%s_%d", "cifXANALYZER",
                                        ptdeviceinfo->minor);
                                break;
                        default:
                                snprintf(ptdeviceinfo->tkdevice->deviceinstance.szDeviceName,
                                        NETANA_MAX_DEVICENAMESIZE,
                                        "%s_%d", "netANALYZER",
                                        ptdeviceinfo->minor);
                                break;
                }
        } else {
                snprintf(ptdeviceinfo->tkdevice->deviceinstance.szDeviceName, NETANA_MAX_DEVICENAMESIZE, "%s_%d", "netANALYZER", ptdeviceinfo->minor);
        }

        /* setup device information */
        ptdeviceinfo->tkdevice->deviceinstance.pvDPM                = ptdeviceinfo->dpminfo->internal_addr;
        ptdeviceinfo->tkdevice->deviceinstance.ulDPMPhysicalAddress = ptdeviceinfo->dpminfo->addr;
        ptdeviceinfo->tkdevice->deviceinstance.ulDPMSize            = ptdeviceinfo->dpminfo->size;
        ptdeviceinfo->tkdevice->deviceinstance.usDeviceClass        = ptdeviceinfo->deviceclass;
        ptdeviceinfo->tkdevice->deviceinstance.ulInterruptNr        = ptdeviceinfo->irq;
        ptdeviceinfo->tkdevice->deviceinstance.pvOSDependent        = (void*)ptdeviceinfo;
        ptdeviceinfo->drv_info                                      = &netana_class->drv_info;

        /* create DMA resources */
        if (0 > netana_create_dma( ptdeviceinfo)) {
                ret = -ENOMEM;
                goto error_dma;
        }
        /* resources for user space notifications - irq-handling */
        atomic_set(&ptdeviceinfo->status_event_counter, 0);
        atomic_set(&ptdeviceinfo->data_event_counter, 0);
        init_waitqueue_head(&ptdeviceinfo->waitqueue_dsr);
        init_waitqueue_head(&ptdeviceinfo->waitqueue_usr_notification);
        init_waitqueue_head(&ptdeviceinfo->waitqueue_status_notification);
        init_waitqueue_head(&ptdeviceinfo->waitqueue_data_notification);
        spin_lock_init(&ptdeviceinfo->irqspinlock);

        if (NULL == (ptdeviceinfo->dsrthread = kthread_run(netana_dsrhandler, ptdeviceinfo,"netana_dsr"))) {
                ret = -ENOMEM;
                printk(KERN_ERR "Error creating DSR!\n");
                goto err_reg_dsr_thread;
        /* add to toolkit control */
        } else if (NETANA_NO_ERROR != (ret = netana_tkit_deviceadd(&ptdeviceinfo->tkdevice->deviceinstance, ptdeviceinfo->minor)) ) {
                ptdeviceinfo->tkdevice->addsucceded = 0;
                printk(KERN_ERR "Error adding device to toolkit! Error: 0x%X\n", ret);
                goto err_add_to_toolkit;
        } else if (SUCCESS != netana_create_sysfsentries(ptdeviceinfo)) {
                printk(KERN_ERR "Error creating sysfs entries\n");
                ret = -ENOMEM;
                goto err_sysfs;
        } else {
                ptdeviceinfo->tkdevice->addsucceded = 1;
        }
        /* signal end of sysfs file creation */
        ret = kobject_uevent( &ptdeviceinfo->device->kobj, KOBJ_CHANGE);

        return SUCCESS;

err_sysfs:
        netana_tkit_deviceremove(&ptdeviceinfo->tkdevice->deviceinstance, 1);
err_add_to_toolkit:
        stop_dsr_thread( ptdeviceinfo);
err_reg_dsr_thread:
        netana_free_dma(ptdeviceinfo);
error_dma:
        device_destroy(netana_class->class, MKDEV(major, ptdeviceinfo->minor));
err_create_device:
        netana_freeminor(ptdeviceinfo);
err_get_minor:
        kfree(tkdevice);
err_devmem:
        netana_classdestroy();
        return ret;
}

/**
* netana_unregisterdevice - Called when module is unloaded
* @ptdeviceinfo :    Pointer to the devices info structure
*/
void netana_unregisterdevice(struct netana_info *ptdeviceinfo)
{
        struct netana_tk_device *tkdevice = ptdeviceinfo->tkdevice;
        /* TODO: move device handling in the open and close functions */
        /* remove device from toolkit control */
        if (ptdeviceinfo->tkdevice->addsucceded)
                netana_tkit_deviceremove(&ptdeviceinfo->tkdevice->deviceinstance, 1);

        /* stop dsr thread */
        stop_dsr_thread(ptdeviceinfo);

        /* free the DMA buffer */
        netana_free_dma(ptdeviceinfo);

        /* cleanup sysfs entries */
        clean_up_sysfs(ptdeviceinfo);

        /* delete basic kernel objects */
        kobject_put(ptdeviceinfo->kobj_ctl_dir);
        kobject_put(ptdeviceinfo->kobj_conf_dir);
        kobject_put(ptdeviceinfo->kobj_hw_dir);
        kobject_put(ptdeviceinfo->kobj_dev_info_dir);
        kobject_put(ptdeviceinfo->kobj_dma_dir);
        kobject_put(ptdeviceinfo->kobj_drv_info);

        /* delete device */
        device_destroy(netana_class->class, MKDEV(major, ptdeviceinfo->minor));
        /* free minor number */
        netana_freeminor(ptdeviceinfo);
        kfree(tkdevice);
        /* check if class could be destroyed */
        netana_classdestroy();
}
