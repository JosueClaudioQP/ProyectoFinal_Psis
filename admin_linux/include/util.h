#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void limpiar_pantalla();
void pausar();
int leer_entero(const char *mensaje, int *valor);
int leer_texto(const char *mensaje, char *buffer, size_t tamano);

#endif
