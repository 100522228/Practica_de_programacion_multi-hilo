#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"
#include <pthread.h>

//Creación e inicialización de una cola circular
int queue_init(int size, int id, circular_queue *mi_cola){
    if (size <= 0){
        fprintf(stderr, "[ERROR][queue] There was an error while using queue with id: %d.\n", 
			id);
		return -1;
    }

    mi_cola->size = size;
    mi_cola->head = 0;
    mi_cola->tail = 0;
    mi_cola->count = 0;
	mi_cola->id = id;
    mi_cola->buffer = malloc(sizeof(struct element) * size);

    return 0;
}

// Función para añadir un nuevo elemento a la cola
int queue_put(struct element* x, circular_queue *mi_cola) {
	if (mi_cola->size == mi_cola->count){
		fprintf(stderr, "[ERROR][queue] There was an error while using queue with id: %d.\n", mi_cola->id);
		return -1;
	}
	mi_cola->buffer[mi_cola->tail] = *x;
	mi_cola->tail = (mi_cola->tail +1)%mi_cola->size;
	mi_cola->count++;
	printf("[OK][queue] Introduced element with id %d in belt %d.\n",x->num_edition , mi_cola->id);
    return 0;
}

// Función para sacar un elemento de la cola
struct element queue_get(circular_queue *mi_cola) {
	int cabeza = mi_cola->head;
	struct element elem=  mi_cola->buffer[cabeza];
	mi_cola->head = (mi_cola->head +1)%mi_cola->size;
	mi_cola->count --;
	printf("[OK][queue] Obtained element with id %d in belt %d.\n", elem.num_edition, elem.id_belt);
    return elem;
}

// Función para comprobar si la cola esta vacía
int queue_empty(circular_queue *mi_cola){
	if (mi_cola->count != 0){
		return 1;
	}
    return 0;
}

// Función para comprobar si la cola esta llena
int queue_full(circular_queue *mi_cola){
	if (mi_cola->count == mi_cola->size){
		return 1;
	}
    return 0;
}

// Función para eliminar y liberar el espacio de la cola
int queue_destroy(circular_queue *mi_cola){
	mi_cola->head = mi_cola->tail = mi_cola->count = mi_cola->size= 0;
	free(mi_cola->buffer);
	mi_cola->buffer = NULL;
    return 0;
}
