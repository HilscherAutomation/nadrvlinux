/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __NETANA_H__
#define __NETANA_H__

#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/firmware.h>

#include "netana_toolkit.h"

/* global definitions */
#define DEVICE_NAME   "netanalyzer"
#define SUCCESS       0

#define NETANA_DRIVER_VERSION_MAJOR    1
#define NETANA_DRIVER_VERSION_MINOR    0
#define NETANA_DRIVER_VERSION_BUILD    6
#define NETANA_DRIVER_VERSION_REVISION 0

#define PCI_VENDOR_ID_HILSCHER       0x15CF
#define PCI_DEVICE_ID_HILSCHER_NETX  0x0000
#define PCI_SUBDEVICE_ID_C500        0x0010
#define PCI_SUBDEVICE_ID_B500        0x0020
#define PCI_SUBDEVICE_ID_B500G       0x0022
#define PCI_SUBDEVICE_ID_C100        0x0012
#define PCI_SUBDEVICE_ID_50_2        0x0002
#define PCI_SUBDEVICE_ID_50_3        0x0003
#define PCI_SUBDEVICE_ID_CIFX        0x0000


struct netana_info;

struct netana_driver_info {
        int device_count;
#ifdef NETANA_PM_AWARE
        struct firmware fw_copy;
        struct firmware bl_copy;
#endif
};

struct netana_sysfs_dir {
  struct kobject     kobj;
  int                objnum;
  struct netana_info *deviceinfo;
};

/* group information structure */
struct netana_sysfs_groups {
  struct kobject          *kobj;
  struct netana_sysfs_dir **attgroup;
  int32_t                 attcnt;
};

/* generic attribute interface, used for group entries */
struct netana_sysfs_entry {
        struct attribute   attr;
        struct kobject     kobj;
        int                objnum;
        ssize_t (*show)( struct netana_info *, char *, void *user_data);
        ssize_t (*store)(struct netana_info *, const char *, size_t, void *user_data);
};

/* generic binary attribute interface, used for group entries */
struct netana_sysfs_entry_bin {
        struct bin_attribute attr;
        struct kobject       kobj;
        int                  objnum;
        ssize_t (*show)( struct netana_info *, int, char *);
        ssize_t (*store)(void*, int, char *, size_t);
};

/* tk-device information */
struct netana_tk_device {
        struct NETANA_DEVINSTANCE_T deviceinstance; /* pointer to the device instance                    */
        NETANA_HANDLE               hdevice;        /* handle returned by netana_open_device()           */
        struct mutex                lock;           /* used to synchronize device access                 */
        int                         addsucceded;    /* true if successfully added to the toolkit         */
        char                        *vala;          /* temporary storage of the filter settings -> value */
        char                        *maska;         /* mask array filter A                               */
        char                        *valb;          /* value array filter B                              */
        char                        *maskb;         /* mask array filter B                               */
};

/* netanalyzer memory information structure */
struct netana_mem {
        struct kobject            kobj;
        unsigned long             addr;
        unsigned long             size;
        void __iomem              *internal_addr;
};

/* netanalyzer dma memory information structure */
struct netana_dma_info {
        void     *data;
        uint32_t datalen;
        int      buffervalid;
};

/* generic device information structure */
struct netana_info {
        /* basic device information                                                                       */
        struct netana_tk_device    *tkdevice;         /* pointer to toolkit device info                   */
        struct pci_dev             *pcidevice;        /* pci info                                         */
        struct device              *device;
        int                        flashbased;        /* 0: ram-based, 1: flash-based                     */
        int                        deviceclass;
        int                        minor;
        char                       devname[NETANA_MAX_DEVICENAMESIZE];
        struct netana_driver_info  *drv_info;

        /* memory, dma, irq resources */
        struct netana_mem          *dpminfo;          /* pointer the dpm information structure             */
        struct netana_mem          **dmainfo;         /* pointer the dma information structure             */
        struct task_struct         *dsrthread;        /* DSR thread                                        */
        int                        dsr_sig;           /* flag indicates DSR execution                      */
        wait_queue_head_t          waitqueue_dsr;     /* waitqueue for DSR                                 */
        unsigned int               irq;               /* DPM interrupt number                              */
        spinlock_t                 irqspinlock;       /* lock for shared buffer (ISR/DSR)                  */
        int                        notify_interrupts; /* flag indicates if state change should be notified */
        /* waitqueue for user notification                                                                 */
        wait_queue_head_t          waitqueue_usr_notification;
        /* waitqueue for user confirmation (state change)                                                  */
        wait_queue_head_t          waitqueue_status_notification;
        int                        status_read_sig;      /* flag indicates state change                    */
        int                        status_read_counter;  /* read status counter (user)                     */
        atomic_t                   status_event_counter; /* state change counter                           */
        /* waitqueue for user confirmation (data read)                                                     */
        wait_queue_head_t          waitqueue_data_notification;
        int                        data_read_sig;      /* flag indicates new data                          */
        atomic_t                   data_event_counter; /* new data counter                                 */
        int                        data_read_counter;  /* read data counter (user)                         */
        struct netana_dma_info     cur_dma_buf;        /* points to current dma buffer to read             */

        /* sysfs objects (base directories)                                                                */
        struct kobject             *kobj_conf_dir;     /* /netanalyzerX/device_config/                     */
        struct kobject             *kobj_ctl_dir;      /* /netanalyzerX/device_control/                    */
        struct kobject             *kobj_dev_info_dir; /* /netanalyzerX/device_information/                */
        struct kobject             *kobj_drv_info;     /* /netanalyzerX/driver_information/                */
        struct kobject             *kobj_hw_dir;       /* /netanalyzerX/hw_resources/                      */
        struct kobject             *kobj_dma_dir;      /* /netanalyzerX/driver_information/dma/            */
        /* sysfs file information objects                                                                  */
        /* /netanalyzerX/device_config/... */
        struct netana_sysfs_dir    *nasysfs_timeout_ctl_dir;  /* pointer to the sysfs entries timeout/...         */
        /* /netanalyzerX/device_control/... */
        struct netana_sysfs_dir    *nasysfs_capture_ctl_dir;  /* pointer to the sysfs entries capture_control/... */
        struct netana_sysfs_dir    *nasysfs_exec_cmd_dir;     /* pointer to the sysfs entries exec_cmd/...        */
        struct netana_sysfs_dir    *nasysfs_mbx_dir;          /* pointer to the sysfs entries mailbox/...         */
        struct netana_sysfs_dir    *nasysfs_pi_ctl_dir;       /* pointer to the sysfs entries pi/...              */
        struct netana_sysfs_dir    *nasysfs_time_ctl_dir;     /* pointer to the sysfs entries time/...            */
        /* /netanalyzerX/device_information/... */
        struct netana_sysfs_dir    *nasysfs_devfeature_dir;   /* pointer to the sysfs entries device_features/... */
        struct netana_sysfs_dir    *nasysfs_devinfo_dir;      /* pointer to the sysfs entries device_info/....    */
        struct netana_sysfs_dir    *nasysfs_sysdevstate_dir;  /* pointer to the sysfs entries device_state/       */
        struct netana_sysfs_dir    *nasysfs_sysidentinfo_dir; /* pointer to the sysfs entries ident_info/...      */
        /* /netanalyzerX/driver_information/... */
        struct netana_sysfs_dir    *nasysfs_sysdrvinfo_dir;   /* pointer to sysfs entry -> driver_information/    */
        /* /netanalyzerX/fw/... */
        struct netana_sysfs_dir    *nasysfs_fwerror_dir;
        /* /netanalyzerX/hw_resources/... */
        struct netana_sysfs_groups sysgpiogroup;              /* pointer to the sysfs entries gpio/gpio0...       */
        struct netana_sysfs_groups sysportgroup;              /* pointer to the sysfs entries port/port0...       */
        struct netana_sysfs_groups sysphygroup;               /* pointer to the sysfs entries phy/phy0...         */
        struct netana_sysfs_groups sysfiltergroup;            /* pointer to the sysfs entries filter/filter0...   */

        struct list_head           act_task_list;      /* used to store netanalyzer specific error values  */
        struct mutex               task_list_lock;     /* used to synchonize access to act_task_list       */

        int                        captureactive;
        int                        statuscallback;
#ifdef NETANA_PM_AWARE
        int                        resumephase;
#endif
        uint8_t                    identinfoblock[IDENT_INFO_BLOCK_SIZE];
};

int  netana_registerdevice   ( struct device      *parent, struct netana_info *deviceinfo);
void netana_unregisterdevice ( struct netana_info *deviceinfo);

void netana_notify_data	(void*    dmabuf,       uint32_t datalen,      void* userdata);
void netana_notify_state(uint32_t capturestate, uint32_t captureerror, void* userdata);

int netana_update_dsr_thread_prio( struct netana_info* ptdeviceinfo, int iPolicy, int iPrio);

#endif //__NETANA_H__

