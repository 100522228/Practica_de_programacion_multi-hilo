#ifndef HEADER_FILE
#define HEADER_FILE

typedef struct {
  int id;                   // Id de la cinta actual
  int size;                 // Tamaño máximo
  int head;                 // Índice de cabeza (extracción)
  int tail;                 // Índice de cola (inserción)
  int count;                // Número actual de elementos
  struct element *buffer;   // Lista circular
} circular_queue;

struct element {
  int num_edition;
  int id_belt;
  int last;
};

int queue_init (int size, int id, circular_queue *mi_cola);
int queue_destroy (circular_queue *mi_cola);
int queue_put (struct element* elem, circular_queue *mi_cola);
struct element queue_get(circular_queue *mi_cola);
int queue_empty (circular_queue *mi_cola);
int queue_full(circular_queue *mi_cola);

#endif