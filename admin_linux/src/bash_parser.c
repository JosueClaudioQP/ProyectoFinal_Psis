#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "bash_parser.h"
#include "util.h"

typedef struct {
    char nombre[128];
} Identificador;

typedef struct {
    Identificador *items;
    size_t cantidad;
    size_t capacidad;
} ListaIdentificadores;

typedef struct {
    int lineas_totales;
    int lineas_comentario;
    int lineas_en_blanco;
    int lineas_codigo;
    int bucles_for;
    int bucles_while;
    int bucles_until;
    int condicionales_if;
    int estructuras_case;
    ListaIdentificadores variables;
    ListaIdentificadores funciones;
} ReporteScript;

static void menu_bash_analizar(int guardar);

static int analizar_script(const char *ruta, ReporteScript *reporte);
static void imprimir_reporte(FILE *salida, const char *ruta, ReporteScript *reporte);
static void liberar_reporte(ReporteScript *reporte);

static char *recortar_espacios(char *texto);
static void quitar_comentario(char *linea);
static int palabra_al_inicio(const char *linea, const char *palabra);

static int es_nombre_variable_valido(const char *nombre);
static int detectar_funcion(const char *linea, char *nombre_funcion, size_t tam);
static int detectar_asignacion_variable(const char *linea, char *nombre_var, size_t tam);

static int existe_identificador(ListaIdentificadores *lista, const char *nombre);
static void agregar_identificador(ListaIdentificadores *lista, const char *nombre);

void menu_bash(){

    int opcion;

    do {
        limpiar_pantalla();

        printf("=====================================\n");
        printf("      ANALIZADOR BASH\n");
        printf("=====================================\n\n");

        printf("1. Analizar script (.sh)\n");
        printf("2. Analizar y guardar reporte en archivo\n");
        printf("0. Volver\n\n");

        if (!leer_entero("Seleccione una opcion: ", &opcion)) {
            printf("\nEntrada invalida.\n");
            pausar();
            continue;
        }

        switch (opcion) {
            case 1:
                menu_bash_analizar(0);
                break;
            case 2:
                menu_bash_analizar(1);
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

static void menu_bash_analizar(int guardar) {
    char ruta[PATH_MAX];
    ReporteScript reporte;

    limpiar_pantalla();
    printf("=====================================\n");
    printf("      ANALIZADOR BASH\n");
    printf("=====================================\n\n");

    if (!leer_texto("Ruta del script (.sh) a analizar: ", ruta, sizeof(ruta))) {
        pausar();
        return;
    }

    if (analizar_script(ruta, &reporte) == -1) {
        perror("No se pudo abrir el script indicado");
        pausar();
        return;
    }

    imprimir_reporte(stdout, ruta, &reporte);

    if (guardar) {
        char ruta_reporte[PATH_MAX];
        char nombre_base[PATH_MAX];
        const char *ultima_barra;
        FILE *f;

        ultima_barra = strrchr(ruta, '/');
        if (ultima_barra != NULL) {
            strncpy(nombre_base, ultima_barra + 1, sizeof(nombre_base) - 1);
        } else {
            strncpy(nombre_base, ruta, sizeof(nombre_base) - 1);
        }
        nombre_base[sizeof(nombre_base) - 1] = '\0';

        mkdir("logs", 0755);

        if (snprintf(ruta_reporte, sizeof(ruta_reporte), "logs/%s_reporte.txt", nombre_base) < (int)sizeof(ruta_reporte)) {
            f = fopen(ruta_reporte, "w");
            if (f != NULL) {
                imprimir_reporte(f, ruta, &reporte);
                fclose(f);
                printf("\nReporte guardado en: %s\n", ruta_reporte);
            } else {
                printf("\nNo se pudo guardar el reporte en disco.\n");
            }
        }
    }

    liberar_reporte(&reporte);

    pausar();
}

static int analizar_script(const char *ruta, ReporteScript *reporte) {
    FILE *f;
    char linea[4096];
    char nombre_temp[128];

    memset(reporte, 0, sizeof(*reporte));

    f = fopen(ruta, "r");
    if (f == NULL) {
        return -1;
    }

    while (fgets(linea, sizeof(linea), f) != NULL) {
        char *sin_espacios;
        char *salto;

        reporte->lineas_totales++;

        salto = strchr(linea, '\n');
        if (salto != NULL) {
            *salto = '\0';
        }

        sin_espacios = recortar_espacios(linea);

        if (sin_espacios[0] == '\0') {
            reporte->lineas_en_blanco++;
            continue;
        }

        if (sin_espacios[0] == '#') {
            reporte->lineas_comentario++;
            continue;
        }

        quitar_comentario(sin_espacios);
        sin_espacios = recortar_espacios(sin_espacios);

        if (sin_espacios[0] == '\0') {
            reporte->lineas_comentario++;
            continue;
        }

        reporte->lineas_codigo++;

        if (palabra_al_inicio(sin_espacios, "for")) {
            reporte->bucles_for++;
        } else if (palabra_al_inicio(sin_espacios, "while")) {
            reporte->bucles_while++;
        } else if (palabra_al_inicio(sin_espacios, "until")) {
            reporte->bucles_until++;
        } else if (palabra_al_inicio(sin_espacios, "if")) {
            reporte->condicionales_if++;
        } else if (palabra_al_inicio(sin_espacios, "elif")) {
            reporte->condicionales_if++;
        } else if (palabra_al_inicio(sin_espacios, "case")) {
            reporte->estructuras_case++;
        }

        if (detectar_funcion(sin_espacios, nombre_temp, sizeof(nombre_temp))) {
            agregar_identificador(&reporte->funciones, nombre_temp);
        } else if (detectar_asignacion_variable(sin_espacios, nombre_temp, sizeof(nombre_temp))) {
            agregar_identificador(&reporte->variables, nombre_temp);
        }
    }

    fclose(f);
    return 0;
}

static char *recortar_espacios(char *texto) {
    char *inicio = texto;
    char *fin;

    while (*inicio == ' ' || *inicio == '\t') {
        inicio++;
    }

    if (*inicio == '\0') {
        return inicio;
    }

    fin = inicio + strlen(inicio) - 1;
    while (fin > inicio && (*fin == ' ' || *fin == '\t' || *fin == '\r')) {
        *fin = '\0';
        fin--;
    }

    return inicio;
}

static void quitar_comentario(char *linea) {
    int en_comillas_simples = 0;
    int en_comillas_dobles = 0;
    char *p;

    for (p = linea; *p != '\0'; p++) {
        if (*p == '\'' && !en_comillas_dobles) {
            en_comillas_simples = !en_comillas_simples;
        } else if (*p == '"' && !en_comillas_simples) {
            en_comillas_dobles = !en_comillas_dobles;
        } else if (*p == '#' && !en_comillas_simples && !en_comillas_dobles) {
            *p = '\0';
            return;
        }
    }
}

static int palabra_al_inicio(const char *linea, const char *palabra) {
    size_t len = strlen(palabra);
    char siguiente;

    if (strncmp(linea, palabra, len) != 0) {
        return 0;
    }

    siguiente = linea[len];
    if (siguiente != '\0' && siguiente != ' ' && siguiente != '\t' &&
        siguiente != ';' && siguiente != '(') {
        return 0;
    }

    return 1;
}

static int es_nombre_variable_valido(const char *nombre) {
    size_t i;

    if (nombre[0] == '\0') {
        return 0;
    }
    if (!isalpha((unsigned char)nombre[0]) && nombre[0] != '_') {
        return 0;
    }

    for (i = 1; nombre[i] != '\0'; i++) {
        if (!isalnum((unsigned char)nombre[i]) && nombre[i] != '_') {
            return 0;
        }
    }

    return 1;
}

static int detectar_funcion(const char *linea, char *nombre_funcion, size_t tam) {
    const char *p;
    size_t i = 0;

    if (strncmp(linea, "function", 8) == 0 && (linea[8] == ' ' || linea[8] == '\t')) {
        p = linea + 8;
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        while (*p != '\0' && (isalnum((unsigned char)*p) || *p == '_') && i < tam - 1) {
            nombre_funcion[i++] = *p++;
        }
        nombre_funcion[i] = '\0';

        return (i > 0);
    }

    p = linea;
    while (*p != '\0' && (isalnum((unsigned char)*p) || *p == '_') && i < tam - 1) {
        nombre_funcion[i++] = *p++;
    }
    nombre_funcion[i] = '\0';

    if (i == 0) {
        return 0;
    }

    while (*p == ' ' || *p == '\t') {
        p++;
    }

    if (p[0] == '(' && p[1] == ')') {
        return 1;
    }

    return 0;
}

static int detectar_asignacion_variable(const char *linea, char *nombre_var, size_t tam) {
    const char *prefijos[] = { "declare ", "export ", "local ", "readonly " };
    size_t cantidad_prefijos = sizeof(prefijos) / sizeof(prefijos[0]);
    const char *p = linea;
    const char *inicio_nombre;
    size_t longitud;
    size_t i;

    for (i = 0; i < cantidad_prefijos; i++) {
        size_t len_prefijo = strlen(prefijos[i]);
        if (strncmp(p, prefijos[i], len_prefijo) == 0) {
            p += len_prefijo;
            while (*p == ' ' || *p == '\t' || *p == '-') {
                if (*p == '-') {
                    while (*p != '\0' && *p != ' ' && *p != '\t') {
                        p++;
                    }
                } else {
                    p++;
                }
            }
            break;
        }
    }

    inicio_nombre = p;

    while (isalnum((unsigned char)*p) || *p == '_') {
        p++;
    }

    longitud = (size_t)(p - inicio_nombre);
    if (longitud == 0 || longitud >= tam) {
        return 0;
    }

    if (*p != '=') {
        return 0;
    }
    if (*(p + 1) == '=') {
        return 0;
    }

    strncpy(nombre_var, inicio_nombre, longitud);
    nombre_var[longitud] = '\0';

    if (!es_nombre_variable_valido(nombre_var)) {
        return 0;
    }

    return 1;
}

static int existe_identificador(ListaIdentificadores *lista, const char *nombre) {
    size_t i;

    for (i = 0; i < lista->cantidad; i++) {
        if (strcmp(lista->items[i].nombre, nombre) == 0) {
            return 1;
        }
    }

    return 0;
}

static void agregar_identificador(ListaIdentificadores *lista, const char *nombre) {
    if (existe_identificador(lista, nombre)) {
        return;
    }

    if (lista->cantidad == lista->capacidad) {
        size_t nueva_cap = (lista->capacidad == 0) ? 16 : lista->capacidad * 2;
        Identificador *tmp = realloc(lista->items, nueva_cap * sizeof(Identificador));
        if (tmp == NULL) {
            return;
        }
        lista->items = tmp;
        lista->capacidad = nueva_cap;
    }

    strncpy(lista->items[lista->cantidad].nombre, nombre, sizeof(lista->items[lista->cantidad].nombre) - 1);
    lista->items[lista->cantidad].nombre[sizeof(lista->items[lista->cantidad].nombre) - 1] = '\0';
    lista->cantidad++;
}

static void liberar_reporte(ReporteScript *reporte) {
    free(reporte->variables.items);
    free(reporte->funciones.items);
    reporte->variables.items = NULL;
    reporte->funciones.items = NULL;
}

static void imprimir_reporte(FILE *salida, const char *ruta, ReporteScript *reporte) {
    size_t i;

    fprintf(salida, "\n===== RESUMEN DEL ANALISIS =====\n");
    fprintf(salida, "Archivo analizado: %s\n\n", ruta);

    fprintf(salida, "Lineas totales: %d\n", reporte->lineas_totales);
    fprintf(salida, "Lineas de codigo: %d\n", reporte->lineas_codigo);
    fprintf(salida, "Lineas de comentario: %d\n", reporte->lineas_comentario);
    fprintf(salida, "Lineas en blanco: %d\n\n", reporte->lineas_en_blanco);

    fprintf(salida, "Bucles detectados:\n");
    fprintf(salida, "  for:   %d\n", reporte->bucles_for);
    fprintf(salida, "  while: %d\n", reporte->bucles_while);
    fprintf(salida, "  until: %d\n\n", reporte->bucles_until);

    fprintf(salida, "Estructuras condicionales:\n");
    fprintf(salida, "  if/elif: %d\n", reporte->condicionales_if);
    fprintf(salida, "  case:    %d\n\n", reporte->estructuras_case);

    fprintf(salida, "Funciones definidas (%zu):\n", reporte->funciones.cantidad);
    for (i = 0; i < reporte->funciones.cantidad; i++) {
        fprintf(salida, "  - %s\n", reporte->funciones.items[i].nombre);
    }
    if (reporte->funciones.cantidad == 0) {
        fprintf(salida, "  (ninguna)\n");
    }
    fprintf(salida, "\n");

    fprintf(salida, "Variables declaradas (%zu):\n", reporte->variables.cantidad);
    for (i = 0; i < reporte->variables.cantidad; i++) {
        fprintf(salida, "  - %s\n", reporte->variables.items[i].nombre);
    }
    if (reporte->variables.cantidad == 0) {
        fprintf(salida, "  (ninguna)\n");
    }
    fprintf(salida, "\n");

    fprintf(salida, "Resumen general:\n");
    fprintf(salida, "El script posee %d lineas en total (%d de codigo, %d de comentarios y %d en blanco).\n",
            reporte->lineas_totales, reporte->lineas_codigo, reporte->lineas_comentario, reporte->lineas_en_blanco);
    fprintf(salida, "Se identificaron %d bucle(s) for, %d bucle(s) while y %d bucle(s) until, ",
            reporte->bucles_for, reporte->bucles_while, reporte->bucles_until);
    fprintf(salida, "ademas de %zu funcion(es) y %zu variable(s) declaradas.\n",
            reporte->funciones.cantidad, reporte->variables.cantidad);
    fprintf(salida, "=================================\n");
}
