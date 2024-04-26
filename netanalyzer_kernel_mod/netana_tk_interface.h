#ifndef __NETANA_TK_INTERFACE_H__
#define __NETANA_TK_INTERFACE_H__

#include <linux/slab.h>
#include <linux/syscalls.h>

#include "netana.h"
#include "netana_toolkit.h"


struct attrid {
        int  objnum;
        int  devnum;
        char *attname;
};

ssize_t netana_show_drvvers       ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t netana_show_dma_buffercnt ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t netana_show_dma_buffersize( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t netana_get_last_error     ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t netana_set_device_class   ( struct netana_info *ptdeviceinfo, const char *buf, size_t tsize, void *pvuser_data);
ssize_t netana_get_device_class   ( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data);
ssize_t netana_set_dsr_prio       ( struct netana_info *ptdeviceinfo, const char *buf, size_t tsize, void *pvuser_data);

ssize_t devstate_show     ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t devstate_show_poll( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t show_device_info  ( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);
ssize_t devfeature_show( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data);

ssize_t fwerror_store( struct netana_info *ptdeviceinfo, const char *buf, size_t tsize, void *pvuser_data);

ssize_t filtermask_show ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t filtermask_store( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t show_filter     ( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);
ssize_t store_filter    ( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);

ssize_t get_relation    ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t set_filter      ( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_gpio_get_reg_val( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);
ssize_t netana_get_gpiovoltage ( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data);
ssize_t netana_port_get_reg_val( struct netana_info *ptNetAnaDevice , char *buf, void *user_data);
ssize_t netana_phy_get_reg_val ( struct netana_info *ptNetAnaDevice, char *buf, void *user_data);

ssize_t netana_store_gpio_mode  ( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);
ssize_t netana_store_gpio_level ( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);
ssize_t netana_set_gpiovoltage  ( struct netana_info *ptdeviceinfo, const char *buf, size_t tsize, void *pvuser_data);

ssize_t netana_port_set_reg_val ( struct netana_info *ptNetAnaDevice, const char *buf, void *user_data);
ssize_t netana_phy_set_reg_val  ( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_config_control( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_blink_device( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_pi_config( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_time_config( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_timeout_config( struct netana_info *ptNetAnaDevice, const char *buf, size_t size, void *user_data);

ssize_t netana_show_ident_info( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data);
ssize_t netana_ident_update   ( struct netana_info *ptdeviceinfo, const char *buf, size_t size, void *pvuser_data);
ssize_t netana_read_ident     ( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);
ssize_t netana_write_ident    ( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);

ssize_t netana_read_mbx_status ( struct netana_info *ptdeviceinfo, char *buf, void *pvuser_data);
ssize_t netana_read_mbx_packet_bin ( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);
ssize_t netana_write_mbx_packet_bin( struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char* buf, loff_t off, size_t count);

void initlist ( struct netana_info *ptNetAnaDevice);
void cleanlist( struct netana_info *ptNetAnaDevice);

#endif //__NETANA_TK_INTERFACE_H__
