#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <windows.h>

#include "fuselib.c"

int main(int argc, char **argv) {
    const char* initBin = "../bin/filesystem.bin";
    int initialization = initFromBin(initBin);

    if(initialization == 0){
        printf("Filesystem propperly mounted\n");

        //devolverArchivo("../data/juani.mp4","videazo");
        //copiarDesdeArchivo("../data/portaTruco.mp3","temazo");
        //rmfile("temazo");
        //Sleep(5000);
        
        //ls();
        mostrarTodo();
    }else{
        printf("Error at init, aborpting.\n");
        return 1;
    }

    exitFileSystem(initBin);
    return initialization;
    
}