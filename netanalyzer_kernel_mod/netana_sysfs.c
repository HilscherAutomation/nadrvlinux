#include <linux/slab.h>
#include <linux/syscalls.h>
#include <asm/current.h>
#include <linux/device.h>
#include <linux/version.h>

#include "netana.h"
#include "netana_toolkit.h"
#include "netana_tk_interface.h"


void cleanup_objset( struct netana_sysfs_groups *ptkobj, int32_t objcnt);

void release_netana_sysfs(struct kobject *kobj)
{
        struct netana_sysfs_dir *sysfs_entry = container_of( kobj, struct netana_sysfs_dir, kobj);
        kfree(sysfs_entry);
}

void release_netana_sysfs_dma(struct kobject *kobj)
{
        struct netana_mem *sysfs_entry = container_of( kobj, struct netana_mem, kobj);
        kfree(sysfs_entry);
}

void release_netana_sysfs_bin(struct kobject *kobj)
{
        struct netana_sysfs_entry *sysfs_entry = container_of( kobj, struct netana_sysfs_entry, kobj);
        kfree(sysfs_entry);
}

/**
* netana_attr_show - generic attribute read function
*/
ssize_t netana_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
        struct netana_sysfs_dir   *dir_entry  = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_sysfs_entry *attr_entry = container_of( attr, struct netana_sysfs_entry, attr);
        struct attrid user_data;

        if (dir_entry)//TODO currently only checked while capture_control is using this sysfs_op
                user_data.objnum = dir_entry->objnum;

        if (attr_entry)
                user_data.devnum = attr_entry->objnum;

        if (attr_entry->show)
                return attr_entry->show( dir_entry->deviceinfo, buf, (void*)&user_data);
        else
                return -EIO;
}

/**
* netana_attr_show - generic attribute write function
*/
ssize_t netana_attr_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t size)
{
        struct netana_sysfs_dir   *dir_entry  = container_of( kobj, struct netana_sysfs_dir, kobj);
        struct netana_sysfs_entry *attr_entry = container_of( attr, struct netana_sysfs_entry, attr);
        struct attrid user_data;

        user_data.objnum = dir_entry->objnum;
        user_data.devnum = attr_entry->objnum;

        if (attr_entry->store)
                return attr_entry->store( (void*)dir_entry->deviceinfo, buf, size, (void*)&user_data);

        return -EIO;
}

ssize_t netana_dma_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
        struct netana_mem *mem_info = container_of(kobj, struct netana_mem, kobj);

        if (0 == strcmp(attr->name, "addr"))
                return sprintf(buf, "0x%lX\n", mem_info->addr);
        else if (0 == strcmp(attr->name, "size"))
                return sprintf(buf, "0x%lX\n", mem_info->size);
        else
                return -EIO;
}


/* generic read/write operations */
static const struct sysfs_ops generic_sysfs_ops = {
        .show  = netana_attr_show,
        .store = netana_attr_store,
};
struct sysfs_ops dma_sysfs_ops = {
        .show  = netana_dma_show,
        .store = NULL,
};

/* --------------------------------------------- */
/* ----------- ATTRIBUTE DEFINIIION ------------ */
/* --------------------------------------------- */
/* -- dma information -- */
struct netana_sysfs_entry dma_info_addr =
        __ATTR(addr, S_IRUGO, NULL, NULL);
struct netana_sysfs_entry dma_info_size =
        __ATTR(size, S_IRUGO, NULL, NULL);
/* -- driver information -- */
struct netana_sysfs_entry drvinfo_dma_bufsiz =
        __ATTR(dma_buffersize, S_IRUGO, netana_show_dma_buffersize, NULL);
struct netana_sysfs_entry drvinfo_dma_bufcnt =
        __ATTR(dma_buffercnt, S_IRUGO, netana_show_dma_buffercnt, NULL);
struct netana_sysfs_entry drvinfo_show_drvver =
        __ATTR(driver_version, S_IRUGO, netana_show_drvvers, NULL);
struct netana_sysfs_entry drvinfo_show_error =
        __ATTR(error, S_IRUGO, netana_get_last_error, NULL);
struct netana_sysfs_entry drvinfo_dev_class =
        __ATTR(device_class, S_IRUGO | S_IWUSR | S_IWGRP, netana_get_device_class, netana_set_device_class);
struct netana_sysfs_entry drv_info_dsr_prio =
        __ATTR(dsr, S_IWUSR | S_IWGRP, NULL, netana_set_dsr_prio);
/* -- device ident info -- */
struct netana_sysfs_entry dev_ident_info =
        __ATTR(ident_info, S_IRUGO | S_IWUSR | S_IWGRP, netana_show_ident_info, netana_ident_update);
/* -- device state -- */
struct netana_sysfs_entry netana_dev_state =
        __ATTR(device_state, S_IRUGO, devstate_show, NULL);
struct netana_sysfs_entry netana_dev_state_poll =
        __ATTR(device_state_poll, S_IRUGO, devstate_show_poll, NULL);
/* -- device feature -- */
struct netana_sysfs_entry netana_dev_feature =
        __ATTR(device_features, S_IRUGO, devfeature_show, NULL);
/* -- fw information -- */
struct netana_sysfs_entry netana_fw_error =
        __ATTR(fw_error, S_IRUGO | S_IWUSR | S_IWGRP, NULL, fwerror_store);
/* -- filter information -- */
struct netana_sysfs_entry netana_dev_filterRel =
        __ATTR(relation, S_IRUGO | S_IWUSR | S_IWGRP, get_relation, set_filter);
/* -- gpio information -- */
struct netana_sysfs_entry gpio_status =
        __ATTR(status, S_IRUGO | S_IWUSR | S_IWGRP, netana_gpio_get_reg_val, netana_store_gpio_mode);
struct netana_sysfs_entry gpio_level =
        __ATTR(level, S_IWUSR, NULL, netana_store_gpio_level);
struct netana_sysfs_entry gpio_voltage_get =
        __ATTR(voltage, S_IRUGO, netana_get_gpiovoltage, NULL);
struct netana_sysfs_entry gpio_voltage_set =
        __ATTR(set_voltage, S_IRUGO | S_IWUSR | S_IWGRP, NULL, netana_set_gpiovoltage);
/* -- port information -- */
struct netana_sysfs_entry port_status =
        __ATTR(status, S_IRUGO, netana_port_get_reg_val, NULL);
/* -- phy information -- */
struct netana_sysfs_entry phy_regval0 =
        __ATTR(regval0, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval1 =
        __ATTR(regval1, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval2 =
        __ATTR(regval2, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval3 =
        __ATTR(regval3, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval4 =
        __ATTR(regval4, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval5 =
        __ATTR(regval5, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval6 =
        __ATTR(regval6, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval7 =
        __ATTR(regval7, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval8 =
        __ATTR(regval8, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval9 =
        __ATTR(regval9, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval10 =
        __ATTR(regval10, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval11 =
        __ATTR(regval11, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval12 =
        __ATTR(regval12, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval13 =
        __ATTR(regval13, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval14 =
        __ATTR(regval14, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval15 =
        __ATTR(regval15, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval16 =
        __ATTR(regval16, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval17 =
        __ATTR(regval17, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval18 =
        __ATTR(regval18, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval19 =
        __ATTR(regval19, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval20 =
        __ATTR(regval20, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval21 =
        __ATTR(regval21, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval22 =
        __ATTR(regval22, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval23 =
        __ATTR(regval23, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval24 =
        __ATTR(regval24, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval25 =
        __ATTR(regval25, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval26 =
        __ATTR(regval26, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval27 =
        __ATTR(regval27, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval28 =
        __ATTR(regval28, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval29 =
        __ATTR(regval29, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval30 =
        __ATTR(regval30, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
struct netana_sysfs_entry phy_regval31 =
        __ATTR(regval31, S_IRUGO | S_IWUSR | S_IWGRP, netana_phy_get_reg_val, netana_phy_set_reg_val);
/* -- device control -- */
struct netana_sysfs_entry netana_capture_control =
        __ATTR(control, S_IWUSR | S_IWGRP, NULL, netana_config_control);
struct netana_sysfs_entry dev_cmd_blink =
        __ATTR(identify, S_IRUGO | S_IWUSR | S_IWGRP, NULL, netana_blink_device);
struct netana_sysfs_entry dev_pi_config =
        __ATTR(config, S_IWUSR | S_IWGRP, NULL, netana_pi_config);
struct netana_sysfs_entry dev_time_config =
        __ATTR(config, S_IWUSR | S_IWGRP, NULL, netana_time_config);
struct netana_sysfs_entry dev_timeout_config =
        __ATTR(config, S_IWUSR | S_IWGRP, NULL, netana_timeout_config);
struct netana_sysfs_entry dev_mailbox_status =
        __ATTR(status, S_IRUGO, netana_read_mbx_status, NULL);

/* ----------- binary ------------ */
/* -- device information -- */
static struct netana_sysfs_entry_bin dev_info =
{
        .attr = {
                .attr = {
                .name = "device_info",
                .mode = S_IRUGO,
        },
        .read = show_device_info,
        .size = sizeof(NETANA_DEVICE_INFORMATION_T),
        },
};
/* -- ident check -- */
static struct netana_sysfs_entry_bin ident_bin =
{
        .attr = {
                .attr = {
                .name = "ident_check",
                .mode = S_IRUGO | S_IWUSR | S_IWGRP,
        },
        .read  = netana_read_ident,
        .write = netana_write_ident,
        .size = IDENT_INFO_BLOCK_SIZE,
        },
};
/* -- filter value -- */
static struct netana_sysfs_entry_bin filter_bin_attr[] = {
{
        .attr = {
                .attr = {
                .name = "vala",
                .mode = S_IRUGO | S_IWUSR | S_IWGRP,
        },
        .read = show_filter,
        .write = store_filter,
        .size = 0,
        },
},
{
        .attr = {
                .attr = {
                .name = "maska",
                .mode = S_IRUGO | S_IWUSR | S_IWGRP,
        },
        .read = show_filter,
        .write = store_filter,
        .size = 0,
        },
},
{
        .attr = {
                .attr = {
                .name = "valb",
                .mode = S_IRUGO | S_IWUSR | S_IWGRP,
        },
        .read = show_filter,
        .write = store_filter,
        .size = 0,
        },
},
{
        .attr = {
                .attr = {
                .name = "maskb",
                .mode = S_IRUGO | S_IWUSR | S_IWGRP,
        },
        .read  = show_filter,
        .write = store_filter,
        .size = 0,
        },
},
};
/* -- mailbox resources -- */
static struct netana_sysfs_entry_bin mailbox_bin_attr =
{
  .attr = {
    .attr = {
      .name = "mailbox",
      .mode = S_IRUGO | S_IWUSR | S_IWGRP,
    },
    .read = netana_read_mbx_packet_bin,
    .write = netana_write_mbx_packet_bin,
    .size = 0, /* set to unlimited driver will check size */
  },
};

/* --------------------------------------------- */
/* ----------- Attribute Groups  --------------- */
/* --------------------------------------------- */
/* dma */
static struct attribute *dma_attrs[] = {
        &dma_info_addr.attr,
        &dma_info_size.attr,
        NULL,
};
/* driver info */
static struct attribute *drvinfo_attrs[] = {
        &drvinfo_dma_bufsiz.attr,
        &drvinfo_dma_bufcnt.attr,
        &drvinfo_show_drvver.attr,
        &drvinfo_show_error.attr,
        &drvinfo_dev_class.attr,
        &drv_info_dsr_prio.attr,
        NULL,
};
/* device ident info */
static struct attribute *device_ident_attrs[] = {
        &dev_ident_info.attr,
        NULL,
};
/* device state */
static struct attribute *drvstate_attrs[] = {
        &netana_dev_state.attr,
        &netana_dev_state_poll.attr,
        NULL,
};
/* device feature */
static struct attribute *devfeature_attrs[] = {
        &netana_dev_feature.attr,
        NULL,
};
/* fw information */
static struct attribute *fwerror_attrs[] = {
        &netana_fw_error.attr,
        NULL,
};
/* filter settings */
static struct attribute *filter_attrs[] = {
        &netana_dev_filterRel.attr,
        NULL,
};
/* gpio settings */
static struct attribute *gpio_attrs[] = {
        &gpio_status.attr,
        &gpio_level.attr,
        &gpio_voltage_get.attr,
        NULL,
};
static struct attribute *gpio_voltage_attrs[] = {
        &gpio_voltage_set.attr,
        NULL,
};
/* port state */
static struct attribute *port_attrs[] = {
        &port_status.attr,
        NULL,
};
/* phy settings */
static struct attribute *phy_attrs[] = {
        &phy_regval0.attr,
        &phy_regval1.attr,
        &phy_regval2.attr,
        &phy_regval3.attr,
        &phy_regval4.attr,
        &phy_regval5.attr,
        &phy_regval6.attr,
        &phy_regval7.attr,
        &phy_regval8.attr,
        &phy_regval9.attr,
        &phy_regval10.attr,
        &phy_regval11.attr,
        &phy_regval12.attr,
        &phy_regval13.attr,
        &phy_regval14.attr,
        &phy_regval15.attr,
        &phy_regval16.attr,
        &phy_regval17.attr,
        &phy_regval18.attr,
        &phy_regval19.attr,
        &phy_regval20.attr,
        &phy_regval21.attr,
        &phy_regval22.attr,
        &phy_regval23.attr,
        &phy_regval24.attr,
        &phy_regval25.attr,
        &phy_regval26.attr,
        &phy_regval27.attr,
        &phy_regval28.attr,
        &phy_regval29.attr,
        &phy_regval30.attr,
        &phy_regval31.attr,
        NULL,
};
/* device control settings */
static struct attribute *capture_ctl_attrs[] = {
        &netana_capture_control.attr,
        NULL,
};
static struct attribute *blink_attrs[] = {
        &dev_cmd_blink.attr,
        NULL,
};
static struct attribute *pi_attrs[] = {
        &dev_pi_config.attr,
        NULL,
};
static struct attribute *time_config_attrs[] = {
        &dev_time_config.attr,
        NULL,
};
static struct attribute *timeout_config_attrs[] = {
        &dev_timeout_config.attr,
        NULL,
};
static struct attribute *mailbox_attrs[] = {
  &dev_mailbox_status.attr,
  NULL,
};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
ATTRIBUTE_GROUPS(dma);
ATTRIBUTE_GROUPS(drvinfo);
ATTRIBUTE_GROUPS(device_ident);
ATTRIBUTE_GROUPS(drvstate);
ATTRIBUTE_GROUPS(devfeature);
ATTRIBUTE_GROUPS(fwerror);
ATTRIBUTE_GROUPS(filter);
ATTRIBUTE_GROUPS(gpio);
ATTRIBUTE_GROUPS(gpio_voltage);
ATTRIBUTE_GROUPS(port);
ATTRIBUTE_GROUPS(phy);
ATTRIBUTE_GROUPS(capture_ctl);
ATTRIBUTE_GROUPS(blink);
ATTRIBUTE_GROUPS(pi);
ATTRIBUTE_GROUPS(time_config);
ATTRIBUTE_GROUPS(timeout_config);
ATTRIBUTE_GROUPS(mailbox);
#endif

/* --------------------------------------------- */
/* ----------- Type Definitions  --------------- */
/* --------------------------------------------- */
/* generic type for dynamic creation */
/*static struct kobj_type netana_generic_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
        .default_attrs = NULL,
};*/
struct kobj_type dma_map_attr_type = {
        .release       = release_netana_sysfs_dma,
        .sysfs_ops     = &dma_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = dma_groups,
#else
        .default_attrs = dma_attrs,
#endif
};
static struct kobj_type driverinfo_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = drvinfo_groups,
#else
        .default_attrs = drvinfo_attrs,
#endif
};
static struct kobj_type dev_ident_info_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = device_ident_groups,
#else
        .default_attrs = device_ident_attrs,
#endif
};
static struct kobj_type devstate_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = drvstate_groups,
#else
        .default_attrs = drvstate_attrs,
#endif
};
static struct kobj_type devfeature_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = devfeature_groups,
#else
        .default_attrs = devfeature_attrs,
#endif
};
static struct kobj_type fwerror_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = fwerror_groups,
#else
        .default_attrs = fwerror_attrs,
#endif
};
static struct kobj_type filter_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = filter_groups,
#else
        .default_attrs = filter_attrs,
#endif
};
static struct kobj_type gpio_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = gpio_groups,
#else
        .default_attrs = gpio_attrs,
#endif
};
static struct kobj_type gpio_volt_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = gpio_voltage_groups,
#else
        .default_attrs = gpio_voltage_attrs,
#endif
};
static struct kobj_type port_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = port_groups,
#else
        .default_attrs = port_attrs,
#endif
};
static struct kobj_type phy_attr_type = {
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = phy_groups,
#else
        .default_attrs = phy_attrs,
#endif
};
static struct kobj_type capture_control_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = capture_ctl_groups,
#else
        .default_attrs = capture_ctl_attrs,
#endif
};
static struct kobj_type exec_cmd_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = blink_groups,
#else
        .default_attrs = blink_attrs,
#endif
};

static struct kobj_type pi_config_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = pi_groups,
#else
        .default_attrs = pi_attrs,
#endif
};

static struct kobj_type time_config_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = time_config_groups,
#else
        .default_attrs = time_config_attrs,
#endif
};

static struct kobj_type timeout_config_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = timeout_config_groups,
#else
        .default_attrs = timeout_config_attrs,
#endif
};

static struct kobj_type mailbox_config_type =
{
        .release       = release_netana_sysfs,
        .sysfs_ops     = &generic_sysfs_ops,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
        .default_groups = mailbox_groups,
#else
        .default_attrs = mailbox_attrs,
#endif
};

/**
* ATTRIBUTE SPECIFIC SETUPS
* ATTRIBUTE SPECIFIC SETUPS
* ATTRIBUTE SPECIFIC SETUPS
*/

/**
 * nasysfs_create_drv_info() - Populates driver information directory
* +driver_information
* |->device_class   = class of device
* |->dma_buffercnt  = number of allocated dma buffers
* |->dma_buffersize = size of dma buffer
* |->driver_version = version kernel mode driver
* |->error          = netanalyzer driver specific error code
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_drv_info( struct netana_info *ptdeviceinfo)
{
        int                       ret = -ENOMEM;
        struct netana_sysfs_dir *drvinfo_attr;

        if (!(drvinfo_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                return -ENOMEM;

        /* store reference to private device info */
        drvinfo_attr->deviceinfo = ptdeviceinfo;

        kobject_init( &drvinfo_attr->kobj, &driverinfo_attr_type);
        if ((ret = kobject_add(&drvinfo_attr->kobj, ptdeviceinfo->kobj_drv_info, "driver_information")))
                goto err_add;

        /* store reference to be able to delete on shut down */
        ptdeviceinfo->nasysfs_sysdrvinfo_dir = drvinfo_attr;
        return ret;

err_add:
        kobject_put(&drvinfo_attr->kobj);
        return ret;
}

/**
 * nasysfs_create_ident_info() - Populates device ident information directory
* +ident_info
* |->ident_check (binary)
* |->ident_info
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_ident_info( struct netana_info *ptdeviceinfo)
{
        int                       ret = -ENOMEM;
        struct netana_sysfs_dir *indent_info_attr;

        if (!(indent_info_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                return -ENOMEM;

        /* store reference to private device info */
        indent_info_attr->deviceinfo = ptdeviceinfo;

        kobject_init( &indent_info_attr->kobj, &dev_ident_info_attr_type);
        if ((ret = kobject_add(&indent_info_attr->kobj, ptdeviceinfo->kobj_dev_info_dir, "ident_info")))
                goto err_add;

        /* store reference to be able to delete on shut down */
        ptdeviceinfo->nasysfs_sysidentinfo_dir = indent_info_attr;

        ret = sysfs_create_bin_file( &indent_info_attr->kobj, &ident_bin.attr);

        return ret;

err_add:
        kobject_put(&indent_info_attr->kobj);
        return ret;
}

/**
 * nasysfs_create_dev_state_info() - Populates device state directory
* +device_state
* |->device_state      = device current state
* |->device_state_poll = device current state (used from user space to confirm capture state changes)
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_dev_state_info( struct netana_info *ptdeviceinfo)
{
        int                       ret = -ENOMEM;
        struct netana_sysfs_dir *devstate_attr;

        if (!(devstate_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                return -ENOMEM;

        /* store reference to private device info */
        devstate_attr->deviceinfo = ptdeviceinfo;

        kobject_init(&devstate_attr->kobj, &devstate_attr_type);
        if ((ret = kobject_add(&devstate_attr->kobj, ptdeviceinfo->kobj_dev_info_dir, "device_state")))
                goto err_add;

        /* store reference to be able to delete on shut down */
        ptdeviceinfo->nasysfs_sysdevstate_dir = devstate_attr;
        return ret;

err_add:
        kobject_put(&devstate_attr->kobj);
        return ret;
}

/**
 * nasysfs_create_dev_info() - Populates device information directory
* +device_info
* |->device_info (binary) = device specific information
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_dev_info( struct netana_info *ptdeviceinfo)
{
        int                       ret = -ENOMEM;
        struct netana_sysfs_dir *devinfo_attr;
        struct kobj_type          *devinfo;

        if (!(devinfo= kzalloc(sizeof(struct kobj_type), GFP_KERNEL)))
                return -ENOMEM;

        devinfo->release = NULL;
        if (!(devinfo_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                goto err_mem;

        /* store reference to private device info */
        devinfo_attr->deviceinfo   = ptdeviceinfo;

        /* store reference to be able to delete on shut down */
        ptdeviceinfo->nasysfs_devinfo_dir = devinfo_attr;

        kobject_init(&devinfo_attr->kobj, devinfo);
        if ((ret = kobject_add(&devinfo_attr->kobj, ptdeviceinfo->kobj_dev_info_dir, "device_info"))) {
                goto err_add;
        }else {
                if ((ret = sysfs_create_bin_file( &devinfo_attr->kobj, &dev_info.attr)))
                        goto err_bin;
        }
        return ret;

err_bin:
        kobject_put(&devinfo_attr->kobj);
err_add:
        kfree(devinfo_attr);
err_mem:
        kfree(devinfo);
        return ret;
}

/**
 * nasysfs_create_dev_features() - Populates device features directory
* +device_features
* |->device_features = device specific features
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_dev_features( struct netana_info *ptdeviceinfo)
{
        int                       ret = -ENOMEM;
        struct netana_sysfs_dir *devfeature_attr;

        if (!(devfeature_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                return -ENOMEM;

        /* store reference to private device info */
        devfeature_attr->deviceinfo = ptdeviceinfo;

        kobject_init(&devfeature_attr->kobj, &devfeature_attr_type);
        if ((ret = kobject_add(&devfeature_attr->kobj, ptdeviceinfo->kobj_dev_info_dir, "device_features")))
                goto err_add;

        /* store reference to be able to delete on shut down */
        ptdeviceinfo->nasysfs_devfeature_dir = devfeature_attr;
        return ret;

err_add:
        kobject_put(&devfeature_attr->kobj);
        return ret;
}

/**
 * nasysfs_create_fw_error() - Populates firmware related information directory
* +fw
* |->fw_error = interface for firmware errors
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_fw_error( struct netana_info *ptdeviceinfo)
{
        int                       ret = -ENOMEM;
        struct netana_sysfs_dir *fwerror_attr;

        if (!(fwerror_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                return -ENOMEM;

        /* store reference to private device info */
        fwerror_attr->deviceinfo = ptdeviceinfo;

        kobject_init(&fwerror_attr->kobj, &fwerror_attr_type);
        if ((ret = kobject_add(&fwerror_attr->kobj, &ptdeviceinfo->device->kobj, "fw")))
                goto err_add;

        /* store reference to be able to delete on shut down */
        ptdeviceinfo->nasysfs_fwerror_dir = fwerror_attr;
        return ret;

err_add:
        kobject_put(&fwerror_attr->kobj);
        return ret;
}

/**
 * nasysfs_create_filter_dir() - Populates filter information directory
* +filter
* |->+filter0     = filter information of filter 0
* |  |->maska     = mask A to be applied
* |  |->maskb     = mask A to be applied
* |  |->relation  = relation of mask A and B
* |  |->vala      = filter value for A
* |  |->valb      = filter value for B
* |->filter1
* ...
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_filter_dir( struct netana_info *ptdeviceinfo)
{
        int            ret  = -ENOMEM;
        int32_t        lidx = 0;
        struct kobject *filter_set;

        if (ptdeviceinfo->tkdevice->deviceinstance.ulGpioCnt > 0)
        {
                /* create kobject for filter directory */
                if (!(filter_set = kobject_create_and_add( "filter", ptdeviceinfo->kobj_hw_dir)))
                        return -ENOMEM;

                if (!(ptdeviceinfo->sysfiltergroup.attgroup = kzalloc((ptdeviceinfo->tkdevice->deviceinstance.ulGpioCnt) * sizeof(struct netana_sysfs_dir*), GFP_KERNEL))) {
                        goto err_group;
                } else if (!(ptdeviceinfo->tkdevice->vala  = kzalloc( ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize, GFP_KERNEL))) {
                        goto err_filter_mem;
                } else if (!(ptdeviceinfo->tkdevice->maska = kzalloc( ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize, GFP_KERNEL))) {
                        goto err_filter_mem;
                } else if (!(ptdeviceinfo->tkdevice->valb  = kzalloc( ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize, GFP_KERNEL))) {
                        goto err_filter_mem;
                } else if (!(ptdeviceinfo->tkdevice->maskb = kzalloc( ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize, GFP_KERNEL))) {
                        goto err_filter_mem;
                }

                ptdeviceinfo->sysfiltergroup.kobj = filter_set;

                /* add filter entries for every available port */
                for(lidx = 0; lidx < ptdeviceinfo->tkdevice->deviceinstance.ulGpioCnt; ++lidx)
                {
                        struct netana_sysfs_dir *filter_attr;

                        if (!(filter_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                                goto err_filter;

                        /* store generic object number to be able to verify the port the filter is associated to */
                        filter_attr->objnum     = lidx;
                        filter_attr->deviceinfo = ptdeviceinfo;

                        kobject_init(&filter_attr->kobj, &filter_attr_type);
                        if ((ret = kobject_add(&filter_attr->kobj, filter_set, "filter%d", lidx))) {
                                kfree(filter_attr);
                                goto err_filter;
                        }else {
                                int i =0;
                                /* initialize vala,maska & vala, maskb */
                                for (i=0;i<=3;i++){
                                        filter_bin_attr[i].attr.size = ptdeviceinfo->tkdevice->deviceinstance.ulFilterSize;
                                        ret = sysfs_create_bin_file( &filter_attr->kobj, &filter_bin_attr[i].attr);
                                }
                                /* store reference to be able to delete on shut down */
                                ptdeviceinfo->sysfiltergroup.attgroup[lidx] = filter_attr;
                                ptdeviceinfo->sysfiltergroup.attcnt         = lidx+1;
                        }
                }
        } else {
                ret = SUCCESS;
        }
        return ret;

err_filter:
        cleanup_objset( &ptdeviceinfo->sysfiltergroup, ptdeviceinfo->sysfiltergroup.attcnt);
        ptdeviceinfo->sysfiltergroup.kobj = NULL;
err_filter_mem:
        if (ptdeviceinfo->tkdevice->vala) {
                kfree(ptdeviceinfo->tkdevice->vala);
                ptdeviceinfo->tkdevice->vala = NULL;
        }
        if (ptdeviceinfo->tkdevice->maska) {
                kfree(ptdeviceinfo->tkdevice->maska);
                ptdeviceinfo->tkdevice->maska = NULL;
        }
        if (ptdeviceinfo->tkdevice->valb) {
                kfree(ptdeviceinfo->tkdevice->valb);
                ptdeviceinfo->tkdevice->valb = NULL;
        }
        if (ptdeviceinfo->tkdevice->maskb) {
                kfree(ptdeviceinfo->tkdevice->maskb);
                ptdeviceinfo->tkdevice->maskb = NULL;
        }
err_group:
        kobject_put(filter_set);
        return -ENOMEM;
}

/**
 * nasysfs_create_gpio_dir() - Populates gpio information directory
* +gpio
* |->+gpio0     = information of gpio 0
* |  |->level   = gpio mode specific settings/info
* |  |->status  = gpio mode specific settings/info
* |  |->voltage = gpio mode specific settings/info
* |->gpio1
* ...
* |->+voltage
*    |->set_voltage = global voltage setting
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_gpio_dir( struct netana_info *ptdeviceinfo)
{
        struct kobject *gpio_set;
        int32_t        lidx = 0;
        int            ret  = -ENOMEM;

        if (ptdeviceinfo->tkdevice->deviceinstance.ulGpioCnt > 0)
        {
                /* create kobject for gpio directory */
                if (!(gpio_set = kobject_create_and_add( "gpio", ptdeviceinfo->kobj_hw_dir)))
                        return -ENOMEM;

                /* allocate one more for voltage attributes */
                ptdeviceinfo->sysgpiogroup.attgroup = kzalloc((ptdeviceinfo->tkdevice->deviceinstance.ulGpioCnt+1) * sizeof(struct netana_sysfs_dir*), GFP_KERNEL);
                ptdeviceinfo->sysgpiogroup.kobj = gpio_set;

                /* add gpio entries for every available gpio */
                for(lidx = 0; lidx < ptdeviceinfo->tkdevice->deviceinstance.ulGpioCnt; ++lidx)
                {
                        struct netana_sysfs_dir *gpio_attr;

                        if (!(gpio_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                                goto err_gpio;

                        /* store generic object number to be able to verify the gpio the file is associated to */
                        gpio_attr->objnum     = lidx;
                        gpio_attr->deviceinfo = ptdeviceinfo;

                        kobject_init(&gpio_attr->kobj, &gpio_attr_type);
                        if ((ret = kobject_add(&gpio_attr->kobj, gpio_set, "gpio%d", lidx))) {
                                kfree(gpio_attr);
                                goto err_gpio;
                        }else {
                                /* store reference to be able to delete on shut down */
                                ptdeviceinfo->sysgpiogroup.attgroup[lidx] = gpio_attr;
                                ptdeviceinfo->sysgpiogroup.attcnt         = lidx;
                        }
                }
                if (lidx>0)
                {
                        struct netana_sysfs_dir *gpio_attr;

                        if (!(gpio_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                                goto err_gpio;

                        /* add file for voltage setting */
                        gpio_attr->deviceinfo = ptdeviceinfo;

                        kobject_init(&gpio_attr->kobj, &gpio_volt_attr_type);
                        if ((ret = kobject_add(&gpio_attr->kobj, gpio_set, "voltage"))) {
                                kfree(gpio_attr);
                                goto err_gpio;
                        }else {
                                /* store reference to be able to delete on shut down */
                                ptdeviceinfo->sysgpiogroup.attgroup[lidx] = gpio_attr;
                                ptdeviceinfo->sysgpiogroup.attcnt         = lidx+1;
                        }
                }
        } else {
                ret = SUCCESS;
        }
        return ret;

err_gpio:
        cleanup_objset( &ptdeviceinfo->sysgpiogroup, ptdeviceinfo->sysgpiogroup.attcnt);
        ptdeviceinfo->sysgpiogroup.kobj = NULL;
        kobject_put(gpio_set);
        return -ENOMEM;
}

/**
 * nasysfs_create_port_dir() - Populates port information directory
* +port
* |->+port0    = information of port 0
* |  |->status = port specific status
* |->port1
* ...
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_port_dir( struct netana_info *ptdeviceinfo)
{
        struct kobject *port_set;
        int32_t        lidx = 0;
        int            ret  = -ENOMEM;

        if (ptdeviceinfo->tkdevice->deviceinstance.ulPortCnt > 0)
        {
                /* create kobject for port directory */
                if (!(port_set = kobject_create_and_add( "port", ptdeviceinfo->kobj_hw_dir)))
                        return -ENOMEM;

                ptdeviceinfo->sysportgroup.attgroup = kzalloc((ptdeviceinfo->tkdevice->deviceinstance.ulPortCnt) * sizeof(struct netana_sysfs_dir*), GFP_KERNEL);
                ptdeviceinfo->sysportgroup.kobj = port_set;

                /* add sysfs entries for every available port */
                for(lidx = 0; lidx < ptdeviceinfo->tkdevice->deviceinstance.ulPortCnt; ++lidx)
                {
                        struct netana_sysfs_dir *port_attr;

                        if (!(port_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                                goto err_port;

                        /* store generic object number to be able to verify the port the file is associated to */
                        port_attr->objnum     = lidx;
                        port_attr->deviceinfo = ptdeviceinfo;

                        kobject_init(&port_attr->kobj, &port_attr_type);
                        if ((ret = kobject_add(&port_attr->kobj, port_set, "port%d", lidx))) {
                                kfree(port_attr);
                                goto err_port;
                        } else {
                                /* store reference to be able to delete on shut down */
                                ptdeviceinfo->sysportgroup.attgroup[lidx] = port_attr;
                                ptdeviceinfo->sysportgroup.attcnt         = lidx+1;
                        }
                }
        } else {
                printk( KERN_INFO "Skip port resource creation (sysfs). No ports detected for device %s!\n", ptdeviceinfo->tkdevice->deviceinstance.szAliasName);
        }
        return ret;

err_port:
        cleanup_objset( &ptdeviceinfo->sysportgroup, ptdeviceinfo->sysportgroup.attcnt);
        ptdeviceinfo->sysportgroup.kobj = NULL;
        kobject_put(port_set);
        return -ENOMEM;
}

/**
 * nasysfs_create_phy_dir() - Populates phy information directory
* +phy
* |->+phy0       = information of phy 0
* |  |->regval0  = state of register 1
* ...
* |  |->regval31 = state of register 32
* |->phy1
* ...
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_phy_dir( struct netana_info *ptdeviceinfo)
{
        struct kobject *phy_set;
        int32_t        lidx = 0;
        int            ret  = -ENOMEM;

        if (ptdeviceinfo->tkdevice->deviceinstance.ulPortCnt > 0)
        {
                struct attribute **tmpphy_group;
                /* create kobject for phy directory */
                if (!(phy_set = kobject_create_and_add( "phy", ptdeviceinfo->kobj_hw_dir)))
                        return -ENOMEM;

                ptdeviceinfo->sysphygroup.attgroup = kzalloc((ptdeviceinfo->tkdevice->deviceinstance.ulPortCnt) * sizeof(struct netana_sysfs_dir*), GFP_KERNEL);

                /* set object index */
                tmpphy_group = phy_attrs;
                while(tmpphy_group[lidx]) {
                        struct netana_sysfs_entry *phy_regval = container_of(tmpphy_group[lidx], struct netana_sysfs_entry, attr);
                        phy_regval->objnum = lidx;
                        lidx++;
                }
                ptdeviceinfo->sysphygroup.kobj = phy_set;
                /* add sysfs entries for every available phy phy0... phy[max]*/
                for(lidx = 0; lidx < ptdeviceinfo->tkdevice->deviceinstance.ulPortCnt; ++lidx) {
                        struct netana_sysfs_dir *phy_attr;
                        if (!(phy_attr = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL)))
                                goto err_phy;

                        /* store generic object number to be able to verify the phy the file is associated to */
                        phy_attr->objnum     = lidx;
                        phy_attr->deviceinfo = ptdeviceinfo;

                        kobject_init(&phy_attr->kobj, &phy_attr_type);
                        if ((ret = kobject_add(&phy_attr->kobj, phy_set, "phy%d", lidx))) {
                                kfree(phy_attr);
                                goto err_phy;
                        } else {
                                /* store reference to be able to delete on shut down */
                                ptdeviceinfo->sysphygroup.attgroup[lidx] = phy_attr;
                                ptdeviceinfo->sysphygroup.attcnt         = lidx+1;
                        }
                }
        } else {
                printk( KERN_INFO "Skip port resource creation (sysfs). No ports detected for device %s!\n", ptdeviceinfo->tkdevice->deviceinstance.szAliasName);
        }
        return ret;

err_phy:
        cleanup_objset( &ptdeviceinfo->sysphygroup, ptdeviceinfo->sysphygroup.attcnt);
        ptdeviceinfo->sysphygroup.kobj = NULL;
        kobject_put(phy_set);
        return -ENOMEM;
}

/**
 * nasysfs_create_capture_ctl() - Populates capture control information directory
* +capture_control
* |->control = controls capture activity
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_capture_ctl( struct netana_info *ptdeviceinfo)
{
        struct netana_sysfs_dir *capture_control = NULL;
        int                       ret              = -ENOMEM;

        /* allocate new netana sysfs entry */
        if ((capture_control = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL))){

                /* initialize sysfs entry of type 'netana_generic_type' */
                kobject_init( &capture_control->kobj, &capture_control_type);
                /* create sysfs entry                                    */
                if ((ret = kobject_add( &capture_control->kobj,
                        ptdeviceinfo->kobj_ctl_dir, "capture_control")))
                        goto err_kobj;

                /* set private information (device info to which the dir belongs) */
                capture_control->deviceinfo = ptdeviceinfo;

                /* example adding additional sysfs file */
                /* if ((ret = sysfs_create_file( &capture_control->kobj, &my_attribute.attr))) */
                /*         goto err_kobj;                                                      */

                /* store reference to be able to delete on shut down */
                ptdeviceinfo->nasysfs_capture_ctl_dir = capture_control;
        }
        return ret;

err_kobj:
        kobject_put(&capture_control->kobj);
        return ret;
}

/**
 * nasysfs_create_exec_cmd() - Populates special commands directory
* +exec_cmd
* |->identify = (ui32) identify device (blink)
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_exec_cmd( struct netana_info *ptdeviceinfo)
{
        struct netana_sysfs_dir *exec_cmd = NULL;
        int                       ret       = -ENOMEM;

        /* allocate new netana sysfs entry */
        if ((exec_cmd = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL))){

                /* initialize sysfs dir entry of type 'netana_generic_type' */
                kobject_init( &exec_cmd->kobj, &exec_cmd_type);
                /* create sysfs entry                                    */
                if ((ret = kobject_add( &exec_cmd->kobj,
                        ptdeviceinfo->kobj_ctl_dir, "exec_cmd")))
                        goto err_kobj;

                /* set private information (device info to which the dir belongs) */
                exec_cmd->deviceinfo = ptdeviceinfo;

                /* store reference to be able to delete on shut down */
                ptdeviceinfo->nasysfs_exec_cmd_dir = exec_cmd;
        }
        return ret;
err_kobj:
        kobject_put(&exec_cmd->kobj);
        return ret;
}

/**
 * nasysfs_create_pi_cfg() - Populates pi configuration directory
* +pi
* |->config = (ui32, ui32) configuration of pi controller
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_pi_cfg( struct netana_info *ptdeviceinfo)
{
        struct netana_sysfs_dir *pi_control = NULL;
        int                     ret         = -ENOMEM;

        /* allocate new netana sysfs entry */
        if ((pi_control = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL))){

                /* initialize sysfs dir entry of type 'netana_generic_type' */
                kobject_init( &pi_control->kobj, &pi_config_type);
                /* create sysfs entry                                    */
                if ((ret = kobject_add( &pi_control->kobj,
                        ptdeviceinfo->kobj_ctl_dir, "pi")))
                        goto err_kobj;

                /* set private information (device info to which the dir belongs) */
                pi_control->deviceinfo = ptdeviceinfo;

                /* store reference to be able to delete on shut down */
                ptdeviceinfo->nasysfs_pi_ctl_dir = pi_control;
        }
        return ret;
err_kobj:
        kobject_put(&pi_control->kobj);
        return ret;
}

/**
 * nasysfs_create_time_cfg() - Populates time resync configuration directory
* +time
* |->config = (ui64) reference time to set
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_time_cfg( struct netana_info *ptdeviceinfo)
{
        struct netana_sysfs_dir *time_control = NULL;
        int                       ret           = -ENOMEM;

        /* allocate new netana sysfs entry */
        if ((time_control = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL))){

                /* initialize sysfs dir entry of type 'netana_generic_type' */
                kobject_init( &time_control->kobj, &time_config_type);
                /* create sysfs entry                                    */
                if ((ret = kobject_add( &time_control->kobj,
                        ptdeviceinfo->kobj_ctl_dir, "time")))
                        goto err_kobj;

                /* set private information (device info to which the dir belongs) */
                time_control->deviceinfo = ptdeviceinfo;

                /* store reference to be able to delete on shut down */
                ptdeviceinfo->nasysfs_time_ctl_dir = time_control;
        }
        return ret;
err_kobj:
        kobject_put(&time_control->kobj);
        return ret;
}

/**
 * nasysfs_create_timeout_cfg() - Populates time resync configuration directory
* +timeout
* |->config = (ui32) default timeout to choose
*
* @ptdeviceinfo : pointer to device information structure
* returns -ENOMEM on error
*/
int nasysfs_create_timeout_cfg( struct netana_info *ptdeviceinfo)
{
        struct netana_sysfs_dir *timeout_control = NULL;
        int                       ret              = -ENOMEM;

        /* allocate new netana sysfs entry */
        if ((timeout_control = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL))){

                /* initialize sysfs dir entry of type 'netana_generic_type' */
                kobject_init( &timeout_control->kobj, &timeout_config_type);
                /* create sysfs entry                                    */
                if ((ret = kobject_add( &timeout_control->kobj,
                        ptdeviceinfo->kobj_conf_dir, "timeout")))
                        goto err_kobj;

                /* set private information (device info to which the dir belongs) */
                timeout_control->deviceinfo = ptdeviceinfo;

                /* store reference to be able to delete on shut down */
                ptdeviceinfo->nasysfs_timeout_ctl_dir = timeout_control;
        }
        return ret;
err_kobj:
        kobject_put(&timeout_control->kobj);
        return ret;
}

/**
 * nasysfs_create_mbx_dir() - Populates mailbox communication directory
 * +mailbox
 * |->mailbox = binary file read/write
 * |->status = (ui32, ui32) - recv, send status
 *
 * @ptdeviceinfo : pointer to device information structure
 * returns -ENOMEM on error
 */
int nasysfs_create_mbx_dir( struct netana_info *ptdeviceinfo)
{
        struct netana_sysfs_dir *nasysfs_mbx_dir = NULL;
        int                     ret              = -ENOMEM;

        /* allocate new netana sysfs entry */
        if ((nasysfs_mbx_dir = kzalloc(sizeof(struct netana_sysfs_dir), GFP_KERNEL))){
                /* initialize sysfs dir entry of type 'mailbox_config_type' */
                kobject_init( &nasysfs_mbx_dir->kobj, &mailbox_config_type);
                /* create sysfs entry                                    */
                if ((ret = kobject_add( &nasysfs_mbx_dir->kobj, ptdeviceinfo->kobj_ctl_dir, "mailbox")))
                        goto err_kobj;

                /* set private information (device info to which the dir belongs) */
                nasysfs_mbx_dir->deviceinfo = ptdeviceinfo;

                /* store reference to be able to delete on shut down */
                ptdeviceinfo->nasysfs_mbx_dir = nasysfs_mbx_dir;

                if ((ret = sysfs_create_bin_file( &nasysfs_mbx_dir->kobj, &mailbox_bin_attr.attr)))
                        goto err_kobj;

        }
        return ret;

err_kobj:
        kobject_put(&nasysfs_mbx_dir->kobj);
        return ret;
}

void cleanup_objset( struct netana_sysfs_groups *group, int32_t objcnt)
{
        if (!group)
                return;

        if (group->attgroup){
                while(--objcnt>0){
                        if (group->attgroup[objcnt]) {
                                struct netana_sysfs_dir *entry = group->attgroup[objcnt];
                                if (entry)
                                        kobject_put(&entry->kobj);
                        }
                }
                kfree(group->attgroup);
                group->attgroup = NULL;
        }
        group->attcnt = 0;
}

void remove_bin_files(struct netana_info *ptdeviceinfo)
{
        int i;
        if (ptdeviceinfo->sysfiltergroup.attgroup) {
                for (i=0;(ptdeviceinfo->sysfiltergroup.attgroup) && (i<=3);i++){
                        struct netana_sysfs_dir *entry = ptdeviceinfo->sysfiltergroup.attgroup[i];
                        if (entry)
                                sysfs_remove_bin_file(&entry->kobj, &filter_bin_attr[i].attr);
                }
        }
        if (ptdeviceinfo->nasysfs_devinfo_dir)
                sysfs_remove_bin_file(&ptdeviceinfo->nasysfs_devinfo_dir->kobj, &dev_info.attr);
        if (ptdeviceinfo->nasysfs_sysidentinfo_dir)
                sysfs_remove_bin_file(&ptdeviceinfo->nasysfs_sysidentinfo_dir->kobj, &ident_bin.attr);
        if (ptdeviceinfo->nasysfs_mbx_dir)
                sysfs_remove_bin_file(&ptdeviceinfo->nasysfs_mbx_dir->kobj, &mailbox_bin_attr.attr);
}

void clean_up_sysfs( struct netana_info *ptdeviceinfo)
{
        /* remove binary files */
        remove_bin_files(ptdeviceinfo);

        /* remove single files */
        if (ptdeviceinfo->nasysfs_capture_ctl_dir)
                kobject_put(&ptdeviceinfo->nasysfs_capture_ctl_dir->kobj);
        if (ptdeviceinfo->nasysfs_exec_cmd_dir)
                kobject_put(&ptdeviceinfo->nasysfs_exec_cmd_dir->kobj);
        if (ptdeviceinfo->nasysfs_pi_ctl_dir)
                kobject_put( &ptdeviceinfo->nasysfs_pi_ctl_dir->kobj);
        if (ptdeviceinfo->nasysfs_time_ctl_dir)
                kobject_put(&ptdeviceinfo->nasysfs_time_ctl_dir->kobj);
        if (ptdeviceinfo->nasysfs_timeout_ctl_dir)
                kobject_put(&ptdeviceinfo->nasysfs_timeout_ctl_dir->kobj);
        if (ptdeviceinfo->nasysfs_sysdrvinfo_dir)
                kobject_put( &ptdeviceinfo->nasysfs_sysdrvinfo_dir->kobj);
        if (ptdeviceinfo->nasysfs_sysdevstate_dir)
                kobject_put( &ptdeviceinfo->nasysfs_sysdevstate_dir->kobj);
        if (ptdeviceinfo->nasysfs_devfeature_dir)
                kobject_put( &ptdeviceinfo->nasysfs_devfeature_dir->kobj);
        if (ptdeviceinfo->nasysfs_fwerror_dir)
                kobject_put( &ptdeviceinfo->nasysfs_fwerror_dir->kobj);
        if (ptdeviceinfo->nasysfs_sysidentinfo_dir)
                kobject_put( &ptdeviceinfo->nasysfs_sysidentinfo_dir->kobj);
        if (ptdeviceinfo->nasysfs_devinfo_dir)
                kobject_put( &ptdeviceinfo->nasysfs_devinfo_dir->kobj);
        if (ptdeviceinfo->nasysfs_mbx_dir)
                kobject_put(&ptdeviceinfo->nasysfs_mbx_dir->kobj);

        /* remove all groups */
         if (ptdeviceinfo->sysgpiogroup.kobj) {
                 cleanup_objset( &ptdeviceinfo->sysgpiogroup,   ptdeviceinfo->sysgpiogroup.attcnt);
                 kobject_put(ptdeviceinfo->sysgpiogroup.kobj);
         }
         if (ptdeviceinfo->sysportgroup.kobj) {
                 cleanup_objset( &ptdeviceinfo->sysportgroup,   ptdeviceinfo->sysportgroup.attcnt);
                 kobject_put(ptdeviceinfo->sysportgroup.kobj);
         }
         if (ptdeviceinfo->sysphygroup.kobj) {
                 cleanup_objset( &ptdeviceinfo->sysphygroup,    ptdeviceinfo->sysphygroup.attcnt);
                 kobject_put(ptdeviceinfo->sysphygroup.kobj);
         }
         if (ptdeviceinfo->sysfiltergroup.kobj) {
                 cleanup_objset( &ptdeviceinfo->sysfiltergroup, ptdeviceinfo->sysfiltergroup.attcnt);
                 kobject_put(ptdeviceinfo->sysfiltergroup.kobj);
         }

        /* remove auxiliary resources */
        if (ptdeviceinfo->tkdevice->vala)
                kfree(ptdeviceinfo->tkdevice->vala);
        if (ptdeviceinfo->tkdevice->maska)
                kfree(ptdeviceinfo->tkdevice->maska);
        if (ptdeviceinfo->tkdevice->valb)
                kfree(ptdeviceinfo->tkdevice->valb);
        if (ptdeviceinfo->tkdevice->maskb)
                kfree(ptdeviceinfo->tkdevice->maskb);

        ptdeviceinfo->tkdevice->vala  = NULL;
        ptdeviceinfo->tkdevice->maska = NULL;
        ptdeviceinfo->tkdevice->valb  = NULL;
        ptdeviceinfo->tkdevice->maskb = NULL;
}

void init_task_list( struct netana_info *ptdeviceinfo)
{
        initlist( ptdeviceinfo);
}

void clean_up_task_list( struct netana_info *ptdeviceinfo)
{
        cleanlist(ptdeviceinfo);
}
