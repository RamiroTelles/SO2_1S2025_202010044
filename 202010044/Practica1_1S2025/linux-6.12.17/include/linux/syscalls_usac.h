#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>

asmlinkage long get_time_cpu(pid_t pid);
asmlinkage long wait_key(int key);
asmlinkage long get_cpu_temp(void);


#endif