#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>

#include <linux/uaccess.h>
#include <linux/time64.h> 
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/sysinfo.h>
#include <linux/unistd.h>
#include <linux/time.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>




void write_buf(const char *buf, struct file *file, loff_t *pos);


//obtiene el tiempo que a estado vivo el proceso

SYSCALL_DEFINE1(get_time_cpu,pid_t,pid){

    struct task_struct *task;

    unsigned long cpu_time;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    cpu_time = task ->utime + task -> stime;
    do_div(cpu_time,1000000);
    return cpu_time;
}

//tampoco funciona
SYSCALL_DEFINE1(wait_key, int, key) {
    struct file *file;
    struct input_event ev;
    loff_t pos = 0; // Posición de lectura para kernel_read
    int ret;

    // Abrir el archivo del evento del teclado
    file = filp_open("/dev/input/event1", O_RDONLY, 664);  // Ajusta 'event1' según el dispositivo correcto
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



//no funciona
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

//parte Proyecto

// #define PROC_NAME "sys_info_proc"

// static const struct proc_ops sysinfo_ops = {
//     .proc_open = sysinfo_open,
//     .proc_read = seq_read,
// };


//funcion escribir en un archivo recibe:
// const char * -> la cadena a escribir
// file *       -> la struct file del archivo
// loff_t       -> posicion a escribir
void write_buf(const char *buf, struct file *file, loff_t *pos){
    ssize_t bytes_written = kernel_write(file, buf, strlen(buf),pos);
    if (bytes_written < 0) {
        pr_err("Failed to write file\n");
        filp_close(file, NULL);
        //return bytes_written;
    }
    pr_info("Wrote %zd bytes\n", bytes_written);

}

SYSCALL_DEFINE0(get_proccess_info){
    
    struct file *file;
    loff_t pos=0;
    //ssize_t bytes_written;
    char buffer[255];

    struct sysinfo si; 
    struct task_struct *task;
    //struct list_head *list;
    unsigned long total_jiffies = jiffies;
    int first_process=1;

    file =filp_open("/tmp/info_proc_sopes2",O_CREAT | O_TRUNC | O_RDWR,644); // Abre archivo
    if(IS_ERR(file)){
        pr_err("No se pudo crear archivo\n");
        return PTR_ERR(file);
    }

    si_meminfo(&si); //obtiene informacion memoria


    //se escribe informacion de la memoria en formato son

    // seq_printf(m,"{\n");
    write_buf("{\n",file,&pos);
    // seq_printf(m, "\"ram_total\": %lu, \n", si.totalram * 4);
    scnprintf(buffer, sizeof(buffer),"\"ram_total\": %lu, \n", si.totalram * 4);
    write_buf(buffer,file,&pos);
    // seq_printf(m, "\"ram_free\": %lu, \n", si.freeram * 4);
    scnprintf(buffer, sizeof(buffer),"\"ram_free\": %lu, \n", si.freeram * 4);
    write_buf(buffer,file,&pos);
    // seq_printf(m, "\"ram_usage\": %lu, \n", (si.totalram - si.freeram) * 4);
    scnprintf(buffer, sizeof(buffer),"\"ram_usage\": %lu, \n", (si.totalram - si.freeram) * 4);
    write_buf(buffer,file,&pos);
    // seq_printf(m,"\"processes\": [\n");

    
    write_buf("\"processes\": [\n",file,&pos);

    for_each_process(task){
        
        unsigned long vsz = 0;
        unsigned long rss = 0;
        unsigned long totalram= si.totalram*4;
        unsigned long mem_usage = 0;
        unsigned long cpu_usage = 0;
        //char *cmdline = NULL;


        if(task->mm){
            vsz = task-> mm->total_vm << (PAGE_SHIFT - 10);
            rss = get_mm_rss(task->mm) << (PAGE_SHIFT - 10);

            mem_usage= (rss*10000)/ totalram; //obtiene uso de memoria del proceso
        }

        unsigned long total_time= task->utime + task->stime;
        cpu_usage = (total_time * 10000)/ total_jiffies; //obtiene uso cpu proceso
        //cmdline = get_process_cmdline(task);

        if(!first_process){
            //seq_printf(m,",\n");
            write_buf(",\n",file,&pos);

        }else{
            first_process=0;
                
        }


        //seq_printf(m, "  {\n");
        write_buf("  {\n",file,&pos);

        //seq_printf(m, "    \"PID\": %d,\n", task->pid);
        scnprintf(buffer, sizeof(buffer), "    \"PID\": %d,\n", task->pid);
        write_buf(buffer,file,&pos);
        //seq_printf(m, "    \"Name\": \"%s\",\n", task->comm);
        scnprintf(buffer, sizeof(buffer),"    \"Name\": \"%s\",\n", task->comm);
        write_buf(buffer,file,&pos);
        
        //seq_printf(m, "    \"Cmdline\": \"%s\",\n", cmdline ? cmdline : "N/A");
        //seq_printf(m, "    \"Vsz\": %lu,\n", vsz);
        scnprintf(buffer, sizeof(buffer),"    \"Vsz\": %lu,\n", vsz);
        write_buf(buffer,file,&pos);
        //seq_printf(m, "    \"Rss\": %lu,\n", rss);
        scnprintf(buffer, sizeof(buffer),"    \"Rss\": %lu,\n", rss);
        write_buf(buffer,file,&pos);
        //seq_printf(m, "    \"MemoryUsage\": %lu.%02lu,\n", mem_usage / 100, mem_usage % 100);
        scnprintf(buffer, sizeof(buffer),"    \"MemoryUsage\": %lu.%02lu,\n", mem_usage / 100, mem_usage % 100);
        write_buf(buffer,file,&pos);
        //seq_printf(m, "    \"CPUUsage\": %lu.%02lu\n", cpu_usage / 100, cpu_usage % 100);
        scnprintf(buffer, sizeof(buffer),"    \"CPUUsage\": %lu.%02lu\n", cpu_usage / 100, cpu_usage % 100);
        write_buf(buffer,file,&pos);
        //seq_printf(m, "  }");
        write_buf("  }",file,&pos);

     


    }
    //seq_printf(m, "\n]\n}\n");
    write_buf("\n]\n}\n",file,&pos);
    filp_close(file, NULL); // cierra archivo

    return 0;
}

//mata proceso, recibe un pid
SYSCALL_DEFINE1(kill_process_pid,pid_t,pid){

    struct task_struct *task;

    task = pid_task(find_vpid(pid),PIDTYPE_PID); //obtiene task

    if(!task) return -ESRCH;
 
    
    send_sig(SIGKILL,task,1); //manda senal kill
    pr_info("Matando proceso %d",pid);
    return 0;
}

//para un proceso, manda un pid
SYSCALL_DEFINE1(stop_process_pid,pid_t,pid){
 
    struct task_struct *task;

    task = pid_task(find_vpid(pid),PIDTYPE_PID); //obtiene task

    if(!task) return -ESRCH;

    send_sig(SIGSTOP,task,1); //envia senal stop
    
    pr_info("Parando proceso %d",pid);
    return 0;
}


//envia senal stop a un proceso
SYSCALL_DEFINE1(continue_process_pid,pid_t,pid){
 
    struct task_struct *task;

    task = pid_task(find_vpid(pid),PIDTYPE_PID); //obtiene proceso

    if(!task) return -ESRCH;

    send_sig(SIGCONT,task,1); //envia senal

    pr_info("Continuando proceso %d",pid);
    return 0;
}


//Obtiene el start_time de un pid sin formatear
SYSCALL_DEFINE2(get_startTime_cpu,pid_t,pid,unsigned long long __user *,start){

    struct task_struct *task;

    //unsigned long cpu_time;
    unsigned long long dato;
    //struct timespec start;

    task = pid_task(find_vpid(pid),PIDTYPE_PID); //obtiene proceso

    if(!task) return -ESRCH;

    //start = task->start_time;

    dato = task->start_time;
    //put_task_struct(task); 
    //cpu_time = (unsigned long)task ->start_time.tv_sec;

    if(copy_to_user(start,&dato,sizeof(dato))) return -EFAULT; // manda al espacio de usuario

    pr_info("Start time: %llu\n", (unsigned long long)dato);
    
    return 0;
}



SYSCALL_DEFINE1(get_energy_info,pid_t,pid){
    
   
    struct sysinfo si;
    
    struct task_struct *task;
    struct task_struct *child_task;
  
    unsigned long total_jiffies = jiffies;

    si_meminfo(&si); //obtiene informacion de la memoria

    unsigned long long read_bytes = 0, write_bytes = 0;
    unsigned long vsz = 0;
    unsigned long rss = 0;
    unsigned long totalram= si.totalram*PAGE_SIZE/1024;
    unsigned long mem_usage = 0;
    unsigned long cpu_usage = 0;
   
    task = pid_task(find_vpid(pid),PIDTYPE_PID); //obtiene proceso

    if(!task) return -ESRCH;

    if(task->mm){
        vsz = task-> mm->total_vm << (PAGE_SHIFT - 10);
        rss = get_mm_rss(task->mm) << (PAGE_SHIFT - 10);

        mem_usage= (rss*10000)/ totalram; //calculo uso de la memoria en kb
    }
 
    unsigned long total_time= task->utime + task->stime;
    cpu_usage = (total_time * 10000)/ total_jiffies; //calculo uso cpu
   
    for_each_thread(task, child_task) { //obtener datos i/o
        read_bytes += child_task->ioac.read_bytes;
        write_bytes += child_task->ioac.write_bytes;
    }

    // Suma de todos los datos del proceso

    //calculo de enregia en donde a la ponderacion final le sumo el io y el uso de memoria ram en MB,
    //y el % de uso del procesador, y le doy un peso de 200 al procesador porque si
    //al final se hace regla de 3 con un consume de 65W, la ponderacion esta mal
    // y nose porque
    
    unsigned long energy = (cpu_usage)*200 + mem_usage/(1024*1024) + (read_bytes + write_bytes)/(1024*1024*1024);

    //unsigned long energy = (cpu_usage) + mem_usage/1024 ;

    //  -> maximo consumo de bateria 65w
    //  -> maximo cpu 100%*2 nucleos
    //  -> maxima ram 12 gb
    //  -> maximo ancho de banda 32759808 mb/s
    // formula usando regla de 3 asumiendo que a maxima capacidad, la pc consume 65W
    energy = (energy*65) /(200*200+12880+(31992));
    //energy = (energy*65) /(200+12880);
    return energy;
}


// Esta funcion no funcion, nose por que :)
SYSCALL_DEFINE2(get_network_traffic, unsigned long __user *, rx_mb, unsigned long __user *, tx_mb)
{
    struct net_device *dev;
    unsigned long long total_rx = 0;
    unsigned long long total_tx = 0;
    const struct rtnl_link_stats64 *stats;

    
    //read_lock(&dev_base_lock);
    for_each_netdev(&init_net, dev) {
        
        stats = dev_get_stats(dev, NULL);
        if (dev) {
            total_rx += stats->rx_bytes;
            total_tx += stats->tx_bytes;
        }
    }
    //read_unlock(&dev_base_lock);

    
    total_rx = total_rx / (1024 * 1024);
    total_tx = total_tx / (1024 * 1024);

    
    if (copy_to_user(rx_mb, &total_rx, sizeof(unsigned long)))
        return -EFAULT;
    if (copy_to_user(tx_mb, &total_tx, sizeof(unsigned long)))
        return -EFAULT;

    return 0;
}