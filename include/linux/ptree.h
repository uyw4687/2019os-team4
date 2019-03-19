#ifndef _LINUX_PTREE_H
#define _LINUX_PTREE_H

#include <linux/prinfo.h>

extern long sys_ptree(struct prinfo *buf, int *nr);

#endif
