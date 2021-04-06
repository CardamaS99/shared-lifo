/**
 * @file buffer.h
 * @author F. Javier Cardama Santiago (javier.cardama@usc.es)
 * @brief Implementación de un buffer LIFO en memoria compartida.
 * 
 * @details La estructura de datos Buffer permite almacenar tantos elementos del
 * tipo indicado como se indiquen a la hora de construirlo.
 * 
 * Para poder realizar la compartición de los valores del buffer entre otros
 * procesos se hace uso de la función mmap() almacenando los ficheros en el
 * fichero indicado en la variable 'file'.
 * 
 * @version 1.0
 * @date 2021-04-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _LIFO_BUFFER_H
#define _LIFO_BUFFER_H

#include <stdint.h>

/**
* ------------------------------ESTRUCTURA DEL TAD------------------------------
* Tipo de dato exportado: puntero a estructura de tipo ST_BUFFER
* Campos:
*		- values: variable que apunta al primer elemento del buffer.
*		- size: número de elementos que puede almacenar el buffer.
*		- count: número de elementos que se han insertado en el buffer.
*       - file: fichero para el almacenamiento del buffer.
*       - count_file: fichero para el almacenamiento de la variable count.
*/
typedef struct ST_BUFFER *Buffer;

/**
 * @brief Valor almacenado en el buffer.
 * 
 */
typedef int BufferValue;

/*
* ---------------------------MODIFICACIÓN DE VARIABLES--------------------------
* De los tres campos de la estructura buffer, solo dos se pueden modificar
* posteriormente a la creación del buffer:
*		- Variable   count: el entero apuntado por 'count' puede verse 
*							decrementado o incrementado con el uso de las 
*							funciones 'LIFO_pop' y 'LIFO_push'.
*
*		- Array de valores: los valores del array pueden ser modificados 
*							mediante las funciones 'LIFO_pop' y 'LIFO_push', 
*                           pudiendo modificar solamente el último valor 
*                           insertado.
*/

/**
 * @brief Constructor del buffer tipo LIFO. Si este ya está creado y almacenado
 * en un fichero con el mismo nombre que 'file' este será sobreescrito.
 * 
 * 
 * @param file el fichero donde se almacenarán los valores del buffer para
 * permitir la compartición entre ficheros.
 * @param size tamaño del buffer.
 * 
 * @pre size > 0
 * @pre file != NULL
 * 
 * @post se crea el fichero 'file'.
 * @post los valores del buffer están establecidos por defecto.
 * 
 * @return Buffer
 */
Buffer LIFO_create_buffer(char* file, uint32_t size);

/**
 * @brief Destruye la instancia creada del buffer, liberando la memoria
 * compartida y eliminando todos los valores.
 * 
 * @param buffer objeto a destruir.
 * 
 * @pre buffer != NULL
 * @pre *buffer != NULL
 * 
 * @post la proyección en memoria del fichero se cierra.
 * @post se borra el fichero de respaldo del buffer.
 * @post *buffer = NULL
 */
void LIFO_destroy_buffer(Buffer *buffer);

/**
 * @brief En caso de que exista el fichero se devuelve la instancia de buffer
 * almacenada en este. En caso contrario se devuelve NULL.
 * 
 * @param file fichero del buffer a obtener.
 * 
 * @pre file != NULL
 * 
 * @return Buffer 
 */
Buffer LIFO_get_buffer(char *file);

/**
 * @brief Inserta el valor indicado por parámetro en la cima de la pila. En caso
 * de que el buffer se encuentre lleno se descarta la inserción.
 * 
 * @param buffer pila en la que realizar el push.
 * @param value valor a insertar.
 * 
 * @pre buffer != NULL
 * @pre count < size
 * 
 * @return 1 si se ha podido insertar correctamente, 0 en caso contrario (buffer
 * lleno).
 */
uint8_t LIFO_push(Buffer buffer, BufferValue value);

/**
 * @brief Saca del buffer la cima de la pila (último elemento insertado), en
 * caso de que el buffer esté vacío no se asegura un valor correcto de retorno,
 * por lo que deberá ser controlado por el usuario.
 * 
 * @param buffer pila a la que realizar el pop.
 * 
 * @pre buffer != NULL.
 * 
 * @return BufferValue 
 */
BufferValue LIFO_pop(Buffer buffer);

/**
 * @brief Obtiene el tamaño del buffer.
 * 
 * @param buffer 
 * 
 * @pre buffer != NULL
 * 
 * @return uint32_t 
 */
uint32_t LIFO_get_size(Buffer buffer);

/**
 * @brief Obtiene el número de elementos que hay en el buffer.
 * 
 * @param buffer 
 * 
 * @pre buffer != NULL.
 * 
 * @return uint32_t 
 */
uint32_t LIFO_get_count(Buffer buffer);

/**
 * @brief Comprueba si la pila está vacía.
 * 
 * @param buffer 
 * 
 * @pre buffer != NULL.
 * 
 * @return uint8_t '1' si la pila está vacía, '0' en caso contrario
 */
uint8_t LIFO_is_clear(Buffer buffer);


/**
 * @brief Comprueba si la pila está llena.
 * 
 * @param buffer 
 * 
 * @pre buffer != NULL.
 * 
 * @return uint8_t '1' si la pila está llena, '0' en caso contrario.
 */
uint8_t LIFO_is_full(Buffer buffer);

/**
 * @brief Imprime todos los valores del buffer.
 * 
 * @param buffer buffer a imprimir
 * 
 * @pre buffer != NULL.
 */
void LIFO_print_buffer(Buffer buffer);

#endif