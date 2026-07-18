#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void limpiar_pantalla() {
    system("clear");
}

void pausar() {
    int caracter;

    printf("\nPresione ENTER para continuar...");

    while ((caracter = getchar()) != '\n' && caracter != EOF) {
    }
}

int leer_entero(const char *mensaje, int *valor) {
    char buffer[64];
    char *fin;
    long numero;

    if (mensaje != NULL) {
        printf("%s", mensaje);
    }

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    numero = strtol(buffer, &fin, 10);

    while (*fin == ' ' || *fin == '\t') {
        fin++;
    }

    if (fin == buffer || (*fin != '\n' && *fin != '\0')) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

int leer_texto(const char *mensaje, char *buffer, size_t tamano) {
    char *posicion;

    if (mensaje != NULL) {
        printf("%s", mensaje);
    }

    if (fgets(buffer, (int)tamano, stdin) == NULL) {
        return 0;
    }

    posicion = strchr(buffer, '\n');
    if (posicion != NULL) {
        *posicion = '\0';
    }

    return buffer[0] != '\0';
}
