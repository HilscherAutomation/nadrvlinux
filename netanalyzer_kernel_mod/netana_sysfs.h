#ifndef __NETANA_SYSFS_H__
#define __NETANA_SYSFS_H__

int  nasysfs_create_drv_info        ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_dev_state_info  ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_dev_features    ( struct netana_info *ptdeviceinfo);
int  nasysfs_create_fw_error        ( struct netana_info *ptdeviceinfo);
int  nasysfs_create_filter_dir      ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_dev_info        ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_gpio_dir        ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_port_dir        ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_phy_dir         ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_capture_ctl     ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_exec_cmd        ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_pi_cfg          ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_time_cfg        ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_timeout_cfg     ( struct netana_info *ptDeviceInfo);
int  nasysfs_create_ident_info      ( struct netana_info *ptdeviceinfo);
int  nasysfs_create_mbx_dir         ( struct netana_info *ptdeviceinfo);

void clean_up_sysfs            ( struct netana_info *ptDeviceInfo);
void init_task_list            ( struct netana_info *ptDeviceInfo);
void clean_up_task_list        ( struct netana_info *ptDeviceInfo);

#endif // __NETANA_SYSFS_H__
