// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 IBM Corporation
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/pci.h>
#include <linux/debugfs.h>
#include "drv.h"

static int config_show(struct seq_file *seq, void *v)
{
	npu_dump_config((struct pci_dev *)seq->private, seq);
	return 0;
}

static int config_open(struct inode *inode, struct file *file)
{
	return single_open(file, config_show, inode->i_private);
}

static const struct file_operations config_fops = {
	.open		= config_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int run_procedure(void *data, u64 pnum)
{
	npu_run_procedure((struct pci_dev *)data, (u32)pnum);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(run_procedure_fops, NULL, run_procedure, "%llu\n");

struct dentry *npu_debugfs_add(struct pci_dev *pd, struct dentry *parent)
{
	struct dentry *dir;

	dir = debugfs_create_dir(dev_name(&pd->dev), parent);

	debugfs_create_file("config", 0444, dir, pd, &config_fops);
	debugfs_create_file("run_procedure", 0222, dir, pd,
			    &run_procedure_fops);

	return dir;
}
