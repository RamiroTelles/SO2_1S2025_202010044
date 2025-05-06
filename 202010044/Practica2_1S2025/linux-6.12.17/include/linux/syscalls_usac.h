#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>

asmlinkage long sincr_folders(const char* path1,const char* path2);
asmlinkage long get_time_cpu(pid_t pid);
asmlinkage long wait_key(int key);
asmlinkage long get_cpu_temp(void);



#endif