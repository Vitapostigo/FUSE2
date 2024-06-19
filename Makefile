# Nombre del ejecutable
TARGET = mainfuse

# Directorio de los archivos fuente
SRC_DIR = src

# Archivos fuente
SRCS = FUSE.c $(SRC_DIR)/fuselib.c

# Dependencias de FUSE
FUSE_CFLAGS = $(shell pkg-config fuse --cflags)
FUSE_LIBS = $(shell pkg-config fuse --libs)

# Bandera para definir _FILE_OFFSET_BITS=64
DEFINES = -D_FILE_OFFSET_BITS=64

# Directorio de montaje
MOUNT_DIR = mount_point

# Objetivo por defecto: compilar el proyecto
all: $(TARGET)

# Compilar el proyecto
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(FUSE_CFLAGS) $(FUSE_LIBS) $(DEFINES)

# Montar el sistema de ficheros
mount: $(TARGET)
	mkdir -p $(MOUNT_DIR)
	./$(TARGET) $(MOUNT_DIR)

# Desmontar el sistema de ficheros
umount:
	fusermount -u $(MOUNT_DIR)

# Limpiar los archivos generados
clean:
	rm -f $(TARGET)
	rm -rf $(MOUNT_DIR)

debug: $(TARGET)
	mkdir -p $(MOUNT_DIR)
	./$(TARGET) -d $(MOUNT_DIR)
