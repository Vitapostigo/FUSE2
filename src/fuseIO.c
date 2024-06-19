#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "fuseHeaders.h"

extern elementoTabla* globalTable;
extern char* currentPath;
extern char* FUSEINITFILES;

int guardarDatos(char* filename, char* data, int size) {
    char* msgError = checksPrevios(filename);   
    if(msgError != NULL){
        printf("%s\n", msgError);
        return 1;
    }

    // Crear la ruta completa del nuevo directorio
    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(filename)+1));
    newString[0]='\0';
    if (newString == NULL) {
        perror("Error al asignar memoria para newString");
        return 1;
    }

    strcpy(newString, currentPath);
    strcat(newString, filename);

    // Verificar si el directorio ya existe
    if (pathExists(newString) != NULL){
        printf("El elemento a crear ya existe\n");
        free(newString);
        return 1;
    }

    // Encontrar el último elemento en la tabla global
    elementoTabla* toAppend = globalTable;
    while (toAppend->next != NULL){
        toAppend = toAppend->next;
    }

    // Crear un nuevo elemento para el nuevo directorio
    toAppend -> next = malloc(sizeof(elementoTabla));
    if (toAppend->next == NULL) {
        perror("Error al asignar memoria para nuevo elemento de la tabla");
        free(newString);
        return 1;
    }
    toAppend = toAppend->next;

    // Asignar la ruta al nuevo elemento
    toAppend -> path = malloc(sizeof(char)*(strlen(newString)+1));
    if (toAppend->path == NULL) {
        perror("Error al asignar memoria para la ruta del nuevo elemento");
        free(toAppend->next);
        free(newString);
        return 1;
    }
    strcpy(toAppend->path, newString);
    free(newString);

    toAppend->next = NULL;

    toAppend -> data = (TFiles*) malloc(sizeof(TFiles));

    if (toAppend -> data == NULL) {
        printf("Error en la asignación de memoria.\n");
        //mirar qué liberar
        return 1;
    }

    toAppend-> data -> size = size;

    toAppend -> data -> binario = (char *) malloc(size);

    if (toAppend -> data -> binario == NULL) {
        fprintf(stderr, "Error en la asignación de memoria para datos.\n");
        //mirar qué liberar
        return 1;
    }
    memcpy(toAppend -> data -> binario, data, size);

    return 0;
}

void copiarDesdeArchivo(const char* filename, char* newFile){
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *) malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error en la asignación de memoria para buffer.\n");
        fclose(file);
        return;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);

    // Inserta los datos en la lista
    guardarDatos(newFile, buffer, file_size);

    free(buffer);
    
}

int devolverArchivo(char* nuevoArchivo,char* archivoEnFUSe){
    char* nombreAbuscar = malloc(sizeof(char)*(strlen(currentPath)+strlen(archivoEnFUSe)+1));
    strcpy(nombreAbuscar,currentPath);
    strcat(nombreAbuscar,archivoEnFUSe);

    elementoTabla* copia = pathExists(nombreAbuscar);

    if (copia == NULL){
        printf("El elemento a devolver no se ha encontrado!\n");
        free(nombreAbuscar);
        return 1;
    }

    FILE *file = fopen(nuevoArchivo, "wb");
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo para escribir.\n");
        return 1;
    }

    fwrite(copia->data->binario, 1, copia->data->size, file);

    fclose(file);
    return 0;
}