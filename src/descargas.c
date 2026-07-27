#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "descargas.h"
#include "util.h"

nodo *frente=NULL;
nodo *final=NULL;

void registrar_descarga(const char *evento,const char *archivo){

    FILE *archivo_log;

    time_t tiempo;

    struct tm *fecha;

    archivo_log=fopen("historial/descargas.log","a");

    if(archivo_log==NULL)
        return;

    time(&tiempo);

    fecha=localtime(&tiempo);

    fprintf(
        archivo_log,
        "%02d/%02d/%04d %02d:%02d:%02d - %s - %s\n",
        fecha->tm_mday,
        fecha->tm_mon+1,
        fecha->tm_year+1900,
        fecha->tm_hour,
        fecha->tm_min,
        fecha->tm_sec,
        evento,
        archivo
    );

    fclose(archivo_log);

}

void agregar_descarga(){

    nodo *nuevo;

    char *nombre;

    nuevo=malloc(sizeof(nodo));

    if(nuevo==NULL){

        printf("\nNo fue posible reservar memoria.\n");

        pausar();

        return;

    }

    limpiar_pantalla();

    printf("=====================================\n");
    printf("        NUEVA DESCARGA\n");
    printf("=====================================\n\n");

    printf("Ruta del archivo origen:\n");
    fgets(nuevo->origen,sizeof(nuevo->origen),stdin);

    nuevo->origen[strcspn(nuevo->origen,"\n")]=0;

    printf("\nRuta destino:\n");
    fgets(nuevo->destino,sizeof(nuevo->destino),stdin);

    nuevo->destino[strcspn(nuevo->destino,"\n")]=0;

    nombre=strrchr(nuevo->origen,'/');

    if(nombre==NULL)
        strcpy(nuevo->nombre,nuevo->origen);
    else
        strcpy(nuevo->nombre,nombre+1);

    strcpy(nuevo->estado,"Pendiente");

    nuevo->sig=NULL;

    if(frente==NULL){

        frente=nuevo;
        final=nuevo;

    }
    else{

        final->sig=nuevo;
        final=nuevo;

    }

    registrar_descarga("AGREGADA",nuevo->nombre);

    printf("\nDescarga agregada correctamente.\n");

    pausar();

}

void mostrar_cola(){

    nodo *aux;

    int contador=1;

    limpiar_pantalla();

    printf("=====================================\n");
    printf("       COLA DE DESCARGAS\n");
    printf("=====================================\n\n");

    if(frente==NULL){

        printf("No existen tareas pendientes.\n");

        pausar();

        return;

    }

    printf("%-5s %-30s %-15s\n","N°","Archivo","Estado");

    printf("--------------------------------------------------------------\n");

    aux=frente;

    while(aux!=NULL){

        printf(
            "%-5d %-30s %-15s\n",
            contador,
            aux->nombre,
            aux->estado
        );

        contador++;

        aux=aux->sig;

    }

    printf("\n");

    printf("Total de tareas: %d\n",contador-1);

    pausar();

}

static void ejecutar_descarga(nodo *actual){

    char comando[700];

    char verificar[700];

    strcpy(actual->estado,"Procesando");

    registrar_descarga("INICIO",actual->nombre);

    printf("=====================================\n");
    printf("      PROCESANDO DESCARGA\n");
    printf("=====================================\n\n");

    printf("Archivo : %s\n",actual->nombre);
    printf("Origen  : %s\n",actual->origen);
    printf("Destino : %s\n\n",actual->destino);

    sprintf(
        verificar,
        "test -f \"%s\"",
        actual->origen
    );

    if(system(verificar)!=0){

        printf("El archivo origen no existe.\n");

        strcpy(actual->estado,"Error");

        registrar_descarga("ERROR",actual->nombre);

        return;

    }

    sprintf(
        comando,
        "rsync -ah --progress \"%s\" \"%s\"",
        actual->origen,
        actual->destino
    );

    if(system(comando)==0){

        strcpy(actual->estado,"Finalizada");

        registrar_descarga("FINALIZADA",actual->nombre);

        printf("\nDescarga completada correctamente.\n");

    }
    else{

        strcpy(actual->estado,"Error");

        registrar_descarga("ERROR",actual->nombre);

        printf("\nOcurrio un error durante la copia.\n");

    }

}

static void procesar_nodo(){

    nodo *aux;

    aux=frente;

    ejecutar_descarga(aux);

    frente=frente->sig;

    if(frente==NULL)
        final=NULL;

    free(aux);

}

void procesar_siguiente(){

    limpiar_pantalla();

    if(frente==NULL){

        printf("No existen tareas pendientes.\n");

        pausar();

        return;

    }

    procesar_nodo();

    pausar();

}

void procesar_todas(){

    int total=0;

    limpiar_pantalla();

    if(frente==NULL){

        printf("No existen tareas pendientes.\n");

        pausar();

        return;

    }

    while(frente!=NULL){

        total++;

        printf("\n=====================================\n");
        printf("Procesando tarea %d\n",total);
        printf("=====================================\n\n");

        procesar_nodo();

    }

    printf("\nSe procesaron %d tareas correctamente.\n",total);

    pausar();

}

void cancelar_descarga(){

    nodo *actual;

    nodo *anterior;

    int opcion;

    int contador=1;

    limpiar_pantalla();

    if(frente==NULL){

        printf("No existen tareas pendientes.\n");

        pausar();

        return;

    }

    actual=frente;

    while(actual!=NULL){

        printf("%d. %s\n",contador,actual->nombre);

        actual=actual->sig;

        contador++;

    }

    printf("\nSeleccione la tarea a cancelar: ");
    scanf("%d",&opcion);

    getchar();

    if(opcion<1 || opcion>=contador){

        printf("\nOpcion invalida.\n");

        pausar();

        return;

    }

    actual=frente;

    anterior=NULL;

    contador=1;

    while(contador<opcion){

        anterior=actual;

        actual=actual->sig;

        contador++;

    }

    registrar_descarga("CANCELADA",actual->nombre);

    if(anterior==NULL){

        frente=actual->sig;

    }
    else{

        anterior->sig=actual->sig;

    }

    if(actual==final){

        final=anterior;

    }

    free(actual);

    printf("\nDescarga cancelada correctamente.\n");

    pausar();

}

void ver_historial_descargas(){

    FILE *archivo;

    char linea[500];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("    HISTORIAL DE DESCARGAS\n");
    printf("=====================================\n\n");

    archivo=fopen("historial/descargas.log","r");

    if(archivo==NULL){

        printf("No existe historial.\n");

        pausar();

        return;

    }

    while(fgets(linea,sizeof(linea),archivo)!=NULL){

        printf("%s",linea);

    }

    fclose(archivo);

    printf("\n");

    pausar();

}

void menu_descargas(){

    int opcion;

    do{

        limpiar_pantalla();

        printf("=====================================\n");
        printf("      COLA DE DESCARGAS\n");
        printf("=====================================\n\n");

        printf("1. Agregar descarga\n");
        printf("2. Ver cola\n");
        printf("3. Procesar siguiente\n");
        printf("4. Procesar toda la cola\n");
        printf("5. Cancelar descarga\n");
        printf("6. Ver historial\n");
        printf("0. Volver\n");

        printf("\nSeleccione una opcion: ");

        scanf("%d",&opcion);

        getchar();

        switch(opcion){

            case 1:

                agregar_descarga();

                break;

            case 2:

                mostrar_cola();

                break;

            case 3:

                procesar_siguiente();

                break;

            case 4:

                procesar_todas();

                break;

            case 5:

                cancelar_descarga();

                break;

            case 6:

                ver_historial_descargas();

                break;

            case 0:

                break;

            default:

                printf("\nOpcion invalida.\n");

                pausar();

        }

    }while(opcion!=0);

}