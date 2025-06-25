/*
 *
 * process_manager.c
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stddef.h>
 #include <pthread.h>
 #include "queue.h"
 #include "factory_manager.h"
 #include "process_manager.h"
 #include <semaphore.h>
  
 #define NUM_THREADS 2

// Declaración de los mutex, condiciones y variables 
circular_queue mi_cola_circular;
int turno_lp = 0;

pthread_mutex_t mutex_process;
pthread_cond_t cond_lectura;
pthread_cond_t cond_escritura;
int fin_escritura;
int items_a_producir = 0;
 
 // Función utilizada para el hilo escritor
 int escritor(int* items_to_produce){
    turno_lp = 0;
    for (int i = 0; i < *items_to_produce; i++){
        pthread_mutex_lock(&mutex_process);
        struct element elemento;
        elemento.id_belt = mi_cola_circular.id;
        elemento.num_edition = i;
        if (i == *items_to_produce-1){
            elemento.last = 1;
            fin_escritura = 1;
        } else{
            elemento.last = 0;
        }
        
        queue_put(&elemento, &mi_cola_circular);

        if (queue_full(&mi_cola_circular) == 1 && fin_escritura == 0){
            turno_lp = 1;
            pthread_cond_signal(&cond_lectura);
            while (turno_lp != 0){
                pthread_cond_wait(&cond_escritura, &mutex_process);
            }
        }
        pthread_mutex_unlock(&mutex_process);
    }
    turno_lp = 1;
    pthread_mutex_unlock(&mutex_process);
    pthread_cond_signal(&cond_lectura);
    return 0;
}

// Función utilizada para el hilo consumidor
int consumidor(int* items_to_produce){
    for (int i = 0; i < *items_to_produce; i++){
        pthread_mutex_lock(&mutex_process);
        while (turno_lp == 0) {
            pthread_cond_wait(&cond_lectura, &mutex_process);
        }
        if (turno_lp == 1 && fin_escritura == 0){
            queue_get(&mi_cola_circular);
            turno_lp = 0;
            pthread_cond_signal(&cond_escritura);
        } else if (turno_lp == 1 && fin_escritura == 1){
            queue_get(&mi_cola_circular);
        }  
        pthread_mutex_unlock(&mutex_process);
    }
    pthread_mutex_unlock(&mutex_process);
    fin_escritura = 0;
    turno_lp =0;
    return 0;
}
  
// Función "process_manager" que ejecuta cada hilo creado por "factory_manager"
int  process_manager (struct Cinta *mi_cinta){
    // Obtención de los parámetros pasados mediante la estructura
    int id = mi_cinta->ID_cinta;
    int belt_size = mi_cinta->Tamaño_cinta;
    int items_to_produce = mi_cinta->N_elementos;
    
    // Bucle de espera para el turno de inicialización de cada hilo
    pthread_mutex_lock(&mutex_factory_inicializacion);
    while (id != id_turno){
        pthread_cond_wait(&cond_inicializacion, &mutex_factory_inicializacion);
    }

    pthread_t threads[NUM_THREADS];
    int rc;
    long t;

    pthread_mutex_unlock(&mutex_factory_inicializacion);
    // Señalización de un hilo inicializado
    pthread_mutex_lock(&mutex_inicializacion_completa);
    termine = 1;
    pthread_cond_signal(&cond_inicializacion_completa);
    pthread_mutex_unlock(&mutex_inicializacion_completa);
    printf("[OK][process_manager] Process_manager with id %d waiting to produce %d elements.\n", id, items_to_produce);	
    pthread_mutex_unlock(&mutex_factory_inicializacion);
    
    // Bucle de espera para el turno de ejecución de cada hilo
    pthread_mutex_lock(&mutex_factory_control);
    while (id != id_turno){
        pthread_cond_wait(&cond_inicializacion, &mutex_factory_control);
    }
    
    // Inicialización de cada mutex y condición
    pthread_mutex_init(&mutex_process, NULL);
    pthread_cond_init(&cond_lectura, NULL);
    pthread_cond_init(&cond_escritura, NULL);

    // Inicialización de la cola circular
    queue_empty(&mi_cola_circular);
    queue_init(belt_size, id, &mi_cola_circular);

    // Comprobación de la validez de los argumentos
    if (belt_size <= 0 || items_to_produce <= 0){
        fprintf(stderr,"[ERROR][process_manager] Arguments not valid.\n");
        termine =1;
        pthread_mutex_unlock(&mutex_factory_control);
        pthread_cond_signal(&cond_fin);
        pthread_mutex_destroy(&mutex_process);
        return -1;
    }

    printf("[OK][process_manager] Belt with id %d has been created with a maximum of %d elements.\n", id, belt_size);
    pthread_mutex_unlock(&mutex_factory_control);

    // Creación del escritor y el consumidor
    for (t=0;t<NUM_THREADS;t++){
        if (t==0){
            rc = pthread_create(&threads[t], NULL, (void *)escritor, &items_to_produce);
        }
        else{
            rc = pthread_create(&threads[t], NULL, (void *)consumidor, &items_to_produce);
        }
            
        if (rc){
            fprintf(stderr,"[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_turno);
            return -1;
            }
        }

    // Recogida del escritor y el consumidor
    int i = 0;
    for (i = 0; i < NUM_THREADS; i++){	
        void* final;
        pthread_join(threads[i], &final);
        if (final!= 0){
            fprintf(stderr,"[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_turno);
            return -1;
        }
    }
    printf("[OK][process_manager] Process_manager with id %d has produced %d elements.\n", id, items_to_produce);
    // Señalización de que el hilo ha terminado
    termine =1;
    pthread_cond_signal(&cond_fin);
    // Eliminación de los mutex y condiciones usadas
    pthread_mutex_destroy(&mutex_process);
    pthread_mutex_unlock(&mutex_factory_control);
    pthread_cond_destroy(&cond_escritura);
    pthread_cond_destroy(&cond_lectura);

    return 0; 
 }