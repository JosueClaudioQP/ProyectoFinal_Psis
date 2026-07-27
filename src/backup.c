#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "backup.h"
#include "util.h"

static void registrar_backup(const char *accion);

static void crear_respaldo();
static void restaurar_respaldo();
static void listar_respaldos();
static void eliminar_respaldo();
static void comprimir_respaldo();
static void ver_historial();

static void registrar_backup(const char *accion){

    FILE *archivo;
    time_t tiempo;
    struct tm *fecha;

    archivo=fopen("historial/backups.log","a");

    if(archivo==NULL)
        return;

    time(&tiempo);

    fecha=localtime(&tiempo);

    fprintf(
        archivo,
        "%02d/%02d/%04d %02d:%02d:%02d - %s\n",
        fecha->tm_mday,
        fecha->tm_mon+1,
        fecha->tm_year+1900,
        fecha->tm_hour,
        fecha->tm_min,
        fecha->tm_sec,
        accion
    );

    fclose(archivo);

}

void menu_backup(){

    int opcion;

    do{

        limpiar_pantalla();

        printf("=====================================\n");
        printf("          RESPALDOS\n");
        printf("=====================================\n\n");

        printf("1. Crear respaldo\n");
        printf("2. Restaurar respaldo\n");
        printf("3. Listar respaldos\n");
        printf("4. Eliminar respaldo\n");
        printf("5. Comprimir respaldo\n");
        printf("6. Ver historial\n");
        printf("0. Volver\n\n");

        if(!leer_entero("Seleccione una opcion: ",&opcion)){

            printf("\nEntrada invalida.\n");

            pausar();

            continue;

        }

        switch(opcion){

            case 1:

                crear_respaldo();

                break;

            case 2:

                restaurar_respaldo();

                break;

            case 3:

                listar_respaldos();

                break;

            case 4:

                eliminar_respaldo();

                break;

            case 5:

                comprimir_respaldo();

                break;

            case 6:

                ver_historial();

                break;

            case 0:

                break;

            default:

                printf("\nOpcion invalida.\n");

                pausar();

        }

    }while(opcion!=0);

}

static void crear_respaldo(){

    char origen[300];
    char nombre[150];
    char comando[700];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("       CREAR RESPALDO\n");
    printf("=====================================\n\n");

    if(!leer_texto("Ruta origen: ",origen,sizeof(origen)))
        return;

    if(!leer_texto("Nombre del respaldo: ",nombre,sizeof(nombre)))
        return;

    sprintf(
        comando,
        "cp -r \"%s\" backups/%s",
        origen,
        nombre
    );

    if(system(comando)==0){

        registrar_backup(nombre);

        printf("\nRespaldo creado correctamente.\n");

    }else{

        printf("\nNo fue posible crear el respaldo.\n");

    }

    pausar();

}

static void listar_respaldos(){

    limpiar_pantalla();

    registrar_backup("listar respaldos");

    printf("=====================================\n");
    printf("      LISTA DE RESPALDOS\n");
    printf("=====================================\n\n");

    system("ls -lh backups");

    printf("\n");

    pausar();

}
static void restaurar_respaldo(){

    char respaldo[150];
    char destino[300];
    char comando[700];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("     RESTAURAR RESPALDO\n");
    printf("=====================================\n\n");

    if(!leer_texto("Nombre del respaldo: ",respaldo,sizeof(respaldo)))
        return;

    if(!leer_texto("Ruta destino: ",destino,sizeof(destino)))
        return;

    sprintf(
        comando,
        "cp -r backups/%s \"%s\"",
        respaldo,
        destino
    );

    if(system(comando)==0){

        registrar_backup("restaurar respaldo");

        printf("\nRespaldo restaurado correctamente.\n");

    }else{

        printf("\nError al restaurar el respaldo.\n");

    }

    pausar();

}

static void eliminar_respaldo(){

    char respaldo[150];
    char comando[500];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("      ELIMINAR RESPALDO\n");
    printf("=====================================\n\n");

    if(!leer_texto("Nombre del respaldo: ",respaldo,sizeof(respaldo)))
        return;

    sprintf(
        comando,
        "rm -rf backups/%s",
        respaldo
    );

    if(system(comando)==0){

        registrar_backup("eliminar respaldo");

        printf("\nRespaldo eliminado correctamente.\n");

    }else{

        printf("\nNo fue posible eliminar el respaldo.\n");

    }

    pausar();

}

static void comprimir_respaldo(){

    char respaldo[150];
    char comando[700];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("     COMPRIMIR RESPALDO\n");
    printf("=====================================\n\n");

    if(!leer_texto("Nombre del respaldo: ",respaldo,sizeof(respaldo)))
        return;

    sprintf(
        comando,
        "tar -czf backups/%s.tar.gz -C backups %s",
        respaldo,
        respaldo
    );

    if(system(comando)==0){

        registrar_backup("comprimir respaldo");

        printf("\nArchivo comprimido correctamente.\n");

    }else{

        printf("\nError al comprimir el respaldo.\n");

    }

    pausar();

}

static void ver_historial(){

    FILE *archivo;
    char linea[400];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("      HISTORIAL BACKUPS\n");
    printf("=====================================\n\n");

    archivo=fopen("historial/backups.log","r");

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
