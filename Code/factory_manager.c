/*
 *
 * factory_manager.c
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stddef.h>
 #include <semaphore.h>
 #include <sys/stat.h>
 #include <pthread.h>
 #include "factory_manager.h"
 #include "process_manager.h"
 
// Declaración de todos lo mutex, condiciones y variables
 pthread_mutex_t mutex_factory_creacion;
 pthread_mutex_t mutex_factory_inicializacion;
 pthread_cond_t cond_inicializacion;
 int id_turno;
 int termine;
 pthread_cond_t cond_fin;
 pthread_mutex_t mutex_factory_control;
 pthread_mutex_t mutex_fin_completo;
pthread_mutex_t mutex_inicializacion_completa;
pthread_cond_t  cond_inicializacion_completa;
 
 
 int main(int argc, const char * argv[] ){
    id_turno = 0;
    termine = 0;
    // Comprobamos si el número de argumentos pasados al main es el correcto
    if (argc != 2){
        fprintf(stderr,"[ERROR][factory_manager] Invalid file.\n");
        return -1;
    }

    // Abrimos el fichero de lectura 
    pid_t fichero_lectura = open(argv[1], O_RDONLY);
    if (fichero_lectura < 0){
        fprintf(stderr,"[ERROR][factory_manager] Invalid file.\n");
        return -1;
    }
    
    // Leemos la primera entrada del fichero de lectura
    char buffer[2];
    if (read(fichero_lectura, &buffer, 1) == 0){
        fprintf(stderr,"[ERROR][factory_manager] Invalid file.\n");
        return -1;
    }
    buffer[1]= '\0';
    int Max_cintas = atoi(buffer);

    // Comprobamos que la primera entrada sea válida
    if(Max_cintas == 0){
        fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
        return -1;
    }

    lseek(fichero_lectura, +1, SEEK_CUR);

    struct Cinta mi_cinta[Max_cintas];
    char digito;
    char entrada[1024];
    int iteracion = 0;
    int lista_ID[Max_cintas];
    int pos = 0;
    int parametro = 1;

    // Bucle de lectura y guardado de todo el resto de parámetros en una estructura para su fácil manejo
    while (read(fichero_lectura, &digito, 1) != 0){
        if (iteracion >= Max_cintas){
            fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
            return -1;
        }
        if (digito == ' ' && parametro == 1) {
            entrada[pos] = '\0';
            mi_cinta[iteracion].ID_cinta = atoi(entrada);
            pos = 0;
            entrada[0] = '\0';
            parametro ++;
        }else if (digito == ' ' && parametro == 2) {
            entrada[pos] = '\0';
            mi_cinta[iteracion].Tamaño_cinta = atoi(entrada);
            pos = 0;
            entrada[0] = '\0';
            parametro ++;
        }else if (digito == ' ' && parametro == 3) {
            entrada[pos] = '\0';
            mi_cinta[iteracion].N_elementos = atoi(entrada);
            lista_ID[iteracion] = mi_cinta[iteracion].ID_cinta;
            pos = 0;
            entrada[0] = '\0';
            parametro = 1;
            iteracion ++;
        } else if (digito != ' '){
            entrada[pos] = digito;
            pos ++;
        }    
    }
    entrada[pos] = '\0';
    mi_cinta[iteracion].N_elementos = atoi(entrada);
    lista_ID[iteracion] = mi_cinta[iteracion].ID_cinta;
    pos = 0;
    entrada[0] = '\0';
    if (parametro != 3)
    {
        fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
        return -1;
    }
    parametro = 1;
    iteracion ++;

    close(fichero_lectura);

    // Inicialización de todos los mutex y condiciones
    pthread_mutex_init(&mutex_factory_creacion, NULL);
    pthread_mutex_init(&mutex_factory_inicializacion, NULL);
    pthread_mutex_init(&mutex_factory_control, NULL);
    pthread_cond_init(&cond_inicializacion, NULL);
    pthread_mutex_init(&mutex_inicializacion_completa, NULL);
    pthread_mutex_init(&mutex_fin_completo, NULL);
    pthread_cond_init(&cond_inicializacion_completa, NULL);
    pthread_cond_init(&cond_fin, NULL);

    pthread_t hilos[iteracion];
    pthread_mutex_lock(&mutex_factory_creacion);

    // Creación de todos los hilos "process_manager" de forma ordenada
    for (int i = 0; i < iteracion; i++) {
        if (pthread_create(&hilos[i], NULL, (void *)process_manager, &mi_cinta[i]) != 0) {
            fprintf(stderr, "[ERROR][factory_manager] Process_manager with id %d has finished with errors.\n", mi_cinta[i].ID_cinta);
        }
        printf("[OK][factory_manager] Process_manager with id %d has been created.\n", mi_cinta[i].ID_cinta);
    }
    pthread_mutex_unlock(&mutex_factory_creacion);
    
    // Inicialización de los hilos de forma ordenada conforme a su creación   
    pthread_mutex_lock(&mutex_factory_control);
    for (int i = 0; i < iteracion; i++){
        pthread_mutex_lock(&mutex_factory_inicializacion);
        id_turno = lista_ID[i];
        pthread_mutex_lock(&mutex_inicializacion_completa);
        pthread_cond_broadcast(&cond_inicializacion);
        pthread_mutex_unlock(&mutex_factory_inicializacion);

        // Bucle con condición para esperar que el hilo correspondiente confirme su inicialización
        while(termine == 0){
            pthread_cond_wait(&cond_inicializacion_completa, &mutex_inicializacion_completa);
        }
        termine = 0;
        pthread_mutex_unlock(&mutex_inicializacion_completa);
    }

    id_turno = 0;
    termine = 0;
    // Ejecución y recogida de los hilos
    pthread_mutex_unlock(&mutex_factory_control);
    for (int i = 0; i < iteracion; i++){
        pthread_mutex_lock(&mutex_factory_control);
        id_turno = lista_ID[i];
        pthread_mutex_unlock(&mutex_factory_control);
        pthread_cond_broadcast(&cond_inicializacion);
        pthread_mutex_lock(&mutex_fin_completo);
        
        // Bucle con condición para esperar que el hilo correspondiente confirme que ha terminado
        while (termine == 0)
        {
            pthread_cond_wait(&cond_fin, &mutex_fin_completo);
        }
        termine = 0;
        pthread_mutex_unlock(&mutex_fin_completo);

        // Recogida de la finalización del hilo
        void* final;
        pthread_join(hilos[i], &final);
        if (final != 0){
            fprintf(stderr,"[ERROR][factory_manager] Process_manager with id %d has finished with errors.\n", id_turno);
        } 
        else {
            printf("[OK][factory_manager] Process_manager with id %d has finished.\n", id_turno);
        }       
    }
 
    // Eliminación de todos los mutex y condiciones utilizadas
    pthread_mutex_destroy(&mutex_factory_creacion);
    pthread_mutex_destroy(&mutex_factory_control);
    pthread_mutex_destroy(&mutex_factory_inicializacion);
    pthread_cond_destroy(&cond_inicializacion);
    pthread_mutex_destroy(&mutex_inicializacion_completa);
    pthread_cond_destroy(&cond_inicializacion_completa);

    printf("[OK][factory_manager] Finishing.\n");

    return 0;
 }
 