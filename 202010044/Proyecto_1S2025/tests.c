#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdlib.h>





// void get_cpu_temperature()
// {
//          long amma = syscall(338);
//          printf("Temperatura %ld\n", amma);
         
// }

// void test_get_time_cpu() {
//     pid_t pid;
//     long cpu_time;

//     printf("Ingrese el PID del proceso: ");
//     scanf("%d", &pid);

//     cpu_time = syscall(336, pid);

//     if (cpu_time < 0) {
//         perror("Error al obtener el tiempo de CPU");
//     } else {
//         printf("Tiempo de CPU usado por el proceso %d: %ld ms\n", pid, cpu_time);
//     }
// }


void create_list_processes(){
    int a= syscall(339);

    if(a!=0){
        printf("Error al crear lista procesos");
    }
    printf("Lista procesos creados");
}

void kill_process() {
    pid_t pid;
    

    printf("Ingrese el PID del proceso: ");
    scanf("%d", &pid);

    int a= syscall(340, pid);

    if(a!=0){
        printf("Error al matar el proceso");
    }
    printf("Proceso muerto");
}

void stop_Process() {
    pid_t pid;
    

    printf("Ingrese el PID del proceso: ");
    scanf("%d", &pid);

    int a= syscall(341, pid);

    if(a!=0){
        printf("Error al parar el proceso");
    }
    printf("Proceso parado");
}

void continue_Process() {
    pid_t pid;
    

    printf("Ingrese el PID del proceso: ");
    scanf("%d", &pid);

    int a= syscall(342, pid);

    if(a!=0){
        printf("Error al continuar el proceso");
    }
    printf("Proceso continuado");
}

void get_start_time_process() {
    pid_t pid;
    unsigned long long dato;
    

    printf("Ingrese el PID del proceso: ");
    scanf("%d", &pid);

    int a= syscall(343, pid,&dato);

    if(dato){
        printf("Start time %llu\n",dato);
    }else{
        printf("Error al obtener Start time");
    }
}

void get_energy_process() {
    pid_t pid;
    unsigned long dato;
    

    printf("Ingrese el PID del proceso: ");
    scanf("%d", &pid);

    dato = syscall(344, pid);

    printf("Energia %lu\n",dato);
}

void get_network(){
    unsigned long tx;
    unsigned long rx;

    syscall(345,&rx,&tx);

    printf("Rx : %lu Tx: %lu",rx,tx);

}

int main(){

    
    int opcion;

    while (1) {
        printf("\n===== Menú de Syscalls =====\n");
        printf("1. Lista Procesos\n");
        printf("2. Matar Proceso\n");
        printf("3. Parar Proceso\n");
        printf("4. Continuar Proceso\n");
        printf("5. Obtener start_time\n");
        printf("6. Obtener Energia Proceso\n");
        printf("7. Obtener Datos Trafico\n");
        printf("8. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                create_list_processes();
                break;
            case 2:
                kill_process();
                break;
            case 3:
                stop_Process();
                break;
            case 4:
                continue_Process();
                break;
            case 5:
                get_start_time_process();
                break;
            case 6:
                get_energy_process();
                break;
            case 7:
                get_network();
                break;
            case 8:
                printf("Saliendo");
                exit(0);
            default:
                printf("Opción no válida. Intente de nuevo.\n");
        }
    }
    return 0;
}