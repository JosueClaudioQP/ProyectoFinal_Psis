#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "archivos.h"
#include "util.h"

static void menu_archivos_listar(void);
static void menu_archivos_copiar(void);
static void menu_archivos_mover(void);
static void menu_archivos_eliminar(void);
static void menu_archivos_buscar(void);
static void menu_archivos_estadisticas(void);
static void listar_directorio(const char *ruta);
static int copiar_ruta(const char *origen, const char *destino);
static int copiar_archivo(const char *origen, const char *destino);
static int copiar_directorio(const char *origen, const char *destino);
static int mover_ruta(const char *origen, const char *destino);
static int eliminar_ruta(const char *ruta);
static int eliminar_directorio(const char *ruta);
static void buscar_por_nombre(const char *base, const char *patron);
static void mostrar_estadisticas(const char *ruta);
static off_t tamano_total(const char *ruta);
static size_t contar_elementos(const char *ruta, size_t *directorios);

void menu_archivos(){

    int opcion;

    do {
        limpiar_pantalla();

        printf("=====================================\n");
        printf("       SHELL DE ARCHIVOS\n");
        printf("=====================================\n\n");

        printf("1. Listar contenido\n");
        printf("2. Copiar archivo o directorio\n");
        printf("3. Mover archivo o directorio\n");
        printf("4. Eliminar archivo o directorio\n");
        printf("5. Buscar por nombre\n");
        printf("6. Estadisticas\n");
        printf("0. Volver\n\n");

        if (!leer_entero("Seleccione una opcion: ", &opcion)) {
            printf("\nEntrada invalida.\n");
            pausar();
            continue;
        }

        switch (opcion) {
            case 1:
                menu_archivos_listar();
                break;
            case 2:
                menu_archivos_copiar();
                break;
            case 3:
                menu_archivos_mover();
                break;
            case 4:
                menu_archivos_eliminar();
                break;
            case 5:
                menu_archivos_buscar();
                break;
            case 6:
                menu_archivos_estadisticas();
                break;
            case 0:
                break;
            default:
                printf("\nOpcion invalida.\n");
                pausar();
                break;
        }

    } while (opcion != 0);

}

static void menu_archivos_listar(void) {
    char ruta[PATH_MAX];

    if (leer_texto("Ruta a listar: ", ruta, sizeof(ruta))) {
        listar_directorio(ruta);
    }

    pausar();
}

static void menu_archivos_copiar(void) {
    char origen[PATH_MAX];
    char destino[PATH_MAX];

    if (leer_texto("Ruta de origen: ", origen, sizeof(origen)) &&
        leer_texto("Ruta de destino: ", destino, sizeof(destino))) {
        if (copiar_ruta(origen, destino) == 0) {
            printf("Operacion completada correctamente.\n");
        }
    }

    pausar();
}

static void menu_archivos_mover(void) {
    char origen[PATH_MAX];
    char destino[PATH_MAX];

    if (leer_texto("Ruta de origen: ", origen, sizeof(origen)) &&
        leer_texto("Ruta de destino: ", destino, sizeof(destino))) {
        if (mover_ruta(origen, destino) == 0) {
            printf("Movimiento completado correctamente.\n");
        }
    }

    pausar();
}

static void menu_archivos_eliminar(void) {
    char ruta[PATH_MAX];

    if (leer_texto("Ruta a eliminar: ", ruta, sizeof(ruta))) {
        if (eliminar_ruta(ruta) == 0) {
            printf("Elemento eliminado correctamente.\n");
        }
    }

    pausar();
}

static void menu_archivos_buscar(void) {
    char base[PATH_MAX];
    char patron[256];

    if (leer_texto("Ruta base de busqueda: ", base, sizeof(base)) &&
        leer_texto("Patron de busqueda: ", patron, sizeof(patron))) {
        buscar_por_nombre(base, patron);
    }

    pausar();
}

static void menu_archivos_estadisticas(void) {
    char ruta[PATH_MAX];

    if (leer_texto("Ruta para analizar: ", ruta, sizeof(ruta))) {
        mostrar_estadisticas(ruta);
    }

    pausar();
}

static void listar_directorio(const char *ruta) {
    DIR *directorio;
    struct dirent *entrada;
    struct stat info;
    char ruta_completa[PATH_MAX];

    directorio = opendir(ruta);
    if (directorio == NULL) {
        perror("No se pudo abrir la ruta");
        return;
    }

    printf("\nContenido de %s:\n\n", ruta);

    while ((entrada = readdir(directorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta, entrada->d_name) >= (int)sizeof(ruta_completa)) {
            continue;
        }

        if (stat(ruta_completa, &info) == -1) {
            continue;
        }

        printf("- %s [%s]", entrada->d_name, S_ISDIR(info.st_mode) ? "directorio" : "archivo");
        if (S_ISREG(info.st_mode)) {
            printf(" - %lld bytes", (long long)info.st_size);
        }
        printf("\n");
    }

    closedir(directorio);
}

static int copiar_ruta(const char *origen, const char *destino) {
    struct stat info;

    if (lstat(origen, &info) == -1) {
        perror("No se pudo acceder al origen");
        return -1;
    }

    if (S_ISREG(info.st_mode)) {
        return copiar_archivo(origen, destino);
    }

    if (S_ISDIR(info.st_mode)) {
        return copiar_directorio(origen, destino);
    }

    fprintf(stderr, "Tipo de archivo no soportado: %s\n", origen);
    return -1;
}

static int copiar_archivo(const char *origen, const char *destino) {
    int fd_origen;
    int fd_destino;
    ssize_t leidos;
    char buffer[8192];

    fd_origen = open(origen, O_RDONLY);
    if (fd_origen == -1) {
        perror("No se pudo abrir el archivo de origen");
        return -1;
    }

    fd_destino = open(destino, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_destino == -1) {
        perror("No se pudo abrir el archivo de destino");
        close(fd_origen);
        return -1;
    }

    while ((leidos = read(fd_origen, buffer, sizeof(buffer))) > 0) {
        if (write(fd_destino, buffer, (size_t)leidos) != leidos) {
            perror("Error al escribir el archivo");
            close(fd_origen);
            close(fd_destino);
            return -1;
        }
    }

    if (leidos == -1) {
        perror("Error al leer el archivo");
        close(fd_origen);
        close(fd_destino);
        return -1;
    }

    close(fd_origen);
    close(fd_destino);
    return 0;
}

static int copiar_directorio(const char *origen, const char *destino) {
    DIR *directorio;
    struct dirent *entrada;
    struct stat info;
    char origen_hijo[PATH_MAX];
    char destino_hijo[PATH_MAX];

    if (mkdir(destino, 0755) == -1 && errno != EEXIST) {
        perror("No se pudo crear el directorio destino");
        return -1;
    }

    directorio = opendir(origen);
    if (directorio == NULL) {
        perror("No se pudo abrir el directorio de origen");
        return -1;
    }

    while ((entrada = readdir(directorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (snprintf(origen_hijo, sizeof(origen_hijo), "%s/%s", origen, entrada->d_name) >= (int)sizeof(origen_hijo) ||
            snprintf(destino_hijo, sizeof(destino_hijo), "%s/%s", destino, entrada->d_name) >= (int)sizeof(destino_hijo)) {
            continue;
        }

        if (lstat(origen_hijo, &info) == -1) {
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            if (copiar_directorio(origen_hijo, destino_hijo) == -1) {
                closedir(directorio);
                return -1;
            }
        } else if (S_ISREG(info.st_mode)) {
            if (copiar_archivo(origen_hijo, destino_hijo) == -1) {
                closedir(directorio);
                return -1;
            }
        }
    }

    closedir(directorio);
    return 0;
}

static int mover_ruta(const char *origen, const char *destino) {
    if (rename(origen, destino) == 0) {
        return 0;
    }

    if (errno != EXDEV) {
        perror("No se pudo mover el elemento");
        return -1;
    }

    if (copiar_ruta(origen, destino) == -1) {
        return -1;
    }

    return eliminar_ruta(origen);
}

static int eliminar_ruta(const char *ruta) {
    struct stat info;

    if (lstat(ruta, &info) == -1) {
        perror("No se pudo acceder al elemento");
        return -1;
    }

    if (S_ISDIR(info.st_mode)) {
        return eliminar_directorio(ruta);
    }

    if (unlink(ruta) == -1) {
        perror("No se pudo eliminar el archivo");
        return -1;
    }

    return 0;
}

static int eliminar_directorio(const char *ruta) {
    DIR *directorio;
    struct dirent *entrada;
    struct stat info;
    char ruta_hija[PATH_MAX];

    directorio = opendir(ruta);
    if (directorio == NULL) {
        perror("No se pudo abrir el directorio");
        return -1;
    }

    while ((entrada = readdir(directorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (snprintf(ruta_hija, sizeof(ruta_hija), "%s/%s", ruta, entrada->d_name) >= (int)sizeof(ruta_hija)) {
            continue;
        }

        if (lstat(ruta_hija, &info) == -1) {
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            if (eliminar_directorio(ruta_hija) == -1) {
                closedir(directorio);
                return -1;
            }
        } else {
            if (unlink(ruta_hija) == -1) {
                perror("No se pudo eliminar un archivo interno");
                closedir(directorio);
                return -1;
            }
        }
    }

    closedir(directorio);

    if (rmdir(ruta) == -1) {
        perror("No se pudo eliminar el directorio");
        return -1;
    }

    return 0;
}

static void buscar_por_nombre(const char *base, const char *patron) {
    DIR *directorio;
    struct dirent *entrada;
    struct stat info;
    char ruta_hija[PATH_MAX];

    directorio = opendir(base);
    if (directorio == NULL) {
        perror("No se pudo abrir la ruta base");
        return;
    }

    while ((entrada = readdir(directorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (strstr(entrada->d_name, patron) != NULL) {
            printf("%s/%s\n", base, entrada->d_name);
        }

        if (snprintf(ruta_hija, sizeof(ruta_hija), "%s/%s", base, entrada->d_name) >= (int)sizeof(ruta_hija)) {
            continue;
        }

        if (lstat(ruta_hija, &info) == -1) {
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            buscar_por_nombre(ruta_hija, patron);
        }
    }

    closedir(directorio);
}

static void mostrar_estadisticas(const char *ruta) {
    struct stat info;
    size_t directorios = 0;
    size_t elementos;
    off_t tamano;

    if (lstat(ruta, &info) == -1) {
        perror("No se pudo analizar la ruta");
        return;
    }

    elementos = contar_elementos(ruta, &directorios);
    tamano = tamano_total(ruta);

    printf("\nEstadisticas de %s\n", ruta);
    printf("- Tipo: %s\n", S_ISDIR(info.st_mode) ? "Directorio" : "Archivo");
    printf("- Tamano total: %lld bytes\n", (long long)tamano);
    printf("- Elementos: %zu\n", elementos);
    printf("- Directorios: %zu\n", directorios);
}

static off_t tamano_total(const char *ruta) {
    struct stat info;
    DIR *directorio;
    struct dirent *entrada;
    char ruta_hija[PATH_MAX];
    off_t total = 0;

    if (lstat(ruta, &info) == -1) {
        return 0;
    }

    if (S_ISREG(info.st_mode)) {
        return info.st_size;
    }

    if (!S_ISDIR(info.st_mode)) {
        return 0;
    }

    directorio = opendir(ruta);
    if (directorio == NULL) {
        return 0;
    }

    while ((entrada = readdir(directorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (snprintf(ruta_hija, sizeof(ruta_hija), "%s/%s", ruta, entrada->d_name) >= (int)sizeof(ruta_hija)) {
            continue;
        }

        total += tamano_total(ruta_hija);
    }

    closedir(directorio);
    return total;
}

static size_t contar_elementos(const char *ruta, size_t *directorios) {
    DIR *directorio;
    struct dirent *entrada;
    struct stat info;
    char ruta_hija[PATH_MAX];
    size_t elementos = 0;

    if (lstat(ruta, &info) == -1) {
        return 0;
    }

    if (!S_ISDIR(info.st_mode)) {
        return 1;
    }

    if (directorios != NULL) {
        *directorios += 1;
    }

    directorio = opendir(ruta);
    if (directorio == NULL) {
        return 0;
    }

    while ((entrada = readdir(directorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (snprintf(ruta_hija, sizeof(ruta_hija), "%s/%s", ruta, entrada->d_name) >= (int)sizeof(ruta_hija)) {
            continue;
        }

        if (lstat(ruta_hija, &info) == -1) {
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            elementos += contar_elementos(ruta_hija, directorios);
        } else {
            elementos += 1;
        }
    }

    closedir(directorio);
    return elementos;
}
