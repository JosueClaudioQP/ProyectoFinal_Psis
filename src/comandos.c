#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "comandos.h"
#include "util.h"

static void registrar_historial(const char *accion);

static void informacion_sistema();
static void uso_disco();
static void uso_memoria();
static void usuarios_conectados();
static void procesos_activos();
static void interfaces_red();
static void espacio_libre();
static void comando_personalizado();
static void ver_historial();

static void registrar_historial(const char *accion){

    FILE *archivo;
    time_t tiempo;
    struct tm *fecha;

    archivo=fopen("historial/comandos.log","a");

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

static void informacion_sistema(){

    limpiar_pantalla();

    registrar_historial("informacion del sistema");

    printf("=====================================\n");
    printf("   INFORMACION DEL SISTEMA\n");
    printf("=====================================\n\n");

    printf("Sistema operativo:\n");
    system("uname -a");

    printf("\n");

    printf("Nombre del equipo:\n");
    system("hostname");

    printf("\n");

    printf("Kernel:\n");
    system("uname -r");

    printf("\n");

    printf("Arquitectura:\n");
    system("uname -m");

    printf("\n");

    printf("Tiempo encendido:\n");
    system("uptime");

    printf("\n");

    pausar();

}

static void uso_disco(){

    limpiar_pantalla();

    registrar_historial("uso del disco");

    printf("=====================================\n");
    printf("      USO DEL DISCO\n");
    printf("=====================================\n\n");

    system("df -h");

    printf("\n");

    printf("Directorios mas pesados:\n\n");

    system("du -sh * 2>/dev/null | sort -hr | head");

    printf("\n");

    pausar();

}

void menu_comandos(){

    int opcion;

    do{

        limpiar_pantalla();

        printf("=====================================\n");
        printf("       COMANDOS LINUX\n");
        printf("=====================================\n\n");

        printf("1. Informacion del sistema\n");
        printf("2. Uso del disco\n");
        printf("3. Uso de memoria\n");
        printf("4. Usuarios conectados\n");
        printf("5. Procesos mas activos\n");
        printf("6. Interfaces de red\n");
        printf("7. Espacio libre\n");
        printf("8. Ejecutar comando personalizado\n");
        printf("9. Ver historial\n");
        printf("0. Volver\n\n");

        if(!leer_entero("Seleccione una opcion: ",&opcion)){

            printf("\nEntrada invalida.\n");

            pausar();

            continue;

        }

        switch(opcion){

            case 1:

                informacion_sistema();

                break;

            case 2:

                uso_disco();

                break;

            case 3:

                uso_memoria();

                break;

            case 4:

                usuarios_conectados();

                break;

            case 5:

                procesos_activos();

                break;

            case 6:

                interfaces_red();

                break;

            case 7:

                espacio_libre();

                break;

            case 8:

                comando_personalizado();

                break;

            case 9:

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
static void uso_memoria(){

    limpiar_pantalla();

    registrar_historial("uso de memoria");

    printf("=====================================\n");
    printf("      USO DE MEMORIA\n");
    printf("=====================================\n\n");

    system("free -h");

    printf("\n");

    printf("Memoria virtual:\n\n");

    system("vmstat");

    printf("\n");

    pausar();

}

static void usuarios_conectados(){

    limpiar_pantalla();

    registrar_historial("usuarios conectados");

    printf("=====================================\n");
    printf("    USUARIOS CONECTADOS\n");
    printf("=====================================\n\n");

    printf("Usuarios activos:\n\n");

    system("who");

    printf("\n");

    printf("Ultimos accesos:\n\n");

    system("last -n 10");

    printf("\n");

    pausar();

}

static void procesos_activos(){

    limpiar_pantalla();

    registrar_historial("procesos mas activos");

    printf("=====================================\n");
    printf("    PROCESOS MAS ACTIVOS\n");
    printf("=====================================\n\n");

    system("ps -eo pid,user,%cpu,%mem,comm --sort=-%cpu | head -15");

    printf("\n");

    pausar();

}

static void interfaces_red(){

    limpiar_pantalla();

    registrar_historial("interfaces de red");

    printf("=====================================\n");
    printf("      INTERFACES DE RED\n");
    printf("=====================================\n\n");

    if(system("ip addr show")==0){

    }
    else{

        printf("El comando ip no esta disponible.\n\n");

        system("ifconfig");

    }

    printf("\n");

    pausar();

}

static void espacio_libre(){

    limpiar_pantalla();

    registrar_historial("espacio libre");

    printf("=====================================\n");
    printf("       ESPACIO LIBRE\n");
    printf("=====================================\n\n");

    system("df -h");

    printf("\n");

    printf("Uso del directorio actual:\n\n");

    system("du -sh .");

    printf("\n");

    pausar();

}

static void comando_personalizado(){

    char comando[300];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("   COMANDO PERSONALIZADO\n");
    printf("=====================================\n\n");

    printf("Ingrese un comando de Linux:\n");

    if(!leer_texto("> ",comando,sizeof(comando))){

        printf("\nEntrada invalida.\n");

        pausar();

        return;

    }

    registrar_historial(comando);

    printf("\n");

    system(comando);

    printf("\n");

    pausar();

}

static void ver_historial(){

    FILE *archivo;

    char linea[500];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("     HISTORIAL DE COMANDOS\n");
    printf("=====================================\n\n");

    archivo=fopen("historial/comandos.log","r");

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
