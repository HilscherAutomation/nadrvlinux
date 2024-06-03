#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/mutex.h>
#include "netana.h"
#include <linux/pm.h>

/* global definitions */
#define DRIVER_AUTHOR "sdoell"
#define DRIVER_DESC   "Device driver for netAnalyzer hardware\n\t\tHilscher Gesellschaft fuer Systemautomation mbH"

/**
* netana_pci_probe - called when pci device matches one of the registered 'netana_pci_ids'
* ptpci_device:      Pointer to pci device
* ptid:              Pointer to the pci information
*/
static int netana_pci_probe( struct pci_dev* ptpci_device, const struct pci_device_id* ptid)
{
        struct netana_info *ptdeviceinfo;
        struct netana_mem  *ptdevicemem = NULL;
        int                ibar         = 0;
        int                ret          = -ENODEV;

        if (!(ptdeviceinfo = kzalloc(sizeof(struct netana_info), GFP_KERNEL)))
                return -ENOMEM;

        if (!(ptdevicemem = kzalloc(sizeof(struct netana_mem), GFP_KERNEL))) {
                kfree(ptdeviceinfo);
                return -ENOMEM;
        }
        /* validate which device is found */
        switch (ptid->device) {
                case PCI_DEVICE_ID_HILSCHER_NETX:
                        ibar = 0;
                        switch (ptid->subdevice)
                        {
                                case PCI_SUBDEVICE_ID_C100:
                                        printk(KERN_INFO "Found netSCOPE device!\n");
                                        break;
                                case PCI_SUBDEVICE_ID_CIFX:
                                        printk(KERN_INFO "Found cifXANALYZER device!\n");
                                        break;
                                default:
                                        printk(KERN_INFO "Found netANALYZER device!\n");
                                        break;
                        }
                        break;
                default:
                        break;
        }
        if ((ret = pci_enable_device(ptpci_device))) {
                goto err_enable;
        }

        /* Todo validate memory types of the device and call the coresponding module */
        if ( 0 > (ret = pci_request_regions( ptpci_device, "netx")))
                goto out_pci_request;

        if ((ptdevicemem->addr    = pci_resource_start( ptpci_device, 0)) ) {
                ptdevicemem->size = pci_resource_len( ptpci_device, ibar);
                ptdevicemem->internal_addr = ioremap( ptdevicemem->addr,
                                                      ptdevicemem->size);
        }
        if (ptdevicemem->internal_addr == NULL)
                goto err_pci_resource;

        printk(KERN_INFO "card physical DPM: 0x%lX!\n", ptdevicemem->addr);
        printk(KERN_INFO "DPM mapped to    : 0x%lX!\n", (long unsigned int)ptdevicemem->internal_addr);
        printk(KERN_INFO "DPM size         : 0x%lX!\n", (long unsigned int)ptdevicemem->size);

        ptdeviceinfo->pcidevice = ptpci_device;
        ptdeviceinfo->dpminfo   = ptdevicemem;
        ptdeviceinfo->irq       = ptpci_device->irq;

        if (SUCCESS == (ret = netana_registerdevice( &ptpci_device->dev, ptdeviceinfo))) {
                pci_set_drvdata( ptpci_device, ptdeviceinfo);
                return SUCCESS;
        }
        printk(KERN_ERR "Device register failed with %d\n", ret);

        ptdeviceinfo->dpminfo = NULL;
        iounmap( ptdevicemem->internal_addr);

err_pci_resource:
        pci_release_regions( ptpci_device);
out_pci_request:
        pci_disable_device( ptpci_device);
err_enable:
        kfree(ptdevicemem);
        kfree(ptdeviceinfo);
        return ret;
}

/**
* netana_pci_remove - called on release
* ptpci_device:      Pointer to pci device
*/
static void netana_pci_remove( struct pci_dev* ptpci_device)
{
        struct netana_info* ptdeviceinfo = pci_get_drvdata( ptpci_device);

        if (ptdeviceinfo)
                netana_unregisterdevice(ptdeviceinfo);

        /* TODO: get device number */
        pci_release_regions( ptpci_device);
        pci_disable_device( ptpci_device);
        pci_set_drvdata( ptpci_device, NULL);
        iounmap( ptdeviceinfo->dpminfo->internal_addr);
        kfree( ptdeviceinfo->dpminfo);
        ptdeviceinfo->dpminfo = NULL;
        kfree( ptdeviceinfo);
}

#ifdef NETANA_PM_AWARE
static int netana_suspend( struct device* dev)
{
        struct pci_dev* ptpci_device = to_pci_dev(dev);
        struct netana_info* ptdeviceinfo = pci_get_drvdata( ptpci_device);

        if (ptdeviceinfo) {
                netana_close_device( &ptdeviceinfo->tkdevice->deviceinstance);
                netana_tkit_deviceremove(&ptdeviceinfo->tkdevice->deviceinstance, 1);
                ptdeviceinfo->tkdevice->addsucceded = 0;
        }
        return 0;
}

static int netana_resume( struct device* dev)
{
        struct pci_dev*     ptpci_device = to_pci_dev(dev);
        struct netana_info* ptdeviceinfo = pci_get_drvdata( ptpci_device);

        ptdeviceinfo->resumephase = 1;

        if (NETANA_NO_ERROR == netana_tkit_deviceadd( &ptdeviceinfo->tkdevice->deviceinstance))
                ptdeviceinfo->tkdevice->addsucceded = 1;

        ptdeviceinfo->resumephase = 0;

        return 0;
}

struct dev_pm_ops netana_pm_ops = {
        .suspend         = netana_suspend,
        .resume          = netana_resume,
};
#endif

/* device IDs */
static struct pci_device_id netana_pci_ids []= {
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = PCI_VENDOR_ID_HILSCHER,
.subdevice = PCI_SUBDEVICE_ID_C500, //NXANL-C500-RE Rev 4
},
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = PCI_VENDOR_ID_HILSCHER,
.subdevice = PCI_SUBDEVICE_ID_B500, //NXANL-B500E-RE
},
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = PCI_VENDOR_ID_HILSCHER,
.subdevice = PCI_SUBDEVICE_ID_B500G, //NXANL-B500G-RE
},
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = PCI_VENDOR_ID_HILSCHER,
.subdevice = PCI_SUBDEVICE_ID_C100, //NXANL-C100-RE
},
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = 0x0,
.subdevice = PCI_SUBDEVICE_ID_50_2, //NXANL-50-RE Rev 2
},
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = 0x0,
.subdevice = PCI_SUBDEVICE_ID_50_3, //NXANL-50-RE Rev 3
},
{
.vendor    = PCI_VENDOR_ID_HILSCHER,
.device    = PCI_DEVICE_ID_HILSCHER_NETX,
.subvendor = 0x0,
.subdevice = 0x0, //cifX ATTENTION: conflicts with uio_netx
},
{0, }
};

static struct pci_driver netana_pci_driver = {
.name      = "netAnaPCI",
.id_table  = netana_pci_ids,
.probe     = netana_pci_probe,
.remove    = netana_pci_remove,
#ifdef NETANA_PM_AWARE
.driver.pm = &netana_pm_ops,
#endif
};

#ifdef CONFIG_OF
#include <linux/platform_device.h>
#include <linux/of.h>
static int nanl_of_probe(struct platform_device *pdev)
{
        int ret = 0;
        struct netana_info *ptdeviceinfo = NULL;
        struct netana_mem *ptdevicemem = NULL;
        struct resource *res;
        int irq, flashbased, deviceclass, rc;

        dev_info(&pdev->dev, "Found netANALYZER device!\n");

        /* Memory allocation */
        ptdeviceinfo = devm_kzalloc(&pdev->dev, sizeof(*ptdeviceinfo), GFP_KERNEL);
        if (!ptdeviceinfo) {
                dev_err(&pdev->dev, "devm_kzalloc() failed!\n");
               return -ENOMEM;
        }
        ptdevicemem = devm_kzalloc(&pdev->dev, sizeof(*ptdevicemem), GFP_KERNEL);
        if (!ptdevicemem) {
                kfree( ptdeviceinfo);
                dev_err(&pdev->dev, "devm_kzalloc() failed!\n");
                return -ENOMEM;
        }
        /* Device tree parsing */
        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!res) {
                kfree( ptdevicemem);
                kfree( ptdeviceinfo);
                dev_err(&pdev->dev, "Invalid or missing 'reg' entry in DT!\n");
                return -EINVAL;
        }
        irq = platform_get_irq(pdev, 0);
        if (irq < 0) {
                dev_err(&pdev->dev, "Invalid or missing 'interrupts' entry in DT!\n");
                ret = -EINVAL;
                goto err;
        } else {
                const char *buf;
                int buflen;
                buf = of_get_property(pdev->dev.of_node, "startuptype", &buflen);
                if (buf && !strncmp(buf, "flash", buflen)) {
                        flashbased = 1;
                }
                else if (buf && !strncmp(buf, "ram", buflen)) {
                        flashbased = 0;
                }
                else {
                        dev_err(&pdev->dev, "Invalid or missing 'startuptype' entry in DT!\n");
                        ret = -EINVAL;
                        goto err;
                }
        }
        if (of_property_read_u32(pdev->dev.of_node, "deviceclass", &deviceclass)) {
                dev_err(&pdev->dev, "Invalid or missing 'deviceclass' entry in DT!\n");
                ret = -EINVAL;
                goto err;
        }
        /* Preliminary work to add netanalyzer devices */
        ptdevicemem->addr = res->start;
        ptdevicemem->size = res->end - res->start + 1;
        ptdevicemem->internal_addr = devm_ioremap(&pdev->dev, ptdevicemem->addr, ptdevicemem->size);

        printk(KERN_INFO "card physical DPM: 0x%lX!\n", ptdevicemem->addr);
        printk(KERN_INFO "DPM mapped to    : 0x%lX!\n", (long unsigned int)ptdevicemem->internal_addr);
        printk(KERN_INFO "DPM size         : 0x%lX!\n", (long unsigned int)ptdevicemem->size);

        ptdeviceinfo->pcidevice = NULL;
        ptdeviceinfo->dpminfo = ptdevicemem;
        ptdeviceinfo->irq = irq;
        ptdeviceinfo->flashbased = flashbased;
        ptdeviceinfo->deviceclass = deviceclass;

        rc = netana_registerdevice(&pdev->dev, ptdeviceinfo);
        if (rc) {
                dev_err(&pdev->dev, "netana_registerdevice() failed!\n");
                ret = -EINVAL;
                goto err;
        }

        dev_info(&pdev->dev, "successfully initialized!\n");

        platform_set_drvdata(pdev, ptdeviceinfo);

        return 0;
err:
        kfree( ptdevicemem);
        kfree( ptdeviceinfo);

        return ret;
}

static int nanl_of_remove(struct platform_device *pdev)
{
        struct netana_info *ptdeviceinfo = platform_get_drvdata(pdev);

        if (ptdeviceinfo != NULL)
                netana_unregisterdevice(ptdeviceinfo);

        devm_iounmap(&pdev->dev, ptdeviceinfo->dpminfo->internal_addr);
        kfree( ptdeviceinfo->dpminfo);
        ptdeviceinfo->dpminfo = NULL;
        kfree( ptdeviceinfo);

        return 0;
}

static const struct of_device_id nanl_of_match[] = {
       { .compatible = "hilscher,netanalyzer", },
       { /* sentinel */ },
};

static struct platform_driver nanl_of_driver = {
       .probe    = nanl_of_probe,
       .remove   = nanl_of_remove,
       .driver   = {
               .name = "netAnaOF",
               .of_match_table = of_match_ptr(nanl_of_match),
       },
};
#endif

static int __init netana_pci_init_module(void)
{
        int ret = pci_register_driver(&netana_pci_driver);
#ifdef CONFIG_OF
        if (ret == 0) {
                platform_driver_register(&nanl_of_driver);
        }
#endif
        return ret;
}

static void __exit netana_pci_exit_module(void)
{
        pci_unregister_driver(&netana_pci_driver);
#ifdef CONFIG_OF
        platform_driver_unregister(&nanl_of_driver);
#endif
}


module_init(netana_pci_init_module);
module_exit(netana_pci_exit_module);

MODULE_LICENSE("GPL");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
