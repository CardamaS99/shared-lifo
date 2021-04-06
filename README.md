# Estructura de datos LIFO en memoria compartida

Implementación en C de una estructura de datos tipo LIFO cuyos valores se mapean a un fichero de respaldo para poder compartir el buffer con otros procesos.

Cada `buffer` creado consta de dos ficheros:
1. Fichero para almacenar los valores del buffer: "file" (indicado en el constructor).
2. Fichero para almacenar el número de elementos del buffer (count): "file-count" (a partir del nombre del fichero original).

## Compilación
Se proporciona un ejemplo de uso en el fichero `main.c`. 

Compilación:
`gcc main.c buffer.c -o main`

Ejecución
`./main`

