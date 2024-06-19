#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h> // Para getuid y getgid
//#include "src/fuselib.c"
#include <ctype.h>
#include "src/fuseHeaders.h"

elementoTabla* globalTable =NULL ;
char* currentPath= NULL;
char* FUSEINITFILES =NULL;

// Obtener atributos de archivo
static int fs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    elementoTabla *element = pathExists((char *)path);

    if (element == NULL) {
        return -ENOENT;
    }

    if (element->data == NULL) { // Es un directorio
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else { // Es un archivo regular
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = element->data->size;
    }

    stbuf->st_uid = element->uid;
    stbuf->st_gid = element->gid;
    stbuf->st_ctime = element->creation_time;
    stbuf->st_mtime = element->creation_time;
    stbuf->st_atime = element->creation_time;

    return 0;
}

// Leer el contenido de un directorio
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    elementoTabla *element = pathExists((char *)path);

    if (element == NULL || element->data != NULL) {
        return -ENOENT;
    }

    // Agrega las entradas de . y .. de forma predeterminada
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // Obtener la lista de archivos y directorios usando la función ls de fuselib.c
    char *list = ls(path);
    if (list == NULL) {
        return -ENOMEM; // Error de memoria
    }

    // Separar la lista en nombres individuales y añadirlos al buffer
    char *token = strtok(list, "\n");
    while (token != NULL) {
        // Eliminar espacios en blanco adicionales
        char *start = token;
        while (isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) end--;
        end[1] = '\0';

        filler(buf, start, NULL, 0);
        token = strtok(NULL, "\n");
    }

    free(list);
    return 0;
}

// Leer el contenido de un archivo
static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;

    elementoTabla *element = pathExists((char *)path);

    if (element == NULL) {
        return -ENOENT;
    }

    if (element->data == NULL) {
        return -EISDIR; // Indica que es un directorio, no se puede leer
    }

    if (offset < element->data->size) {
        if (offset + size > element->data->size) {
            size = element->data->size - offset;
        }
        memcpy(buf, element->data->binario + offset, size);
    } else {
        size = 0;
    }

    return size;
}

static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi; // Evitar advertencia de parámetro no utilizado

    // Verificar si el archivo ya existe
    elementoTabla* file = pathExists((char*)path);
    if (file != NULL) {
        return -EEXIST; // Archivo ya existe
    }

    // Crear un nuevo elementoTabla para el archivo
    elementoTabla* newFile = (elementoTabla*) malloc(sizeof(elementoTabla));
    if (newFile == NULL) {
        return -ENOMEM; // Error de memoria
    }

    // Asignar y configurar los campos del nuevo archivo
    newFile->path = strdup(path);
    if (newFile->path == NULL) {
        free(newFile);
        return -ENOMEM;
    }
    newFile->mode = mode;
    newFile->creation_time = time(NULL);
    newFile->next = globalTable;
    globalTable = newFile;

    printf("Archivo creado: %s\n", path);
    return 0;
}

// Crear un directorio
static int fs_mkdir(const char *path, mode_t mode) {
    // Verificar si el directorio ya existe
    if (pathExists((char *)path) != NULL) {
        return -EEXIST; // El directorio ya existe
    }

    // Verificar condiciones previas y permisos de la ruta
    char *parentPath = strdup(path);
    remove_last_elementArg(parentPath);
    elementoTabla *parentElement = pathExists(parentPath);
    free(parentPath);

    if (parentElement == NULL || parentElement->data != NULL) {
        return -ENOENT; // El directorio padre no existe o es un archivo
    }

    // Crear una nueva entrada para el directorio
    elementoTabla *newDir = (elementoTabla *)malloc(sizeof(elementoTabla));
    if (newDir == NULL) {
        return -ENOMEM; // No se pudo asignar memoria
    }

    newDir->path = strdup(path);
    newDir->data = NULL; // Un directorio no tiene datos binarios
    newDir->mode = mode | S_IFDIR;
    newDir->uid = getuid();
    newDir->gid = getgid();
    newDir->creation_time = time(NULL);
    newDir->next = NULL;

    // Añadir el nuevo directorio a la lista global
    if (globalTable == NULL) {
        globalTable = newDir;
    } else {
        elementoTabla *current = globalTable;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newDir;
    }
    return 0;
}


// Renombrar un archivo o directorio
static int fs_rename(const char *from, const char *to) {
    renombrar(from, to); // Usa tu implementación de renombrar

    return 0;
}

// Eliminar un archivo
static int fs_unlink(const char *path) {
    rmfile((char *)path); // Implementa esta función para eliminar archivos

    return 0;
}

// Eliminar un directorio
static int fs_rmdir(const char *path) {
    elementoTabla* dir = pathExists((char*)path);
    if (dir == NULL) {
        printf("El directorio no existe.\n");
        return -ENOENT;
    }

    // Verificar si el directorio está vacío
    char* contents = ls(path);
    if (contents == NULL) {
        printf("Error al listar el contenido del directorio.\n");
        return -EIO;
    }
    if (strlen(contents) > 0) {
        printf("El directorio no está vacío.\n");
        free(contents);
        return -ENOTEMPTY;
    }
    free(contents);

    // Eliminar el directorio usando removedir
    removedir((char*)path);
    printf("Directorio eliminado.\n");
    return 0;
}

static struct fuse_operations fuse_oper = {
    .getattr   = fs_getattr,
    .readdir   = fs_readdir,
    .read      = fs_read,
    .create =    fs_create,
    .mkdir     = fs_mkdir,
    .rename    = fs_rename,
    .unlink    = fs_unlink,
    .rmdir     = fs_rmdir,
};

int main(int argc, char *argv[]) {
    initEmptyFilesystem(); // Inicializa tu sistema de ficheros

    return fuse_main(argc, argv, &fuse_oper, NULL);
}
