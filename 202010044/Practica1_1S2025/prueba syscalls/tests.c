#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdlib.h>





void get_cpu_temperature()
{
         long amma = syscall(338);
         printf("Temperatura %ld\n", amma);
         
}

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

int main(){
    int opcion;

    while (1) {
        printf("\n===== Menú de Syscalls =====\n");
        printf("1. Obtener tiempo de CPU\n");
        printf("2. Temperatura\n");
        printf("3.Prueba Tecla\n");
        printf("4. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                test_get_time_cpu();
                break;
            case 2:
                get_cpu_temperature();
                break;
            case 3:
                printf("leer tecla");
                break;
            case 4:
                printf("Saliendo");
                exit(0);
            default:
                printf("Opción no válida. Intente de nuevo.\n");
        }
    }
    return 0;
}