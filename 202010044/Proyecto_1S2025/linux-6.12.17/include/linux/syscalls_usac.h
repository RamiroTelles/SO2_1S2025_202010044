#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>
#include <linux/time64.h> 
#include <linux/uaccess.h> 

asmlinkage long get_time_cpu(pid_t pid);
asmlinkage long wait_key(int key);
asmlinkage long get_cpu_temp(void);
asmlinkage long get_proccess_info(void);
asmlinkage long kill_process_pid(pid_t pid);
asmlinkage long stop_process_pid(pid_t pid);
asmlinkage long continue_process_pid(pid_t pid);
asmlinkage long get_startTime_cpu(pid_t,pid,unsigned long long __user *,start);
asmlinkage long get_energy_info(void);
asmlinkage long get_network_traffic(void);


#endif