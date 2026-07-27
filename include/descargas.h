#ifndef DESCARGAS_H
#define DESCARGAS_H

typedef struct nodo{

    char origen[300];

    char destino[300];

    char nombre[150];

    char estado[30];

    struct nodo *sig;

}nodo;

void menu_descargas();

void agregar_descarga();

void mostrar_cola();

void procesar_siguiente();

void procesar_todas();

void cancelar_descarga();

void ver_historial_descargas();

#endif
