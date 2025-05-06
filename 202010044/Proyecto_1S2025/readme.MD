# Diseno sobre Syscall que sincroniza Archivos De arpetas

## Librerias clave utilizadas

Se ha hecho uso de inotify para la recopilacion de eventos de creacion, modificacion y eliminacion. Hilos de kernel para la sincronizacion de ambas carpetas, y mutex para evitar condiciones de carrera y problemas de sincronizacion entre cada hilo.

## SysCalls Creadas

```cpp
void write_buf(const char *buf, struct file *file, loff_t *pos);

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

//parte Proyecto

// #define PROC_NAME "sys_info_proc"

// static const struct proc_ops sysinfo_ops = {
//     .proc_open = sysinfo_open,
//     .proc_read = seq_read,
// };

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

    struct sysinfo si; // estructura que contiene la informacion de la memoria
    struct task_struct *task;
    //struct list_head *list;
    unsigned long total_jiffies = jiffies;
    int first_process=1;

    file =filp_open("/tmp/info_proc_sopes2",O_CREAT | O_TRUNC | O_RDWR,644);
    if(IS_ERR(file)){
        pr_err("No se pudo crear archivo\n");
        return PTR_ERR(file);
    }

    si_meminfo(&si); // obtiene la informacion de la memoria

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

            mem_usage= (rss*10000)/ totalram;
        }

        unsigned long total_time= task->utime + task->stime;
        cpu_usage = (total_time * 10000)/ total_jiffies;
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
    filp_close(file, NULL);

    return 0;
}

SYSCALL_DEFINE1(kill_process_pid,pid_t,pid){

    struct task_struct *task;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;
 
    
    send_sig(SIGKILL,task,1);
    pr_info("Matando proceso %d",pid);
    return 0;
}

SYSCALL_DEFINE1(stop_process_pid,pid_t,pid){
 
    struct task_struct *task;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    send_sig(SIGSTOP,task,1);
    
    pr_info("Parando proceso %d",pid);
    return 0;
}

SYSCALL_DEFINE1(continue_process_pid,pid_t,pid){
 
    struct task_struct *task;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    send_sig(SIGCONT,task,1);

    pr_info("Continuando proceso %d",pid);
    return 0;
}

SYSCALL_DEFINE2(get_startTime_cpu,pid_t,pid,unsigned long long __user *,start){

    struct task_struct *task;

    //unsigned long cpu_time;
    unsigned long long dato;
    //struct timespec start;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    //start = task->start_time;

    dato = task->start_time;
    //put_task_struct(task); 
    //cpu_time = (unsigned long)task ->start_time.tv_sec;

    if(copy_to_user(start,&dato,sizeof(dato))) return -EFAULT;

    pr_info("Start time: %llu\n", (unsigned long long)dato);
    
    return 0;
}

SYSCALL_DEFINE1(get_energy_info,pid_t,pid){
    
   

    
    struct task_struct *task;
  
    unsigned long total_jiffies = jiffies;
  
    unsigned long cpu_usage = 0;
    unsigned long long io_bytes =0;
    unsigned long MemoryUsage=0;
   
    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

 

   

    return 0;
}

// SYSCALL_DEFINE2(get_network_traffic, unsigned long __user *, rx_mb, unsigned long __user *, tx_mb)
// {
//     struct net_device *dev;
//     unsigned long total_rx = 0;
//     unsigned long total_tx = 0;

//     // Iteramos sobre todas las interfaces de red
//     read_lock(&dev_base_lock);
//     for_each_netdev(&init_net, dev) {
//         const struct rtnl_link_stats64 *stats = dev_get_stats(dev, NULL);
//         if (stats) {
//             total_rx += stats->rx_bytes;
//             total_tx += stats->tx_bytes;
//         }
//     }
//     read_unlock(&dev_base_lock);

//     // Convertimos a MB
//     total_rx = total_rx / (1024 * 1024);
//     total_tx = total_tx / (1024 * 1024);

//     // Copiamos los resultados al espacio de usuario
//     if (copy_to_user(rx_mb, &total_rx, sizeof(unsigned long)))
//         return -EFAULT;
//     if (copy_to_user(tx_mb, &total_tx, sizeof(unsigned long)))
//         return -EFAULT;

//     return 0;
// }
```

## Problemas encontrados

1. Error forma undefined reference to __x64___x64_sys_get_cpu_temp'

Este error se puede deber a que no se a declarado bien la syscall en el archivo syscalls_usac.h o en el syscalls_64.tbl,
La syscall debe llevar el mismo nombre en el header y en la tabla.

Tambien puede ser debido a que las syscalls no se compilaron en el kernel. Es importante que la carpeta nueva creada tenga el Makefile con el comando `obj-y := syscalls_usac.o`. Y se debe modificar el Kbuild y agregar `obj-y += syscalls_usac/` al final para especificar que el nuevo directorio se debe compilar.

En caso el archivo syscalls_usac.c este dentro de `kernel/` se debe de especificar en el Makefile dentro de `kernel/` que se tiene compilar ese archivo, agregandole al final del Makefile `obj-y := syscalls_usac.o`.

2. Error tipo Error 2
make: *** [Makefile:224: __sub-make] Error 2

Estos errores son mu ambiguos, para esto sirve tener mas informacion sobre el error, podemos usar el comando `dmesg | tail -50` para obtener mas informacion sobre el error.

En mi caso fue un simple problema de sintaxis, y el comando me dio la informacion

```cpp
    prinfk("Bienvenido a linux usac :D")
        ^
```

3. No agregar la carpeta nueva al Kbuild o al makefile

Este es un error comun el que hace que nuestro codigo nuevo no se compila y puedan dar errores en la compilacion. Para esto, se deben de agregar las instrucciones de compilacion al Makefile al final con `obj-y := syscalls_usac.o` y al Kbuild con `obj-y += syscalls_usac/`.

4. Problemas al mandar datos al espacio de usuario

En el kernel se tiene la regla de que no se pueden mandar apuntadores del kernel al espacio de usuario entonces hay que buscar otras maneras para mandar la informacion.

Solicion uno: Copiar los datos a una estructura del usuario

A la hora de definir los parametros se define una variable en el espacio de usuario y se le agrega __user para especificar que es del espacio de usuario.

```cpp
SYSCALL_DEFINE2(get_startTime_cpu,pid_t,pid,unsigned long long __user *,start)       
```
Luego con la funcion siguiente se copian los datos de una estructura a la estructura de usuario para que el usuario reciba la informacion.

```cpp
if(copy_to_user(start,&dato,sizeof(dato))) return -EFAULT;       
```

Solucion dos: Escribir un archivo con los datos.

Con la funcion filp_open es posible abrir un archivo y obtener el descriptor del archivo.

```cpp
    struct file *file;
    loff_t pos=0;
    //ssize_t bytes_written;
    char buffer[255];

    file =filp_open("/tmp/info_proc_sopes2",O_CREAT | O_TRUNC | O_RDWR,644);
    if(IS_ERR(file)){
        pr_err("No se pudo crear archivo\n");
        return PTR_ERR(file);
    }    
```

Y con el descriptor del archivo es posible escribir datos en el archivo con la siguiente funcion

```cpp
void write_buf(const char *buf, struct file *file, loff_t *pos){
    ssize_t bytes_written = kernel_write(file, buf, strlen(buf),pos);
    if (bytes_written < 0) {
        pr_err("Failed to write file\n");
        filp_close(file, NULL);
        //return bytes_written;
    }
    pr_info("Wrote %zd bytes\n", bytes_written);

}
```

5. Problema al obtener el start_time de un proceso

Al obtener el start_time, este nos puede dar problemas si no conocemos el tipo de dato correcto que necesitamos para obtenerlo, podremos encontrar que el dato de start_time es u64, el cual significa unsigned long long, asi que deberemos de obtener ese dato con una variable de ese tipo como el codigo siguiente:

```cpp
SYSCALL_DEFINE2(get_startTime_cpu,pid_t,pid,unsigned long long __user *,start){

    struct task_struct *task;

    //unsigned long cpu_time;
    unsigned long long dato;
    //struct timespec start;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    //start = task->start_time;

    dato = task->start_time;
    //put_task_struct(task); 
    //cpu_time = (unsigned long)task ->start_time.tv_sec;

    if(copy_to_user(start,&dato,sizeof(dato))) return -EFAULT;

    pr_info("Start time: %llu\n", (unsigned long long)dato);
    
    return 0;
}
```

6. Condiciones de carrera al momento de leer las interfaces de red

Al momento de leer las interfaces de red podemos tener condiciones de carrera porque hay varios hilos usandolos constantemente, asi que necesitaremos usar la funcion read_lock() para evitar estos problemas como en el siguiente codigo.

```cpp
SYSCALL_DEFINE2(get_network_traffic, unsigned long __user *, rx_mb, unsigned long __user *, tx_mb)
{
    struct net_device *dev;
    unsigned long total_rx = 0;
    unsigned long total_tx = 0;

    // Iteramos sobre todas las interfaces de red
    read_lock(&dev_base_lock);
    for_each_netdev(&init_net, dev) {
        const struct rtnl_link_stats64 *stats = dev_get_stats(dev, NULL);
        if (stats) {
            total_rx += stats->rx_bytes;
            total_tx += stats->tx_bytes;
        }
    }
    read_unlock(&dev_base_lock);

    // Convertimos a MB
    total_rx = total_rx / (1024 * 1024);
    total_tx = total_tx / (1024 * 1024);

    // Copiamos los resultados al espacio de usuario
    if (copy_to_user(rx_mb, &total_rx, sizeof(unsigned long)))
        return -EFAULT;
    if (copy_to_user(tx_mb, &total_tx, sizeof(unsigned long)))
        return -EFAULT;

    return 0;
}

```