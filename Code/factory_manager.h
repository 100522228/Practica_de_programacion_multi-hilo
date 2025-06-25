#include <pthread.h>
#include <semaphore.h>

struct Cinta{
    int ID_cinta;
    int Tamaño_cinta;
    int N_elementos;
};

extern pthread_mutex_t mutex_factory_creacion;
extern pthread_mutex_t mutex_factory_inicializacion;
extern pthread_cond_t cond_inicializacion;
extern int id_turno;
extern int termine;
extern pthread_cond_t cond_fin;
extern pthread_mutex_t mutex_fin_completo;
extern pthread_mutex_t mutex_factory_control;
extern pthread_cond_t cond_inicializacion_completa;
extern pthread_mutex_t mutex_inicializacion_completa;