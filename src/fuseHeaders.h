#ifndef FUSEHEADERS_H
#define FUSEHEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

/////////////Estructuras////////////
typedef struct Files{
    unsigned long size;
    char* binario;
} TFiles;

typedef struct tabla{
    char* path;
    time_t creation_time;     // Tiempo de creación del archivo
    uid_t uid;                // ID de usuario del propietario
    gid_t gid;                // ID de grupo del propietario
    mode_t mode;              // Modo de protección del archivo
    struct Files* data;
    struct tabla* next;
}elementoTabla;
/////////////Estructuras////////////

/////////Variables globales/////////
#define LONGESTPATHSIZE 1024
elementoTabla* globalTable = NULL;
char* currentPath = NULL;
char* FUSEINITFILES = NULL;
/////////Variables globales/////////

////////////////////////////////////fuseinitexit.c////////////////////////////////////
int initEmptyFilesystem();
int loadAsDir(const char*);
int loadAsFile(const char*);
int initFromBin(const char*);
void exportarABin(const char*,const char*,size_t);
void cleanFileSystem();
void saveAllDataFromFiles();
void fileSystemToBin(const char*);
void exitFileSystem(const char*);
////////////////////////////////////fuseinitexit.c////////////////////////////////////

///////////////////////////////////////fuseIO.c///////////////////////////////////////
int guardarDatos(char*, char* , int);
void copiarDesdeArchivo(const char*, char*);
int devolverArchivo(char*,char*);
///////////////////////////////////////fuseIO.c///////////////////////////////////////

//////////////////////////////////fuselibUtilities.c//////////////////////////////////
elementoTabla* pathExists(char*);
char* checksPrevios(char*);
int subdir_inmediato(const char*,const char*);
char* ultimoComponente(char*);
void remove_last_element();
void remove_last_elementArg(char*);
int startsWith(const char*, const char*);
void cambiarHijos(const char*, const char*);
char* absoluteFromRelative(const char*);
void mostrarTodo();     //<-Debug
void totalsize();       //<-Debug
unsigned long hash_djb2(const char*);
char* hash_string(const char*);
//////////////////////////////////fuselibUtilities.c//////////////////////////////////

//////////////////////////////////////fuselib.c///////////////////////////////////////
int createDir(char*);
char* ls();                                             //Mirar especificación, declara memoria que hay que liberar
void pwd();
void changeDirectory(char*);
void renombrar(const char*,const char*);
void rmfile(char*);
void removedir(char*);
//////////////////////////////////////fuselib.c///////////////////////////////////////
#endif
