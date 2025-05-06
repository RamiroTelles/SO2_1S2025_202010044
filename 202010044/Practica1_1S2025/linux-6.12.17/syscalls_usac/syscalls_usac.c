#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/delay.h>

SYSCALL_DEFINE1(get_time_cpu,pid_t,pid){

    struct task_struct *task;

    unsigned long cpu_time;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    cpu_time = task ->utime + task -> stime;
    do_div(cpu_time,1000000);
    return cpu_time;
}


SYSCALL_DEFINE1(wait_key, int, key) {
    struct file *file;
    struct input_event ev;
    loff_t pos = 0; // Posición de lectura para kernel_read
    int ret;

    // Abrir el archivo del evento del teclado
    file = filp_open("/dev/input/event1", O_RDONLY, 0);  // Ajusta 'event1' según el dispositivo correcto
    if (IS_ERR(file)) {
        printk(KERN_ERR "No se pudo abrir /dev/input/event1\n");
        return PTR_ERR(file);
    }

    printk(KERN_INFO "Esperando la tecla %d...\n", key);

    while (1) {
        ret = kernel_read(file, &ev, sizeof(ev), &pos);
        if (ret != sizeof(ev)) {
            printk(KERN_ERR "Error al leer evento del teclado\n");
            break;
        }

        if (ev.type == EV_KEY && ev.code == key && ev.value == 1) {
            printk(KERN_INFO "Tecla %d presionada!\n", key);
            break;
        }
        msleep(10);
    }

    filp_close(file, NULL);
    return 0;
}

SYSCALL_DEFINE0(get_cpu_temp) {
    struct file *file;
    char buf[16];
    loff_t pos = 0;
    long temp;
    int ret;

    //file = filp_open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY, 0);
    //if (IS_ERR(file)) 
     //   return PTR_ERR(file);

    //ret = kernel_read(file, buf, sizeof(buf) - 1, &pos);
    //filp_close(file, NULL);

    //if (ret < 0)
    //    return ret;

    printk("Pruebas temperatura");

    //buf[ret] = '\0'; 
    //kstrtol(buf, 10, &temp);
    
    return 44; 
}