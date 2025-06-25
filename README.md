# Práctica de Sistemas Operativos - Factoría Multihilo

Este proyecto fue desarrollado como parte de la tercera práctica de la asignatura de Sistemas Operativos en la Universidad Carlos III de Madrid.

Contiene un sistema multihilo implementado en C para entornos **UNIX/Linux**, que simula una factoría donde múltiples hilos producen y consumen datos en colas circulares, siguiendo el patrón productor-consumidor.

## ⚙️ Requisitos

- Sistema operativo tipo UNIX (Linux, macOS...)
- Compilador C (por ejemplo, `gcc`)
- Pthreads (biblioteca estándar en sistemas UNIX)
- Makefile para automatizar la compilación

## 📁 Estructura del proyecto

- `factory_manager.c`: Proceso principal que gestiona la entrada, crea los hilos y coordina su ejecución.
- `process_manager.c`: Hilos secundarios encargados de gestionar colas y sincronizar productores/consumidores.
- `circular_queue.c` y `circular_queue.h`: Implementación de la cola circular y sus funciones auxiliares.
- `Makefile`: Facilita la compilación del proyecto.

## 🚀 Ejecución

Compilar el proyecto con:

```bash
make
```

Ejecutar el programa con un fichero de entrada:

```bash
./factory_manager entrada.txt
```

El archivo `entrada.txt` debe seguir el formato <N.º max cintas> [<ID cinta> <Tamaño de cinta> <N.º elementos>]+ donde:


- **N.º max cintas**:  
  Número máximo de hilos `process_manager` que se pueden crear.  

- **ID cinta**:  
  Identificador único de la cinta asociada a cada `process_manager`. Se usa para identificar los productos en las trazas del programa.

- **Tamaño de cinta**:  
  Capacidad máxima del buffer circular (cola) entre el productor y el consumidor.

- **N.º elementos**:  
  Número total de elementos que la cinta debe generar (cantidad de trabajo).

### 🧪 Ejemplo de entrada válida:

```4 5 5 2 1 2 3 3 5 2 ```

> Esto indica que se pueden crear un máximo de 4 cintas:  
> - Process_manager con ID `5`, tamaño `5`, y `2` elementos a generar.  
> - Process_manager con ID `1`, tamaño `2`, y `3` elementos a generar.
> - Process_manager con ID `3`, tamaño `5`, y `2` elementos a generar.


## 🧠 Diseño y Arquitectura

Este proyecto sigue una arquitectura jerárquica multihilo:

### Niveles principales de control

1. **`Factory_manager` (proceso principal)**  
   - Crea y coordina los hilos `Process_manager`.

2. **`Process_manager` (hilos hijos del factory)**  
   - Cada uno crea su propia cola circular y dos hilos internos: un productor y un consumidor.

3. **Hilos productor y consumidor**  
   - Se comunican mediante mutex y variables de condición.

### Fases del `Factory_manager`

- **Lectura del archivo de entrada**: Extrae el número máximo de hilos y la configuración de cada cinta.
- **Inicialización**: Prepara mutex, condiciones y estructuras auxiliares.
- **Creación de hilos `Process_manager`**: Todos se crean y se bloquean hasta que el último esté listo.
- **Inicialización secuencial de hilos**: Cada hilo se inicializa cuando le corresponde su turno.
- **Ejecución secuencial**: Cada hilo ejecuta su trabajo y el `Factory_manager` lo recoge con `pthread_join`.

### Jerarquía de control

| Entidad               | Controla a...                            | Se comunica con...                           |
|-----------------------|------------------------------------------|----------------------------------------------|
| `Factory_manager`     | Todos los `Process_manager`              | Usando mutex y variables de condición        |
| `Process_manager`     | Su productor y consumidor internos       | También con mutex y señales                  |
| Productor/Consumidor  | Se coordinan entre sí                    | Mediante condiciones compartidas             |
