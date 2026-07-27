#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "backup.h"
#include "util.h"

#define DIR_BACKUPS "backups"

typedef struct {
    char relpath[PATH_MAX];
    time_t mtime;
    off_t tamano;
} EntradaManifiesto;

typedef struct {
    EntradaManifiesto *items;
    size_t cantidad;
    size_t capacidad;
} ListaManifiesto;

typedef struct {
    char relpath[PATH_MAX];
    int version;
} EntradaRestauracion;

typedef struct {
    EntradaRestauracion *items;
    size_t cantidad;
    size_t capacidad;
} ListaRestauracion;

static void menu_backup_crear(void);
static void menu_backup_listar_trabajos(void);
static void menu_backup_ver_versiones(void);
static void menu_backup_restaurar(void);

static void ejecutar_respaldo(const char *trabajo, const char *origen);
static void recorrer_origen(const char *base_origen, const char *subruta,
                             ListaManifiesto *anterior, ListaManifiesto *nuevo,
                             const char *ruta_version, FILE *agregados, int *num_agregados);

static int cargar_manifiesto(const char *ruta_archivo, ListaManifiesto *lista);
static int guardar_manifiesto(const char *ruta_archivo, ListaManifiesto *lista);
static void agregar_a_manifiesto(ListaManifiesto *lista, const char *relpath, time_t mtime, off_t tamano);
static int buscar_en_manifiesto(ListaManifiesto *lista, const char *relpath);
static void liberar_manifiesto(ListaManifiesto *lista);

static void agregar_a_restauracion(ListaRestauracion *lista, const char *relpath, int version);
static int buscar_en_restauracion(ListaRestauracion *lista, const char *relpath);
static void quitar_de_restauracion(ListaRestauracion *lista, const char *relpath);
static void liberar_restauracion(ListaRestauracion *lista);

static int crear_ruta_recursiva(const char *ruta);
static int asegurar_directorio_padre(const char *ruta_archivo);
static int copiar_archivo_simple(const char *origen, const char *destino);
static int obtener_ultima_version(const char *trabajo);
static int existe_trabajo(const char *trabajo);

void menu_backup(){

    int opcion;

    do {
        limpiar_pantalla();

        printf("=====================================\n");
        printf("          RESPALDOS\n");
        printf("=====================================\n\n");

        printf("1. Crear/actualizar respaldo incremental\n");
        printf("2. Listar trabajos de respaldo\n");
        printf("3. Ver versiones de un trabajo\n");
        printf("4. Restaurar una version\n");
        printf("0. Volver\n\n");

        if (!leer_entero("Seleccione una opcion: ", &opcion)) {
            printf("\nEntrada invalida.\n");
            pausar();
            continue;
        }

        switch (opcion) {
            case 1:
                menu_backup_crear();
                break;
            case 2:
                menu_backup_listar_trabajos();
                break;
            case 3:
                menu_backup_ver_versiones();
                break;
            case 4:
                menu_backup_restaurar();
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

static void menu_backup_crear(void) {
    char trabajo[256];
    char origen[PATH_MAX];
    char ruta_trabajo[PATH_MAX];
    char ruta_origen_txt[PATH_MAX + 64];
    struct stat info;
    FILE *f;

    limpiar_pantalla();
    printf("=====================================\n");
    printf("     CREAR / ACTUALIZAR RESPALDO\n");
    printf("=====================================\n\n");

    if (!leer_texto("Nombre del trabajo de respaldo: ", trabajo, sizeof(trabajo))) {
        pausar();
        return;
    }

    if (crear_ruta_recursiva(DIR_BACKUPS) == -1) {
        perror("No se pudo crear el directorio de respaldos");
        pausar();
        return;
    }

    snprintf(ruta_trabajo, sizeof(ruta_trabajo), "%s/%s", DIR_BACKUPS, trabajo);

    if (existe_trabajo(trabajo)) {

        snprintf(ruta_origen_txt, sizeof(ruta_origen_txt), "%s/origen.txt", ruta_trabajo);
        f = fopen(ruta_origen_txt, "r");
        if (f == NULL || fgets(origen, sizeof(origen), f) == NULL) {
            printf("No se pudo leer el origen registrado para este trabajo.\n");
            if (f != NULL) {
                fclose(f);
            }
            pausar();
            return;
        }
        fclose(f);

        {
            char *salto = strchr(origen, '\n');
            if (salto != NULL) {
                *salto = '\0';
            }
        }

        printf("Trabajo existente. Se usara el origen ya registrado:\n%s\n", origen);

    } else {

        if (!leer_texto("Ruta de origen a respaldar: ", origen, sizeof(origen))) {
            pausar();
            return;
        }

        if (stat(origen, &info) == -1 || !S_ISDIR(info.st_mode)) {
            printf("La ruta de origen no existe o no es un directorio.\n");
            pausar();
            return;
        }

        if (crear_ruta_recursiva(ruta_trabajo) == -1) {
            perror("No se pudo crear el directorio del trabajo");
            pausar();
            return;
        }

        snprintf(ruta_origen_txt, sizeof(ruta_origen_txt), "%s/origen.txt", ruta_trabajo);
        f = fopen(ruta_origen_txt, "w");
        if (f != NULL) {
            fprintf(f, "%s\n", origen);
            fclose(f);
        }
    }

    ejecutar_respaldo(trabajo, origen);

    pausar();
}

static void ejecutar_respaldo(const char *trabajo, const char *origen) {
    char ruta_trabajo[PATH_MAX];
    char ruta_manifiesto[PATH_MAX + 64];
    char ruta_versiones_txt[PATH_MAX + 64];
    char ruta_version[PATH_MAX + 32];
    char ruta_datos[PATH_MAX + 64];
    char ruta_agregados[PATH_MAX + 64];
    char ruta_eliminados[PATH_MAX + 64];
    char ruta_info[PATH_MAX + 64];
    ListaManifiesto anterior;
    ListaManifiesto nuevo;
    FILE *f_agregados;
    FILE *f_eliminados;
    FILE *f_info;
    int num_agregados = 0;
    int num_eliminados = 0;
    int version_nueva;
    size_t i;
    time_t ahora;
    char fecha_texto[64];

    memset(&anterior, 0, sizeof(anterior));
    memset(&nuevo, 0, sizeof(nuevo));

    snprintf(ruta_trabajo, sizeof(ruta_trabajo), "%s/%s", DIR_BACKUPS, trabajo);
    snprintf(ruta_manifiesto, sizeof(ruta_manifiesto), "%s/manifiesto.txt", ruta_trabajo);
    snprintf(ruta_versiones_txt, sizeof(ruta_versiones_txt), "%s/version_actual.txt", ruta_trabajo);

    cargar_manifiesto(ruta_manifiesto, &anterior);

    version_nueva = obtener_ultima_version(trabajo) + 1;

    snprintf(ruta_version, sizeof(ruta_version), "%s/v%d", ruta_trabajo, version_nueva);
    snprintf(ruta_datos, sizeof(ruta_datos), "%s/datos", ruta_version);

    if (crear_ruta_recursiva(ruta_datos) == -1) {
        perror("No se pudo crear el directorio de la version");
        liberar_manifiesto(&anterior);
        return;
    }

    snprintf(ruta_agregados, sizeof(ruta_agregados), "%s/agregados.txt", ruta_version);
    snprintf(ruta_eliminados, sizeof(ruta_eliminados), "%s/eliminados.txt", ruta_version);
    snprintf(ruta_info, sizeof(ruta_info), "%s/info.txt", ruta_version);

    f_agregados = fopen(ruta_agregados, "w");
    if (f_agregados == NULL) {
        perror("No se pudo crear el registro de archivos agregados");
        liberar_manifiesto(&anterior);
        return;
    }

    recorrer_origen(origen, "", &anterior, &nuevo, ruta_version, f_agregados, &num_agregados);
    fclose(f_agregados);

    f_eliminados = fopen(ruta_eliminados, "w");
    if (f_eliminados != NULL) {
        for (i = 0; i < anterior.cantidad; i++) {
            if (buscar_en_manifiesto(&nuevo, anterior.items[i].relpath) == -1) {
                fprintf(f_eliminados, "%s\n", anterior.items[i].relpath);
                num_eliminados++;
            }
        }
        fclose(f_eliminados);
    }

    ahora = time(NULL);
    strftime(fecha_texto, sizeof(fecha_texto), "%Y-%m-%d %H:%M:%S", localtime(&ahora));

    f_info = fopen(ruta_info, "w");
    if (f_info != NULL) {
        fprintf(f_info, "version=%d\n", version_nueva);
        fprintf(f_info, "fecha=%s\n", fecha_texto);
        fprintf(f_info, "origen=%s\n", origen);
        fprintf(f_info, "agregados=%d\n", num_agregados);
        fprintf(f_info, "eliminados=%d\n", num_eliminados);
        fclose(f_info);
    }

    guardar_manifiesto(ruta_manifiesto, &nuevo);

    f_info = fopen(ruta_versiones_txt, "w");
    if (f_info != NULL) {
        fprintf(f_info, "%d\n", version_nueva);
        fclose(f_info);
    }

    printf("\nRespaldo completado: version %d\n", version_nueva);
    printf("Archivos nuevos o modificados copiados: %d\n", num_agregados);
    printf("Archivos eliminados detectados: %d\n", num_eliminados);

    liberar_manifiesto(&anterior);
    liberar_manifiesto(&nuevo);
}

static void recorrer_origen(const char *base_origen, const char *subruta,
                             ListaManifiesto *anterior, ListaManifiesto *nuevo,
                             const char *ruta_version, FILE *agregados, int *num_agregados) {
    char ruta_absoluta[PATH_MAX];
    DIR *dir;
    struct dirent *entrada;
    struct stat info;

    if (subruta[0] == '\0') {
        snprintf(ruta_absoluta, sizeof(ruta_absoluta), "%s", base_origen);
    } else {
        snprintf(ruta_absoluta, sizeof(ruta_absoluta), "%s/%s", base_origen, subruta);
    }

    dir = opendir(ruta_absoluta);
    if (dir == NULL) {
        return;
    }

    while ((entrada = readdir(dir)) != NULL) {
        char nueva_subruta[PATH_MAX];
        char ruta_completa[PATH_MAX];

        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (subruta[0] == '\0') {
            if (snprintf(nueva_subruta, sizeof(nueva_subruta), "%s", entrada->d_name) >= (int)sizeof(nueva_subruta)) {
                continue;
            }
        } else {
            if (snprintf(nueva_subruta, sizeof(nueva_subruta), "%s/%s", subruta, entrada->d_name) >= (int)sizeof(nueva_subruta)) {
                continue;
            }
        }

        if (snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", base_origen, nueva_subruta) >= (int)sizeof(ruta_completa)) {
            continue;
        }

        if (lstat(ruta_completa, &info) == -1) {
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            recorrer_origen(base_origen, nueva_subruta, anterior, nuevo, ruta_version, agregados, num_agregados);
        } else if (S_ISREG(info.st_mode)) {
            int indice_anterior = buscar_en_manifiesto(anterior, nueva_subruta);
            int modificado = 1;

            if (indice_anterior != -1 &&
                anterior->items[indice_anterior].mtime == info.st_mtime &&
                anterior->items[indice_anterior].tamano == info.st_size) {
                modificado = 0;
            }

            if (modificado) {
                char ruta_destino[PATH_MAX];

                if (snprintf(ruta_destino, sizeof(ruta_destino), "%s/datos/%s", ruta_version, nueva_subruta) < (int)sizeof(ruta_destino)) {
                    if (copiar_archivo_simple(ruta_completa, ruta_destino) == 0) {
                        fprintf(agregados, "%s\n", nueva_subruta);
                        (*num_agregados)++;
                    }
                }
            }

            agregar_a_manifiesto(nuevo, nueva_subruta, info.st_mtime, info.st_size);
        }
    }

    closedir(dir);
}

static void menu_backup_listar_trabajos(void) {
    DIR *dir;
    struct dirent *entrada;
    struct stat info;
    char ruta[PATH_MAX];
    int encontrados = 0;

    limpiar_pantalla();
    printf("=====================================\n");
    printf("     TRABAJOS DE RESPALDO\n");
    printf("=====================================\n\n");

    dir = opendir(DIR_BACKUPS);
    if (dir == NULL) {
        printf("Aun no se ha creado ningun respaldo.\n");
        pausar();
        return;
    }

    while ((entrada = readdir(dir)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        if (snprintf(ruta, sizeof(ruta), "%s/%s", DIR_BACKUPS, entrada->d_name) >= (int)sizeof(ruta)) {
            continue;
        }

        if (stat(ruta, &info) == -1 || !S_ISDIR(info.st_mode)) {
            continue;
        }

        printf("- %s (version actual: %d)\n", entrada->d_name, obtener_ultima_version(entrada->d_name));
        encontrados++;
    }

    closedir(dir);

    if (encontrados == 0) {
        printf("Aun no se ha creado ningun respaldo.\n");
    }

    pausar();
}

static void menu_backup_ver_versiones(void) {
    char trabajo[256];
    int ultima;
    int i;

    limpiar_pantalla();
    printf("=====================================\n");
    printf("     VERSIONES DE UN TRABAJO\n");
    printf("=====================================\n\n");

    if (!leer_texto("Nombre del trabajo: ", trabajo, sizeof(trabajo))) {
        pausar();
        return;
    }

    if (!existe_trabajo(trabajo)) {
        printf("El trabajo indicado no existe.\n");
        pausar();
        return;
    }

    ultima = obtener_ultima_version(trabajo);

    if (ultima == 0) {
        printf("El trabajo no tiene versiones registradas todavia.\n");
        pausar();
        return;
    }

    printf("\nVersiones disponibles para '%s':\n\n", trabajo);

    for (i = 1; i <= ultima; i++) {
        char ruta_info[PATH_MAX];
        FILE *f;
        char linea[512];

        snprintf(ruta_info, sizeof(ruta_info), "%s/%s/v%d/info.txt", DIR_BACKUPS, trabajo, i);
        f = fopen(ruta_info, "r");

        printf("Version %d:\n", i);
        if (f != NULL) {
            while (fgets(linea, sizeof(linea), f) != NULL) {
                printf("  %s", linea);
            }
            fclose(f);
        } else {
            printf("  (sin informacion disponible)\n");
        }
        printf("\n");
    }

    pausar();
}

static void menu_backup_restaurar(void) {
    char trabajo[256];
    char destino[PATH_MAX];
    int version_objetivo;
    int ultima;
    int v;
    ListaRestauracion resultado;
    size_t i;
    int copiados = 0;

    memset(&resultado, 0, sizeof(resultado));

    limpiar_pantalla();
    printf("=====================================\n");
    printf("       RESTAURAR VERSION\n");
    printf("=====================================\n\n");

    if (!leer_texto("Nombre del trabajo: ", trabajo, sizeof(trabajo))) {
        pausar();
        return;
    }

    if (!existe_trabajo(trabajo)) {
        printf("El trabajo indicado no existe.\n");
        pausar();
        return;
    }

    ultima = obtener_ultima_version(trabajo);
    if (ultima == 0) {
        printf("El trabajo no tiene versiones registradas todavia.\n");
        pausar();
        return;
    }

    printf("Este trabajo tiene %d version(es) disponibles.\n", ultima);

    if (!leer_entero("Version a restaurar: ", &version_objetivo)) {
        printf("Entrada invalida.\n");
        pausar();
        return;
    }

    if (version_objetivo < 1 || version_objetivo > ultima) {
        printf("Version fuera de rango.\n");
        pausar();
        return;
    }

    if (!leer_texto("Directorio de destino para restaurar: ", destino, sizeof(destino))) {
        pausar();
        return;
    }

    if (crear_ruta_recursiva(destino) == -1) {
        perror("No se pudo crear el directorio de destino");
        pausar();
        return;
    }

    for (v = 1; v <= version_objetivo; v++) {
        char ruta_agregados[PATH_MAX];
        char ruta_eliminados[PATH_MAX];
        FILE *f;
        char linea[PATH_MAX];

        snprintf(ruta_agregados, sizeof(ruta_agregados), "%s/%s/v%d/agregados.txt", DIR_BACKUPS, trabajo, v);
        f = fopen(ruta_agregados, "r");
        if (f != NULL) {
            while (fgets(linea, sizeof(linea), f) != NULL) {
                char *salto = strchr(linea, '\n');
                if (salto != NULL) {
                    *salto = '\0';
                }
                if (linea[0] != '\0') {
                    agregar_a_restauracion(&resultado, linea, v);
                }
            }
            fclose(f);
        }

        snprintf(ruta_eliminados, sizeof(ruta_eliminados), "%s/%s/v%d/eliminados.txt", DIR_BACKUPS, trabajo, v);
        f = fopen(ruta_eliminados, "r");
        if (f != NULL) {
            while (fgets(linea, sizeof(linea), f) != NULL) {
                char *salto = strchr(linea, '\n');
                if (salto != NULL) {
                    *salto = '\0';
                }
                if (linea[0] != '\0') {
                    quitar_de_restauracion(&resultado, linea);
                }
            }
            fclose(f);
        }
    }

    for (i = 0; i < resultado.cantidad; i++) {
        char ruta_origen_dato[PATH_MAX];
        char ruta_destino_final[PATH_MAX];

        if (snprintf(ruta_origen_dato, sizeof(ruta_origen_dato), "%s/%s/v%d/datos/%s",
                     DIR_BACKUPS, trabajo, resultado.items[i].version, resultado.items[i].relpath) >= (int)sizeof(ruta_origen_dato)) {
            continue;
        }

        if (snprintf(ruta_destino_final, sizeof(ruta_destino_final), "%s/%s", destino, resultado.items[i].relpath) >= (int)sizeof(ruta_destino_final)) {
            continue;
        }

        if (copiar_archivo_simple(ruta_origen_dato, ruta_destino_final) == 0) {
            copiados++;
        }
    }

    printf("\nRestauracion completada (version objetivo: %d).\n", version_objetivo);
    printf("Archivos restaurados: %d de %zu\n", copiados, resultado.cantidad);

    liberar_restauracion(&resultado);

    pausar();
}

static int existe_trabajo(const char *trabajo) {
    char ruta[PATH_MAX];
    struct stat info;

    snprintf(ruta, sizeof(ruta), "%s/%s", DIR_BACKUPS, trabajo);
    return (stat(ruta, &info) == 0 && S_ISDIR(info.st_mode));
}

static int obtener_ultima_version(const char *trabajo) {
    char ruta[PATH_MAX];
    FILE *f;
    int version = 0;

    snprintf(ruta, sizeof(ruta), "%s/%s/version_actual.txt", DIR_BACKUPS, trabajo);
    f = fopen(ruta, "r");
    if (f != NULL) {
        if (fscanf(f, "%d", &version) != 1) {
            version = 0;
        }
        fclose(f);
    }

    return version;
}

static int crear_ruta_recursiva(const char *ruta) {
    char copia[PATH_MAX];
    char *p;
    size_t len;

    len = strlen(ruta);
    if (len == 0 || len >= sizeof(copia)) {
        return -1;
    }
    strcpy(copia, ruta);

    for (p = copia + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(copia, 0755) == -1 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(copia, 0755) == -1 && errno != EEXIST) {
        return -1;
    }

    return 0;
}

static int asegurar_directorio_padre(const char *ruta_archivo) {
    char copia[PATH_MAX];
    char *ultima_barra;

    if (strlen(ruta_archivo) >= sizeof(copia)) {
        return -1;
    }
    strcpy(copia, ruta_archivo);

    ultima_barra = strrchr(copia, '/');
    if (ultima_barra == NULL) {
        return 0;
    }

    *ultima_barra = '\0';
    if (copia[0] == '\0') {
        return 0;
    }

    return crear_ruta_recursiva(copia);
}

static int copiar_archivo_simple(const char *origen, const char *destino) {
    int fd_origen;
    int fd_destino;
    ssize_t leidos;
    char buffer[8192];

    fd_origen = open(origen, O_RDONLY);
    if (fd_origen == -1) {
        return -1;
    }

    if (asegurar_directorio_padre(destino) == -1) {
        close(fd_origen);
        return -1;
    }

    fd_destino = open(destino, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_destino == -1) {
        close(fd_origen);
        return -1;
    }

    while ((leidos = read(fd_origen, buffer, sizeof(buffer))) > 0) {
        if (write(fd_destino, buffer, (size_t)leidos) != leidos) {
            close(fd_origen);
            close(fd_destino);
            return -1;
        }
    }

    close(fd_origen);
    close(fd_destino);

    return (leidos == -1) ? -1 : 0;
}

static void agregar_a_manifiesto(ListaManifiesto *lista, const char *relpath, time_t mtime, off_t tamano) {
    if (lista->cantidad == lista->capacidad) {
        size_t nueva_cap = (lista->capacidad == 0) ? 64 : lista->capacidad * 2;
        EntradaManifiesto *tmp = realloc(lista->items, nueva_cap * sizeof(EntradaManifiesto));
        if (tmp == NULL) {
            return;
        }
        lista->items = tmp;
        lista->capacidad = nueva_cap;
    }

    strncpy(lista->items[lista->cantidad].relpath, relpath, PATH_MAX - 1);
    lista->items[lista->cantidad].relpath[PATH_MAX - 1] = '\0';
    lista->items[lista->cantidad].mtime = mtime;
    lista->items[lista->cantidad].tamano = tamano;
    lista->cantidad++;
}

static int buscar_en_manifiesto(ListaManifiesto *lista, const char *relpath) {
    size_t i;

    for (i = 0; i < lista->cantidad; i++) {
        if (strcmp(lista->items[i].relpath, relpath) == 0) {
            return (int)i;
        }
    }

    return -1;
}

static void liberar_manifiesto(ListaManifiesto *lista) {
    free(lista->items);
    lista->items = NULL;
    lista->cantidad = 0;
    lista->capacidad = 0;
}

static int cargar_manifiesto(const char *ruta_archivo, ListaManifiesto *lista) {
    FILE *f;
    char linea[PATH_MAX + 64];

    f = fopen(ruta_archivo, "r");
    if (f == NULL) {
        return -1;
    }

    while (fgets(linea, sizeof(linea), f) != NULL) {
        char relpath[PATH_MAX];
        char *tab1;
        char *tab2;
        size_t len_relpath;
        long long mtime_val;
        long long tamano_val;

        tab1 = strchr(linea, '\t');
        if (tab1 == NULL) {
            continue;
        }
        tab2 = strchr(tab1 + 1, '\t');
        if (tab2 == NULL) {
            continue;
        }

        len_relpath = (size_t)(tab1 - linea);
        if (len_relpath >= sizeof(relpath)) {
            continue;
        }
        memcpy(relpath, linea, len_relpath);
        relpath[len_relpath] = '\0';

        mtime_val = atoll(tab1 + 1);
        tamano_val = atoll(tab2 + 1);

        agregar_a_manifiesto(lista, relpath, (time_t)mtime_val, (off_t)tamano_val);
    }

    fclose(f);
    return 0;
}

static int guardar_manifiesto(const char *ruta_archivo, ListaManifiesto *lista) {
    FILE *f;
    size_t i;

    f = fopen(ruta_archivo, "w");
    if (f == NULL) {
        return -1;
    }

    for (i = 0; i < lista->cantidad; i++) {
        fprintf(f, "%s\t%lld\t%lld\n", lista->items[i].relpath,
                (long long)lista->items[i].mtime,
                (long long)lista->items[i].tamano);
    }

    fclose(f);
    return 0;
}

static void agregar_a_restauracion(ListaRestauracion *lista, const char *relpath, int version) {
    int indice = buscar_en_restauracion(lista, relpath);

    if (indice != -1) {
        lista->items[indice].version = version;
        return;
    }

    if (lista->cantidad == lista->capacidad) {
        size_t nueva_cap = (lista->capacidad == 0) ? 64 : lista->capacidad * 2;
        EntradaRestauracion *tmp = realloc(lista->items, nueva_cap * sizeof(EntradaRestauracion));
        if (tmp == NULL) {
            return;
        }
        lista->items = tmp;
        lista->capacidad = nueva_cap;
    }

    strncpy(lista->items[lista->cantidad].relpath, relpath, PATH_MAX - 1);
    lista->items[lista->cantidad].relpath[PATH_MAX - 1] = '\0';
    lista->items[lista->cantidad].version = version;
    lista->cantidad++;
}

static int buscar_en_restauracion(ListaRestauracion *lista, const char *relpath) {
    size_t i;

    for (i = 0; i < lista->cantidad; i++) {
        if (strcmp(lista->items[i].relpath, relpath) == 0) {
            return (int)i;
        }
    }

    return -1;
}

static void quitar_de_restauracion(ListaRestauracion *lista, const char *relpath) {
    int indice = buscar_en_restauracion(lista, relpath);
    size_t i;

    if (indice == -1) {
        return;
    }

    for (i = (size_t)indice; i + 1 < lista->cantidad; i++) {
        lista->items[i] = lista->items[i + 1];
    }
    lista->cantidad--;
}

static void liberar_restauracion(ListaRestauracion *lista) {
    free(lista->items);
    lista->items = NULL;
    lista->cantidad = 0;
    lista->capacidad = 0;
}
