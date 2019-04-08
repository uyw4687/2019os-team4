#ifndef _LINUX_ROTATION_H
#define _LINUX_ROTATION_H

extern long sys_set_rotation(int degree);
extern long sys_rotlock_read(int degree, int range);
extern long sys_rotlock_write(int degree, int range);
extern long sys_rotunlock_read(int degree, int range);
extern long sys_rotunlock_write(int degree, int range);

#endif
