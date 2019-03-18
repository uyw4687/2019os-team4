#ifndef _LINUX_PTREE_H
#define _LINUX_PTREE_H

#include <linux/prinfo.h>

extern int sys_ptree(struct prinfo *buf, int *nr);

#endif
