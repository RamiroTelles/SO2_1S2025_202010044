# Diseno sobre Syscall que sincroniza Archivos De arpetas

## Librerias clave utilizadas

Se ha hecho uso de inotify para la recopilacion de eventos de creacion, modificacion y eliminacion. Hilos de kernel para la sincronizacion de ambas carpetas, y mutex para evitar condiciones de carrera y problemas de sincronizacion entre cada hilo.

## Funciones Clave del funcionamiento

### Los signal Handlers

```cpp
void handle_sigint(int sig){
    if(keep_running==0){
        exit(EXIT_FAILURE);
    }
    keep_running=0;
    //kthread_stop(kthread_1);
    //kthread_stop(kthread_2);

}

void setup_signal_handler(){
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;

    if(sigaction(SIGINT,&sa,NULL)==-1){
        printf("Error crear manejador de senales");
        exit(EXIT_FAILURE);
    }
}
```

Se tratan de 2 funciones las cuales manejan la senal que se manda a un proceso al presionar ctrl+c, esto hace posible que despues de la sincronizacion, se haga limpieza de memoria y se cierren los archivos acordes.

### Funiones para crear,modificar y eliminar archivos

```cpp
void create_file(const char *pathFile){

    FILE *fd = fopen(pathFile,"w");
    if(fd==NULL){
        printf("Error al crear archivo %s\n",pathFile);
        exit(EXIT_FAILURE);
    }
    fclose(fd);
    printf("Archivo creado exitosamente: %s\n", pathFile);
}

void delete_file(const char *pathFile){

    if(unlink(pathFile)==-1){
        printf("Error al eliminar archivo");
        exit(EXIT_FAILURE);
    }

    printf("Se elimino archivo en %s\n",pathFile);

}

void modify_file(const char *ref,const char *dest){

    FILE *fd1 = fopen(ref,"r");

    if(fd1==NULL){
        printf("Error al abrir archivo %s\n",ref);
        exit(EXIT_FAILURE);
    }
    char buff[100];
    FILE *fd2 = fopen(dest,"w");
    if(fd2==NULL){
        printf("Error al abrir archivo %s\n",dest);
        exit(EXIT_FAILURE);
        
    }

    while(fgets(buff,sizeof(buff),fd1)){
        if(fputs(buff,fd2)<0){
            printf("Error al escribir en archivo modificado");
            exit(EXIT_FAILURE);
        }
    }

    fclose(fd1);
    fclose(fd2);
}

```

Funciones que se encargan de crear,eliminar o modificar los archivos en el directorio destino.

### Funcion listen_folder1

Esta funcion es la que hace posible la sincronizacion de los 2 directorios, recibe como parametro el directorio base y el destino.

Se basa en la idea de que un hilo recibira un directorio referencia y uno destino, y los eventos que pasen en referencia, los replicara en destino.

Esto puede dar ciertos problemas con la sincronizacion, y la solucion se habla mas adelante en problemas encontrados.

```cpp
void* listen_folder1(void *args){
    int fd,wd;
    char buff[BUF_LEN];

    path_struct *paths = (path_struct *) args; 

    fd = inotify_init();
    if(fd<0){
        printf("Error al iniciar inotify");
        exit(EXIT_FAILURE);
    }

    //wd = inotify_add_watch(fd,path1, IN_MODIFY| IN_CREATE|IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    wd = inotify_add_watch(fd,paths->path1, IN_MODIFY| IN_CREATE|IN_DELETE);

    if(wd<0){
        printf("Error anadir watch");
    }

    while(1){
        int length = read(fd,buff,BUF_LEN);
        if(!keep_running){
            break;
        }
        if(length<0){
            
            printf("Error al leer evento");
            break;
        }

        for(int i=0;i<length;){
            struct inotify_event *event = (struct inotify_event *)&buff[i];
            printf("--------------------------\n");
            printf("evento: %s",event->name);
            printf("wd: %d\n", event->wd);

            printf("mask: %u (", event->mask);
            if (event->mask & IN_ACCESS)        printf("IN_ACCESS ");
            if (event->mask & IN_MODIFY)       printf("IN_MODIFY ");
            if (event->mask & IN_ATTRIB)       printf("IN_ATTRIB ");
            if (event->mask & IN_CLOSE_WRITE)  printf("IN_CLOSE_WRITE ");
            if (event->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
            if (event->mask & IN_OPEN)         printf("IN_OPEN ");
            if (event->mask & IN_MOVED_FROM)   printf("IN_MOVED_FROM ");
            if (event->mask & IN_MOVED_TO)     printf("IN_MOVED_TO ");
            if (event->mask & IN_CREATE)       printf("IN_CREATE ");
            if (event->mask & IN_DELETE)       printf("IN_DELETE ");
            if (event->mask & IN_DELETE_SELF)  printf("IN_DELETE_SELF ");
            if (event->mask & IN_MOVE_SELF)    printf("IN_MOVE_SELF ");
            if (event->mask & IN_ISDIR)        printf("IN_ISDIR ");
            if (event->mask & IN_UNMOUNT)      printf("IN_UNMOUNT ");
            if (event->mask & IN_Q_OVERFLOW)   printf("IN_Q_OVERFLOW ");
            if (event->mask & IN_IGNORED)      printf("IN_IGNORED ");
            printf(")\n");
            
            printf("cookie: %u\n", event->cookie);
            
            
            printf("len: %u\n", event->len);
            
                    
            printf("\n-----------------------------\n");

            if(event->len){
                if(event->mask & IN_CREATE){

                    if(!(event->name[0]=='.')){

                        //agregar lock

                        if(sinc==0){
                            mutex_lock(&lock);
                            sinc=1;
                            char *combined_path = malloc(strlen(paths->path2) + strlen(event->name) + 2);
                            if (!combined_path) {
                                printf("Error malloc create");
                                exit(EXIT_FAILURE);
                            }

                            sprintf(combined_path, "%s/%s", paths->path2, event->name);

                            create_file(combined_path);
                            
                            printf("Crear %s\n",event->name);
                            printf("en ruta%s\n",paths->path2);

                            //quitar lock
                            free(combined_path);
                            mutex_unlock(&lock);
                        }else{
                            sinc=0;
                        }

                    }else{
                        printf("Archivo oculto %s\n",event->name);
                        printf("en ruta %s\n",paths->path1);
                    }

                        
                    
                }
                else if(event->mask & IN_MODIFY){
                    if(!(event->name[0]=='.')){

                        //agregar lock
                        if(sinc==0){
                            mutex_lock(&lock);
                            sinc=1;
                            char *combined_path_ref = malloc(strlen(paths->path1) + strlen(event->name) + 2);
                        
                            if (!combined_path_ref) {
                                printf("Error malloc create");
                                exit(EXIT_FAILURE);
                            }
                            char *combined_path_dest = malloc(strlen(paths->path2) + strlen(event->name) + 2);
                            if (!combined_path_dest) {
                                printf("Error malloc create");
                                exit(EXIT_FAILURE);
                            }

                            sprintf(combined_path_ref, "%s/%s", paths->path1, event->name);
                            sprintf(combined_path_dest, "%s/%s", paths->path2, event->name);

                            modify_file(combined_path_ref,combined_path_dest);
                            
                            printf("Modificar %s\n",event->name);
                            printf("en ruta%s\n",paths->path2);

                            //quitar lock
                            free(combined_path_ref);
                            free(combined_path_dest);
                            mutex_unlock(&lock);
                        }else{
                            sinc=0;
                        }

                        
                    }else{
                        printf("Archivo oculto %s\n",event->name);
                        printf("en ruta %s\n",paths->path1);
                    }
                }
                else if(event->mask & IN_DELETE){
                    if(!(event->name[0]=='.')){

                        //agregar lock
                        if(sinc==0){
                            mutex_lock(&lock);
                            sinc=1;
                            char *combined_path = malloc(strlen(paths->path2) + strlen(event->name) + 2);
                            if (!combined_path) {
                                printf("Error malloc create");
                                exit(EXIT_FAILURE);
                            }

                            sprintf(combined_path, "%s/%s", paths->path2, event->name);

                            delete_file(combined_path);
                            
                            printf("Eliminar %s\n",event->name);
                            printf("en ruta%s\n",paths->path2);

                            //quitar lock


                            free(combined_path);
                            mutex_unlock(&lock);

                        }else{
                            sinc=0;
                        }
                        
                    }else{
                        printf("Archivo oculto %s\n",event->name);
                        printf("en ruta %s\n",paths->path1);
                    }
                }

            }

            i+= EVENT_SIZE + event->len;
        }

        
    }
    printf("Limpiando....");
    inotify_rm_watch(fd,wd);
    //pthread_mutex_destroy(&lock);
    close(fd);
    exit(EXIT_SUCCESS);

}
```

### Solucion problemas con condicion de Carrera y sincronizacion

Como podemos ver en la siguiente porcion de codigo, se utilizo una flag sinc para avisar al otro hilo si la carpeta destino se ha modificado por el hilo, ya si esto no se controla, podria provocar un loop infinito.

Tambien se utilizaron mutex a la hora de modificar la bandera sinc y tambien al crear,eliminar o modificar archivos en la carpeta destino para evitar condiciones de carrera.

```cpp


//agregar lock
if(sinc==0){
    mutex_lock(&lock);
    sinc=1;
    char *combined_path = malloc(strlen(paths->path2) + strlen(event->name) + 2);
    if (!combined_path) {
        printf("Error malloc create");
        exit(EXIT_FAILURE);
    }

    sprintf(combined_path, "%s/%s", paths->path2, event->name);

    delete_file(combined_path);
                            
    printf("Eliminar %s\n",event->name);
    printf("en ruta%s\n",paths->path2);

                            //quitar lock


    free(combined_path);
    mutex_unlock(&lock);

}else{
    sinc=0;
}
                        
                    
```

### Funcion principal de la syscall

Esta funcion se encarga de recibir los parametros y pasarselos a los 2 hilos, los cuales, cada uno monitoreara una carpeta y escribira e la otra.

```cpp
SYSCALL_DEFINE2(sincr_folders, const char *,path1,const char *, path2){
    setup_signal_handler();

    //listen_folder1("/home/rami/Documents/f1","/home/rami/Documents/f2");

    //pthread_t threads[2];

    mutex_init(&lock);

    path_struct args1 = {path1,path2};
    path_struct args2 = {path2,path1};

    //pthread_create(&threads[0],NULL,listen_folder1,&args1);
    //pthread_create(&threads[1],NULL,listen_folder1,&args2);

    // Esperar a que los hilos terminen (en este caso no deberían terminar)
    //pthread_join(threads[0], NULL);
    //pthread_join(threads[1], NULL);
    kthread_1 = kthread_run(listen_folder1,&args1,"kthread_1");
    kthread_2 = kthread_run(listen_folder1,&args2,"kthread_2");

    if(kthread_1 ==NULL || kthread_2==NULL){
        printf("Hilos fallan al iniciarse");
        return -1;
    }

    if(kthread_1){
        kthread_stop(kthread_1);
    }
    if(kthread_2){
        kthread_stop(kthread_2);
    }

    //pthread_mutex_destroy(&lock);
    
    print("...");
    return 0;

}

```

## Problemas encontrados

### Problemas al modificar archivos con nano

Dependiendo de que programa utilizemos para modificar un archivo, este manejara la modificacion de distintas maneras, unos programas mueven unos archivos o los renombran, y esto hace muy dificil tener una idea clara de que archivo se modifica y que se modifica.

En el caso de nano, puede ser que el editor de codigo cree archivos temporales, los cuales pueden hacer que se creen archivos de mas en la sincronizacion.

Solucion:

Normalmente los archivos temporales son ocultos al usuario y empiezan por un '.', entonces se ignoran todos los eventos donde el archivo empieza con punto.
```cpp
if(!(event->name[0]=='.')){
```

### Problema con sincronizacion de hilos

El codigo se basa en que un hilo si encuentra cambios en una carpeta, los replica en la otra, entonces esto puede dar el caso en que un hilo modifique un folder, y el otro hilo al detectar el cambio, modificara el primer folder nuevamente, creando un loop infinito.

Para esto se utilizo una variable global la cual se utiliza como bandera para que cada hilo sepa cual modificacion fue hecha por el hilo, y cual por el usuario.

Se utiliza un mutex antes de cambiar la variable y hacer la modificacion correspondiente para evitar condiciones de carrera.

```cpp


//agregar lock
if(sinc==0){
    mutex_lock(&lock);
    sinc=1;
    char *combined_path = malloc(strlen(paths->path2) + strlen(event->name) + 2);
    if (!combined_path) {
        printf("Error malloc create");
        exit(EXIT_FAILURE);
    }

    sprintf(combined_path, "%s/%s", paths->path2, event->name);

    delete_file(combined_path);
                            
    printf("Eliminar %s\n",event->name);
    printf("en ruta%s\n",paths->path2);

                            //quitar lock


    free(combined_path);
    mutex_unlock(&lock);

}else{
    sinc=0;
}
                        
                    
```

