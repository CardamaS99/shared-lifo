#include "buffer.h"

#include <stdio.h>

#define BUFFER_SIZE 10
#define BUFFER_FILE ".buffer"

int main(int argc, char const *argv[])
{
    Buffer buffer;
    int a = 7, b = 3;

    // Se intenta obtener el buffer y, en caso de que no exista, se crea.
    if((buffer = LIFO_get_buffer(BUFFER_FILE)) == NULL)
        buffer = LIFO_create_buffer(BUFFER_FILE, BUFFER_SIZE);

    // Se realiza el push
    LIFO_push(buffer, a);
    LIFO_push(buffer, b);

    // Se imprime el buffer
    LIFO_print_buffer(buffer);

    // Se sacan los valores
    getchar();
    printf("%d\n", LIFO_pop(buffer));
    getchar();
    printf("%d\n", LIFO_pop(buffer));

    LIFO_destroy_buffer(&buffer);

    return 0;
}
