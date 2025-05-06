# Manual de compilacion de kernel y Cracion de Syscalls

## Primeros Pasos

### Eleccion y descarga de Kernel linux
Primero elejiremos la version del kernel en la que trabajaremos, podemos bajarla facilente desde kernel.org. 

Para la demostracion de este ejemplo se utilizara la longterm actual hasta la fecha que es la 6.12.17.

### Descompresion y creacion de area de trabajo

Luego lo descomprimiremos usando el siguiente comando:

```
sudo tar -xf linux-4.12.17.tar.xz
```

Y lo ultimo que necesitaremos seran las librerias para la compilacion del kernel que podemos obtener corriendo el siguiente comando si se tiene una distribucion basada en ubuntu.

```
sudo apt install build-essential libncurses-dev libssl-dev libelf-dev bison flex -y
```

## Modificacion del kernel 

## Mensaje Bienvenida

Para esto buscaremos el archivo ```init/main.c``` y luego buscaremos la funcion start_kernel() para luego agregar un printk() dentro de esta.

```cpp
void start_kernel(void)
{
	char *command_line;
	char *after_dashes;


	printk("Bienvenido a Usac Linux :D");
```

## Cambiar UTS_SYSNAME

Para esto buscaremos el archivo ```iclude/linux/uts.h``` y modificaremos el valor de UTS_SYSNAME dentro del archivo.

```cpp
/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_UTS_H
#define _LINUX_UTS_H

/*
 * Defines for what uname() should return 
 */
#ifndef UTS_SYSNAME
#define UTS_SYSNAME "USAC Linux"
#endif

#ifndef UTS_NODENAME
#define UTS_NODENAME CONFIG_DEFAULT_HOSTNAME /* set by sethostname() */
#endif

#ifndef UTS_DOMAINNAME
#define UTS_DOMAINNAME "(none)"	/* set by setdomainname() */
#endif

#endif
```

## Agregar Syscall

### Directorio nuevo con codigo syscalls
Crearemos un directorio en donde agregaremos nuestras syscalls, y dentro crearemos el archivo .c que llevara el codigo de las syscalls y tambien un archivo Makefile necesario para compilar las syscalls.

```
mkdir syscalls_usac
cd syscalls_usac/
touch syscalls_usac.c
touch Makefile
```
dentro de syscalls_usac.c ya podemos agregar nuestro codigo, necesitaremos agregar las siguientes librerias.

```cpp
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

```

Luego para agregar una nueva syscall usaremos la palabra reservada SYSCALL_DEFINE#N, donde #N es el numero de argumentos que se le pasaran a la syscall.

Luego dentro de () agregaremos el nombre de la syscall, y luego los parametros que van con la nomenclatura tipo,nombre como e el siguiente ejemplo:

```cpp
SYSCALL_DEFINE1(get_time_cpu,pid_t,pid){

    struct task_struct *task;

    unsigned long cpu_time;

    task = pid_task(find_vpid(pid),PIDTYPE_PID);

    if(!task) return -ESRCH;

    cpu_time = task ->utime + task -> stime;
    do_div(cpu_time,1000000);
    return cpu_time;
}
```

Este ejemplo es una syscall que toma un pid y mide el tiempo que pasa el procesador en el proceso.

Y las demas syscalls van una detras de otra, se agrego al ejemplo tambien las siguientes syscalls:

```cpp
SYSCALL_DEFINE0(get_cpu_temp) {
    struct file *file;
    char buf[16];
    loff_t pos = 0;
    long temp;
    int ret;

    file = filp_open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY, 0);
    if (IS_ERR(file)) 
        return PTR_ERR(file);

    ret = kernel_read(file, buf, sizeof(buf) - 1, &pos);
    filp_close(file, NULL);

    if (ret < 0)
        return ret;

    printk("Pruebas temperatura");

    buf[ret] = '\0'; 
    kstrtol(buf, 10, &temp);
    
    return 0044; 
}
```
Ahora que ya tenemos el codigo de las syscalls escrito debemos de escribir la siguiente instruccion en el Makefile para que se logre compilar junto al kernel.

```
obj-y := syscalls_usac.o
```

### Agregar syscalls a Header

Ahora debemos hacer el .h de las syscalls, para esto nos iremos a include/linux/ y adentro crearemos el archivo syscalls_usac.h

```
touch syscalls_usac.h
```

En este archivo debemos de definir nuestras syscalls, las definiremos de la siguiente manera :

```cpp
#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>

asmlinkage long get_time_cpu(pid_t pid);
asmlinkage long wait_key(int key);
asmlinkage long get_cpu_temp(void);


#endif
```

### Agregar syscalls a tabla de syscalls

Dependiendo de la arquitectura de nuestro procesador si es de x32 o x64, deberemos modificar cierto archivo .tbl , este se encuentra en ```arch/x86/entry/syscalls/``` , en esta carpeta estara syscalls_64.tbl y syscalls_32.tbl .

Modificaremos la tabla dependiendo de la arquitectura de nuestro procesador.

Y en la tabla agregaremos nuestras syscalls con la siguiente nomenclatura al final del archivo:

```
336 common  get_time_cpu    sys_get_time_cpu
337 common  wait_key        sys_wait_key
338 common  get_cpu_temp    sys_get_cpu_temp  
```

Es importante que los numeros clave de las syscalls no se repitan y estos no esten dentro de los rangos que no se pueden utilizar, En el archivo hay ciertos comentarios que daran la informacion necesaria para saber que rango de numeros usar.

## Agregar carpeta nueva al KBUILD

En el directorio raiz buscaremos la carpeta Kbuild, en esta agregaremos las instrucciones para que el kernel sepa que tiene que compilar la nueva carpeta creada.

```
obj-y 			+= syscalls_usac/
```

## Compilacion del kernel

Primero deberemos crear una configuracion para poder compilar el kernel con el siguiente comando:

```
make localmodconfig
```

Luego deshabilitaremos las llaves privadas con los siguientes comandos:

```
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS
```
Luego pasaremos a compilar el kernel, con la bandera j#N
le podemos asignar cuantos nucleos de nuestro procesador queremos que consuma:

```
fakeroot make -j2
```

Para saber si el kernel se compilo con exito, podemos usar el siguiente comando el cual debe devolver 0 para saber que no ha habido problema.

```
echo $?
```

y por ultimo instalaremos el kernel con los siguientes comandos:

```
sudo make modules_install
sudo make install
sudo reboot
```

## Seleccion del kernel desde el GRUB

Ahora necesitaremos reiniciar la computadora para ingresar al grub, cuando se este reiniciando, si estamos en maquina virtual apretaremos `shift+esc`.

Esto abrira un menu en el cual podremos elegir nuestro sistema operativo, o opciones avanzadas. Nos iremos a opciones avanzadas.

Luego eligiremos la version del kernel que compilamos, y listo.

## Ver UTS_SYSNAME modificado

con el comando `uname -r` podremos ver el UTS_SYSNAME modificado

## Ver mensaje en kernel

Podremos ver el mensaje con el siguiente codigo

```cpp
dmesg | grep "Bienvenido a Usac Linux :D"
```
## Prueba nuevas syscalls

para probar nuestras nuevas syscalls, podemos probarlas facilmente con la funcion `syscall(#n,arg)` donde #n es el numero de syscall que definimos en la tabla de syscalls syscalls_64.tbl o syscalls_32.tbl, y luego le agregaremos los argumentos despues de una coma si es que la syscall necesita argumentos.

```cpp

void test_get_time_cpu() {
    pid_t pid;
    long cpu_time;

    printf("Ingrese el PID del proceso: ");
    scanf("%d", &pid);

    cpu_time = syscall(336, pid);

    if (cpu_time < 0) {
        perror("Error al obtener el tiempo de CPU");
    } else {
        printf("Tiempo de CPU usado por el proceso %d: %ld ms\n", pid, cpu_time);
    }
}
```

## Problemas Frecuentes

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


