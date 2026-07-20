#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "comandos.h"
#include "util.h"

#define HISTORIAL_COMANDOS "historial/comandos.log"

static void menu_comandos_ejecutar(void);
static void menu_comandos_historial(void);
static void menu_comandos_limpiar_historial(void);
static int ejecutar_comando(const char *comando, char *salida, size_t tamano_salida, char *errores, size_t tamano_errores, int *codigo_salida);
static void registrar_historial(const char *comando, int codigo_salida);
static int leer_archivo(const char *ruta, char *buffer, size_t tamano);

void menu_comandos(){

    int opcion;

    do {
        limpiar_pantalla();

        printf("=====================================\n");
        printf("       COMANDOS LINUX\n");
        printf("=====================================\n\n");

        printf("1. Ejecutar comando\n");
        printf("2. Ver historial\n");
        printf("3. Limpiar historial\n");
        printf("0. Volver\n\n");

        if (!leer_entero("Seleccione una opcion: ", &opcion)) {
            printf("\nEntrada invalida.\n");
            pausar();
            continue;
        }

        switch (opcion) {
            case 1:
                menu_comandos_ejecutar();
                break;
            case 2:
                menu_comandos_historial();
                break;
            case 3:
                menu_comandos_limpiar_historial();
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

static void menu_comandos_ejecutar(void) {
    char comando[512];
    char salida[8192];
    char errores[8192];
    int codigo_salida;

    if (!leer_texto("Ingrese el comando Linux: ", comando, sizeof(comando))) {
        printf("\nNo se ingreso ningun comando.\n");
        pausar();
        return;
    }

    salida[0] = '\0';
    errores[0] = '\0';

    if (ejecutar_comando(comando, salida, sizeof(salida), errores, sizeof(errores), &codigo_salida) == -1) {
        printf("\nNo se pudo ejecutar el comando.\n");
        pausar();
        return;
    }

    printf("\n--- Salida estandar ---\n");
    if (salida[0] != '\0') {
        printf("%s\n", salida);
    } else {
        printf("(sin salida)\n");
    }

    printf("\n--- Errores ---\n");
    if (errores[0] != '\0') {
        printf("%s\n", errores);
    } else {
        printf("(sin errores)\n");
    }

    printf("\nCodigo de salida: %d\n", codigo_salida);
    registrar_historial(comando, codigo_salida);
    pausar();
}

static void menu_comandos_historial(void) {
    char contenido[16384];

    if (leer_archivo(HISTORIAL_COMANDOS, contenido, sizeof(contenido)) == -1) {
        printf("\nNo hay historial registrado.\n");
        pausar();
        return;
    }

    printf("\nHistorial de comandos:\n\n%s\n", contenido);
    pausar();
}

static void menu_comandos_limpiar_historial(void) {
    FILE *archivo;

    archivo = fopen(HISTORIAL_COMANDOS, "w");
    if (archivo == NULL) {
        perror("No se pudo limpiar el historial");
        pausar();
        return;
    }

    fclose(archivo);
    printf("\nHistorial limpiado correctamente.\n");
    pausar();
}

static int ejecutar_comando(const char *comando, char *salida, size_t tamano_salida, char *errores, size_t tamano_errores, int *codigo_salida) {
    char archivo_salida[] = "/tmp/admin_linux_out_XXXXXX";
    char archivo_error[] = "/tmp/admin_linux_err_XXXXXX";
    int fd_salida;
    int fd_error;
    pid_t pid;
    int estado;

    fd_salida = mkstemp(archivo_salida);
    if (fd_salida == -1) {
        perror("No se pudo crear el archivo temporal de salida");
        return -1;
    }

    fd_error = mkstemp(archivo_error);
    if (fd_error == -1) {
        perror("No se pudo crear el archivo temporal de errores");
        close(fd_salida);
        unlink(archivo_salida);
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("No se pudo crear el proceso hijo");
        close(fd_salida);
        close(fd_error);
        unlink(archivo_salida);
        unlink(archivo_error);
        return -1;
    }

    if (pid == 0) {
        dup2(fd_salida, STDOUT_FILENO);
        dup2(fd_error, STDERR_FILENO);

        close(fd_salida);
        close(fd_error);

        execl("/bin/sh", "sh", "-c", comando, (char *)NULL);
        perror("No se pudo ejecutar el comando");
        _exit(127);
    }

    close(fd_salida);
    close(fd_error);

    if (waitpid(pid, &estado, 0) == -1) {
        perror("Error al esperar el proceso hijo");
        unlink(archivo_salida);
        unlink(archivo_error);
        return -1;
    }

    if (leer_archivo(archivo_salida, salida, tamano_salida) == -1) {
        salida[0] = '\0';
    }

    if (leer_archivo(archivo_error, errores, tamano_errores) == -1) {
        errores[0] = '\0';
    }

    unlink(archivo_salida);
    unlink(archivo_error);

    if (WIFEXITED(estado)) {
        *codigo_salida = WEXITSTATUS(estado);
    } else if (WIFSIGNALED(estado)) {
        *codigo_salida = 128 + WTERMSIG(estado);
    } else {
        *codigo_salida = -1;
    }

    return 0;
}

static void registrar_historial(const char *comando, int codigo_salida) {
    FILE *archivo;
    time_t ahora;
    struct tm *tiempo_local;
    char fecha[64];

    archivo = fopen(HISTORIAL_COMANDOS, "a");
    if (archivo == NULL) {
        perror("No se pudo registrar el historial");
        return;
    }

    ahora = time(NULL);
    tiempo_local = localtime(&ahora);

    if (tiempo_local != NULL) {
        strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tiempo_local);
    } else {
        snprintf(fecha, sizeof(fecha), "fecha_desconocida");
    }

    fprintf(archivo, "[%s] comando: %s | codigo: %d\n", fecha, comando, codigo_salida);
    fclose(archivo);
}

static int leer_archivo(const char *ruta, char *buffer, size_t tamano) {
    FILE *archivo;
    size_t leidos;

    archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        return -1;
    }

    leidos = fread(buffer, 1, tamano - 1, archivo);
    buffer[leidos] = '\0';

    fclose(archivo);
    return 0;
}
