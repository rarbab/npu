// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 IBM Corporation
 */

#ifndef __DRV_H
#define __DRV_H

void npu_dump_config(struct pci_dev *pd, struct seq_file *seq);
u32 npu_run_procedure(struct pci_dev *pd, u32 pnum);
int npu_chip_id(struct pci_dev *pd);
int npu_chip_link_index(struct pci_dev *pd);

#endif /* __DRV_H */
