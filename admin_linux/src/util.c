#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void limpiar_pantalla() {
    system("clear");
}

void pausar() {
    printf("\nPresione ENTER para continuar...");
    getchar();
}
