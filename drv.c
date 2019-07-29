// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 IBM Corporation
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <asm/debugfs.h>

struct dentry *npu_debugfs_add(struct pci_dev *pd, struct dentry *parent);

void npu_dump_config(struct pci_dev *pd, struct seq_file *seq)
{
	u32 dword;
	int pos;

	seq_printf(seq, "%s (rev %x)\n", dev_name(&pd->dev), pd->revision);
	
	for (pos = 0; pos < 256; pos += 4) {
		if (!(pos % 16))
			seq_printf(seq, "%02x:", pos);

		pci_read_config_dword(pd, pos, &dword);
		seq_printf(seq, " %02x %02x %02x %02x",
			   dword & 0xff,
			   dword >> 8 & 0xff,
			   dword >> 16 & 0xff,
			   dword >> 24);

		if (pos % 16 == 12)
			seq_puts(seq, "\n");
	}
}

#define PROCEDURE_STATUS_REG	0x84
#define PROCEDURE_CONTROL_REG	0x88
#define PROCEDURE_COMPLETE	(1 << 30)

u32 npu_run_procedure(struct pci_dev *pd, u32 pnum)
{
	u32 rc;

	pci_write_config_dword(pd, PROCEDURE_CONTROL_REG, pnum);
	pci_read_config_dword(pd, PROCEDURE_STATUS_REG, &rc);

	while (!(rc & PROCEDURE_COMPLETE)) {
		usleep_range(500, 1500);
		pci_read_config_dword(pd, PROCEDURE_STATUS_REG, &rc);
	}

	return rc;
}

int npu_chip_id(struct pci_dev *pd)
{
	return of_get_ibm_chip_id(pd->dev.of_node);
}

#define LINK_NUM		0x8c

int npu_chip_link_index(struct pci_dev *pd)
{
	u8 dev_index;

	pci_read_config_byte(pd, LINK_NUM, &dev_index);

	return dev_index;
}

static struct pci_device_id npu_id_table[] = {
	{ PCI_VDEVICE(IBM, 0x04ea) },
	{}
};

static struct dentry *npu_debugfs_root = NULL;

struct npu_drvdata {
	struct dentry *debugfs;
};

static int npu_probe(struct pci_dev *pd, const struct pci_device_id *id)
{
	struct device *dev = &pd->dev;
	struct npu_drvdata *drvdata;

	drvdata = devm_kmalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	drvdata->debugfs = npu_debugfs_add(pd, npu_debugfs_root);
	dev_set_drvdata(dev, drvdata);

	return 0;
}

static void npu_remove(struct pci_dev *pd)
{
	struct device *dev = &pd->dev;
	struct npu_drvdata *drvdata;

	drvdata = dev_get_drvdata(dev);
	debugfs_remove_recursive(drvdata->debugfs);
	devm_kfree(dev, drvdata);
}

static struct pci_driver npu_driver = {
	.name		= KBUILD_MODNAME,
	.id_table	= npu_id_table,
	.probe		= npu_probe,
	.remove		= npu_remove,
};

static int __init npu_module_init(void)
{
	int rc;

	npu_debugfs_root = debugfs_create_dir(npu_driver.name,
					      powerpc_debugfs_root);

	rc = pci_register_driver(&npu_driver);
	if (rc)
		debugfs_remove_recursive(npu_debugfs_root);

	return rc;
}

static void __exit npu_module_exit(void)
{
	pci_unregister_driver(&npu_driver);
	debugfs_remove_recursive(npu_debugfs_root);
}

module_init(npu_module_init);
module_exit(npu_module_exit);

MODULE_LICENSE("GPL");
