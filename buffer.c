#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct ST_BUFFER
{
    BufferValue *values;
    uint32_t size;
    uint32_t *count;
    char *file;
    char *count_file;
};

void _print_error(char *message)
{
    perror(message);
}

/**
 * @brief Comprueba que ninguno de los punteros relacionados con buffer sea nulo
 * y, en caso de serlo, devuelve 1. En caso contrario se retorna 0.
 * 
 * @param buffer 
 */
uint8_t _check_nulls(Buffer *buffer)
{
    /* Se comprueba que los valores no sean nulos */
    if (buffer == NULL || (*buffer) == NULL)
    {
        _print_error("BUFFER = NULL in LIFO_DESTROY_BUFFER");
        return 1;
    }

    if ((*buffer)->values == NULL)
    {
        _print_error("BUFFER VALUES = NULL in LIFO_DESTROY_BUFFER");
        return 1;
    }

    if ((*buffer)->count == NULL)
    {
        _print_error("BUFFER VALUES = NULL in LIFO_DESTROY_BUFFER");
        return 1;
    }

    return 0;
}

Buffer LIFO_create_buffer(char *file, uint32_t size)
{
    /* -  0. DECLARACIÓN DE VARIABLES               ------------------------- */
    // Descriptor del fichero de los valores del buffer y count.
    int fd, count_fd;

    // Buffer a devolver al usuario.
    Buffer buffer;

    // Fichero para la variable count.
    char *count_file;

    /* -  1. RESERVA DE MEMORIA LOCAL               ------------------------- */
    // Se reserva memoria para la estructura del buffer que es local
    if ((buffer = (Buffer)malloc(sizeof(struct ST_BUFFER))) == NULL)
    {
        _print_error("MALLOC ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  2.   CREACIÓN DE FICHEROS                 ------------------------- */
    /* -  2.1. Valores del buffer                   ------------------------- */
    // Creación del fichero para almacenar los valores del buffer
    if ((fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
    {
        _print_error("OPEN ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se trunca el archivo con el nuevo número de bytes que va a contener, que
    // en este caso será el tamaño del propio buffer.
    if (truncate(file, sizeof(BufferValue) * size) != 0)
    {
        _print_error("TRUNCATE ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  2.2. Variable count                       ------------------------- */
    count_file = (char *)malloc(sizeof(char) * (strlen(file)) + 7);

    if (count_file == NULL)
    {
        _print_error("MALLOC ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se copia el nombre del fichero.
    strcpy(count_file, file);

    // Se le añade el final de -count.
    strcat(count_file, "-count");

    // Se crea el fichero
    if ((count_fd = open(count_file, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
    {
        _print_error("FOPEN ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se trunca con el nuevo tamaño
    if (truncate(count_file, sizeof(uint32_t)) != 0)
    {
        _print_error("TRUNCATE ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  3.   MEMORIA COMPARTIDA                   ------------------------- */
    /* -  3.1. Valores del buffer                   ------------------------- */
    // Se reserva la memoria del array de valores mediante el uso mmap que
    // permite realizar un mapeo compartido.
    buffer->values = (BufferValue *)mmap(NULL, sizeof(BufferValue) * size,
                                         PROT_READ | PROT_WRITE, MAP_SHARED,
                                         fd, 0);

    if (buffer->values == MAP_FAILED)
    {
        _print_error("MMAP VALUES in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  3.2. Variable count                       ------------------------- */
    buffer->count = (uint32_t *)mmap(NULL, sizeof(uint32_t),
                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                     count_fd, 0);

    if (buffer->count == MAP_FAILED)
    {
        _print_error("MMAP COUNT in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  4.   CIERRE DE FICHEROS                   ------------------------- */
    // Se cierran los ficheros
    if (close(fd) < 0)
    {
        _print_error("CLOSE VALUES in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    if (close(count_fd) < 0)
    {
        _print_error("CLOSE COUNT in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se actualizan las variables
    buffer->file = file;
    buffer->count_file = count_file;
    buffer->size = size;

    *(buffer->count) = 0;

    // Se devuelve el buffer
    return buffer;
}

void LIFO_destroy_buffer(Buffer *buffer)
{
    if (_check_nulls(buffer))
        return;

    /* -  1.   VALORES DEL BUFFER                   ------------------------- */
    // Se cierran las proyecciones de los valores del buffer
    if (munmap((*buffer)->values, sizeof(BufferValue) * (*buffer)->size) < 0)
    {
        _print_error("MUNMAP VALUES in LIFO_DESTROY_BUFFER");
        return;
    }

    // Se elimina el fichero de respaldo (no se hacen comprobaciones, si ya se
    // ha borrado se libera el resto de la memoria)
    remove((*buffer)->file);

    /* -  2.   VARIABLE COUNT                       ------------------------- */
    // Se cierra la proyección
    if (munmap((*buffer)->count, sizeof(uint32_t)) < 0)
    {
        _print_error("MUNMAP COUNT in LIFO_DESTROY_BUFFER");
        return;
    }

    // Se elimina el fichero de respaldo
    remove((*buffer)->count_file);

    /* -  3.   LIBERACIÓN DE MEMORIA                ------------------------- */
    free((*buffer)->count_file);
    free(*buffer);

    (*buffer) = NULL;
}

Buffer LIFO_get_buffer(char *file)
{
    // Descriptor del fichero.
    int fd, count_fd;

    // Estructura stat para almacenar información de los ficheros de respaldo.
    // Esta estructura se va a utilizar para conocer el tamaño del fichero y
    // saber cuántos elementos tiene el buffer.
    struct stat finfo_values;

    // Buffer en el que se almacenarán los valores
    Buffer buffer;

    char *count_file;

    // Se reserva memoria para la estructura del buffer que es local
    if ((buffer = (Buffer)malloc(sizeof(struct ST_BUFFER))) == NULL)
    {
        _print_error("MALLOC ERROR in LIFO_GET_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  1.   VALORES DEL BUFFER                   ------------------------- */
    // Se abre el fichero de respaldo de los valores
    if ((fd = open(file, O_RDWR, 0666)) < 0)
    {
        // En caso de que no haya ningún fichero asociado a los valores del
        // buffer, se devuelve NULL
        free(buffer);
        return NULL;
    }

    // Se obtiene la estructura stat del fichero de respaldo
    if (fstat(fd, &finfo_values))
    {
        _print_error("FSTAT ERROR in LIFO_GET_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se obtiene la proyección mediante el campo st_size de la estructura stat
    if ((buffer->values = (BufferValue *)mmap(NULL, finfo_values.st_size,
                                              PROT_READ | PROT_WRITE,
                                              MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        _print_error("MMAP ERROR in LIFO_GET_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se cierra el fichero de respaldo
    if (close(fd) < 0)
    {
        _print_error("CLOSE ERROR in LIFO_GET_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  2.   VARIABLE COUNT                       ------------------------- */

    // Crea un fichero a partir del base para la variable 'count'
    count_file = (char *)malloc(sizeof(char) * (strlen(file)) + 7);

    if (count_file == NULL)
    {
        _print_error("MALLOC ERROR in LIFO_CREATE_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se copia el nombre del fichero.
    strcpy(count_file, file);

    // Se le añade el final de -count.
    strcat(count_file, "-count");

    // Para la variable count no es necesario el stat, tan solo el respaldo.
    if ((count_fd = open(count_file, O_RDWR, 0666)) < 0)
    {
        // En caso de que no haya ningún fichero asociado a los valores del
        // buffer, se devuelve NULL
        if (munmap(buffer->values, finfo_values.st_size) < 0)
        {
            _print_error("MUNMAP VALUES in LIFO_DESTROY_BUFFER");
        }

        free(buffer);
        return NULL;
    }

    // Se obtiene la proyección
    buffer->count = (uint32_t *)mmap(NULL, sizeof(uint32_t),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED, count_fd, 0);
    if (buffer->count == MAP_FAILED)
    {
        _print_error("MMAP ERROR in LIFO_GET_BUFFER");
        exit(EXIT_FAILURE);
    }

    // Se cierra el fichero de respaldo
    if (close(count_fd) < 0)
    {
        _print_error("CLOSE ERROR in LIFO_GET_BUFFER");
        exit(EXIT_FAILURE);
    }

    /* -  3.   ESTABLECER VARIABLES                 ------------------------- */
    buffer->size = finfo_values.st_size / sizeof(BufferValue);
    buffer->file = file;
    buffer->count_file = count_file;

    return buffer;
}

uint8_t LIFO_push(Buffer buffer, BufferValue value)
{
    uint32_t count;

    if (_check_nulls(&buffer))
        exit(EXIT_FAILURE);

    // Se mira el valor de count y se comprueba que no sea mayor que el tamaño
    // del buffer
    count = *(buffer->count);

    if (count < buffer->size)
    {
        // Se inserta el valor en la última posición libre (la apuntada por
        // count).
        buffer->values[count] = value;

        // Se incrementa el valor del contador en 1.
        (*(buffer->count))++;

        return 1;
    }

    return 0;
}

BufferValue LIFO_pop(Buffer buffer)
{
    uint32_t count;
    
    // Valor devuelto por el pop
    BufferValue value;

    if (_check_nulls(&buffer))
        exit(EXIT_FAILURE);

    // Se mira el valor de count y se comprueba que no sea menor que el tamaño
    // del buffer
    count = *(buffer->count);

    if (count > 0)
    {
        value = buffer->values[count - 1];
        (*(buffer->count))--;
    }
    
    return value;
}

uint32_t LIFO_get_size(Buffer buffer)
{
    return buffer->size;
}

uint32_t LIFO_get_count(Buffer buffer)
{
    return *(buffer->count);
}

uint8_t LIFO_is_clear(Buffer buffer)
{
    return buffer->count == 0;
}

uint8_t LIFO_is_full(Buffer buffer)
{
    return buffer->count == buffer->size;
}

void LIFO_print_buffer(Buffer buffer)
{
    int count;

	int i;

	count = *(buffer->count);

	for (i = 0; i < buffer->size; i++)
	{
		if (i == 0)
		{
			printf("┌─");
		}
		else if (i == buffer->size - 1)
		{
			printf("┬─┐\n");
		}
		else
		{
			printf("┬─");
		}
	}
	for (i = 0; i < buffer->size; i++)
	{
		if (i == buffer->size - 1)
		{
			if (i < count)
			{
				if (buffer->values[i] == -1)
				{
					printf("│▓│\n");
				}
				else
				{
					printf("│%d│\n", buffer->values[i]);
				}
			}
			else
			{
				printf("│ │\n");
			}
		}
		else
		{
			if (i < count)
			{
				if (buffer->values[i] == -1)
				{
					printf("│▓");
				}
				else
				{
					printf("│%d", buffer->values[i]);
				}
			}
			else
			{
				printf("│ ");
			}
		}
	}

	for (i = 0; i < buffer->size; i++)
	{
		if (i == 0)
		{
			printf("├─");
		}
		else if (i == buffer->size - 1)
		{
			printf("┴─┘\n");
		}
		else
		{
			printf("┴─");
		}
	}

	printf("└─> Tam: %d | Count: %d\n", buffer->size, *(buffer->count));
}