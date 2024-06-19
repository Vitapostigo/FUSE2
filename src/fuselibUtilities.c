#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
//En teoría sobran los siguientes 3 imports. Si falla algo se descomentan.
//#include <errno.h>
//#include <fcntl.h>
//#include <time.h>

#include "fuseHeaders.h"

extern elementoTabla* globalTable;
extern char* currentPath;
extern char* FUSEINITFILES;

elementoTabla* pathExists(char* path){
    /*
    elementoTabla* copia = (elementoTabla*) globalTable;
    while(copia != NULL && strcmp(copia -> path, path) != 0){
        copia = copia -> next;
    }
    return copia;
    */
    elementoTabla* current = globalTable;
    while (current != NULL) {
        if (strcmp(current->path, path) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

char* checksPrevios(char* newDir){
    if(strchr(newDir, '/')){
        return "The new directory can't contain the: / sign.";
    }
    if(strchr(newDir, '?')){
        return "The new directory can't contain the: ? sign.";
    }
    if(strchr(newDir, '.')){
        return "The new directory can't contain the: . sign.";
    }
    if(strlen(newDir) + strlen(currentPath) >= LONGESTPATHSIZE - 1){
        return "Path is too long for the given dir to create.";
    }
    
    return NULL;
}

int subdir_inmediato(const char* parent,const char* child) {
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);

    if (parent[parent_len - 1] == '/') {
        parent_len--;
    }

    // Asegurarse de que 'child' comienza con 'parent' y que el siguiente caracter es '/'
    if (strncmp(parent, child, parent_len) == 0 && child[parent_len] == '/') {
        // Verificar que solo hay un nivel de directorio de diferencia
        const char* rest = child + parent_len + 1;
        // Verificar que no hay más slashes después del primer nivel
        return (strchr(rest, '/') == NULL || strchr(rest, '/') == rest + strlen(rest) - 1);
    }
    return 0;
}

char* ultimoComponente(char* path) {
    // Hacer una copia de la ruta para no modificar la original
    char* path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("Error al duplicar la ruta");
        return NULL;
    }

    //Tomamos el ultimo componente
    char* last_component = basename(path_copy);

    // Hacer otra copia para retornar, porque path_copy se libera
    char* result = strdup(last_component);
    free(path_copy);
    return result;
}



void remove_last_element() {
    size_t len = strlen(currentPath);

    if (currentPath[len - 1] == '/') {
        currentPath[len - 1] = '\0';
        len--;
    }

    for (size_t i = len - 1; i > 0; i--) {
        if (currentPath[i] == '/') {
            if (i == 0) {
                currentPath[1] = '\0';
            } else {
                currentPath[i + 1] = '\0';
            }
            return;
        }
    }
    
    strcpy(currentPath, "/");
}

// Implementación de otras funciones...

void remove_last_elementArg(char* path) {
    char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL) {
        if (lastSlash == path) { // La raíz "/"
            *(lastSlash + 1) = '\0';
        } else {
            *lastSlash = '\0';
        }
    }
}

// Implementación de otras funciones...


// Función para verificar si un string empieza con un prefijo dado
int startsWith(const char *str, const char *prefix) {
    while (*prefix) {
        if (*prefix++ != *str++) {
            return 0;
        }
    }
    return 1;
}

// Función para reemplazar el prefijo en una lista enlazada
void cambiarHijos(const char *path, const char *newPrefix) {
    elementoTabla* head = (elementoTabla*) globalTable;

    while (head != NULL) {
        if (startsWith(head->path, path)) {
            // Creamos un nuevo string con el prefijo reemplazado
            size_t newLen = strlen(newPrefix) + strlen(head->path) - strlen(path) + 1;
            char *newStr = (char *)malloc(newLen);
            strcpy(newStr, newPrefix);
            strcat(newStr, head->path + strlen(path));

            // Liberamos el string viejo y asignamos el nuevo
            free(head->path);
            head->path = newStr;
        }
        head = head->next;
    }
}

//Declara char* en memoria. Hay que liberarlos...
char* absoluteFromRelative(const char* rel){
    if(strcmp(rel,"/")==0){
        return strdup("/\0");
    }
    else if(strcmp(rel,".")==0){
        return strdup(currentPath);
    }
    else if(strcmp(rel,"..")==0){
        char* newFrom = strdup(currentPath);
        remove_last_elementArg(newFrom);

        if(strcmp(currentPath,newFrom)==0){ //Si el dir no cambia es que estamos saltando a root.
            free(newFrom);
            return strdup("/\0");
        }
        return newFrom;
    }
    else{
        char* newFrom = malloc(sizeof(char)*(strlen(currentPath)+strlen(rel)+1));
        //newFrom[0]='\0';
        strcpy(newFrom, currentPath);
        strcat(newFrom, rel);
        return newFrom;
    }
}

void mostrarTodo(){
    elementoTabla* copia = (elementoTabla*) globalTable;
    while(copia != NULL){
        printf("Path: %s\n",copia->path);
        copia = copia -> next;
    }
}

void totalsize(){
    int contador = 0;
    elementoTabla* copia = (elementoTabla*) globalTable;
    while(copia != NULL ){
        printf("Nombre: %s\n",copia->path);
        copia = copia -> next;
        contador++;
    }
    printf("Totasize: %i\n",contador);
}

//String para obtener el nombre de los binarios a guardar en /bin
//Uso: char *hash_str = hash_string(input);
unsigned long hash_djb2(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

// Función que devuelve el hash como string
char* hash_string(const char *str) {
    unsigned long hash = hash_djb2(str);

    // Crear un buffer para el string del hash
    // Un unsigned long en hexadecimal puede tener hasta 16 caracteres más el nulo
    char *output = malloc(17);
    if (output == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    snprintf(output, 17, "%lx", hash);
    return output;
}
