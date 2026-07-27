#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "bash_parser.h"
#include "util.h"

typedef struct{

    char archivo[300];

    int lineas;
    int comentarios;
    int vacias;
    int comandos;

    int ifs;
    int fors;
    int whiles;
    int cases;
    int funciones;
    int variables;

} estadisticas;

static estadisticas ultimo;

static void analizar_script();
static void mostrar_estadisticas();
static void buscar_estructuras();
static void mostrar_primeras_lineas();
static void ver_historial();

static void registrar_historial();

static int empieza_con(const char *cadena,const char *texto);

void menu_bash(){

    int opcion;

    memset(&ultimo,0,sizeof(ultimo));

    do{

        limpiar_pantalla();

        printf("=====================================\n");
        printf("      ANALIZADOR BASH\n");
        printf("=====================================\n\n");

        printf("1. Analizar script\n");
        printf("2. Mostrar estadisticas\n");
        printf("3. Buscar estructuras\n");
        printf("4. Mostrar primeras lineas\n");
        printf("5. Ver historial\n");
        printf("0. Volver\n\n");

        if(!leer_entero("Seleccione una opcion: ",&opcion)){

            printf("\nEntrada invalida.\n");

            pausar();

            continue;

        }

        switch(opcion){

            case 1:

                analizar_script();

                break;

            case 2:

                mostrar_estadisticas();

                break;

            case 3:

                buscar_estructuras();

                break;

            case 4:

                mostrar_primeras_lineas();

                break;

            case 5:

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

static int empieza_con(const char *cadena,const char *texto){

    while(*texto){

        if(*cadena!=*texto)
            return 0;

        cadena++;
        texto++;

    }

    return 1;

}

static void registrar_historial(){

    FILE *archivo;
    time_t tiempo;
    struct tm *fecha;

    archivo=fopen("historial/bash.log","a");

    if(archivo==NULL)
        return;

    time(&tiempo);

    fecha=localtime(&tiempo);

    fprintf(archivo,
            "%02d/%02d/%04d %02d:%02d:%02d\n",
            fecha->tm_mday,
            fecha->tm_mon+1,
            fecha->tm_year+1900,
            fecha->tm_hour,
            fecha->tm_min,
            fecha->tm_sec);

    fprintf(archivo,"Archivo: %s\n",ultimo.archivo);
    fprintf(archivo,"Lineas: %d\n",ultimo.lineas);
    fprintf(archivo,"Comentarios: %d\n",ultimo.comentarios);
    fprintf(archivo,"Vacias: %d\n",ultimo.vacias);
    fprintf(archivo,"Comandos: %d\n",ultimo.comandos);
    fprintf(archivo,"If: %d\n",ultimo.ifs);
    fprintf(archivo,"For: %d\n",ultimo.fors);
    fprintf(archivo,"While: %d\n",ultimo.whiles);
    fprintf(archivo,"Case: %d\n",ultimo.cases);
    fprintf(archivo,"Funciones: %d\n",ultimo.funciones);
    fprintf(archivo,"Variables: %d\n",ultimo.variables);

    fprintf(archivo,
            "---------------------------------------\n");

    fclose(archivo);

}
static void analizar_script(){

    FILE *archivo;
    char ruta[300];
    char linea[1000];
    char *p;

    limpiar_pantalla();

    printf("=====================================\n");
    printf("      ANALIZAR SCRIPT BASH\n");
    printf("=====================================\n\n");

    if(!leer_texto("Ruta del script: ",ruta,sizeof(ruta)))
        return;

    archivo=fopen(ruta,"r");

    if(archivo==NULL){

        printf("\nNo fue posible abrir el archivo.\n");

        pausar();

        return;

    }

    memset(&ultimo,0,sizeof(ultimo));

    strcpy(ultimo.archivo,ruta);

    while(fgets(linea,sizeof(linea),archivo)!=NULL){

        ultimo.lineas++;

        linea[strcspn(linea,"\n")]='\0';

        p=linea;

        while(*p==' ' || *p=='\t')
            p++;

        if(*p=='\0'){

            ultimo.vacias++;

            continue;

        }

        if(*p=='#'){

            ultimo.comentarios++;

            continue;

        }

        ultimo.comandos++;

        if(empieza_con(p,"if"))
            ultimo.ifs++;

        if(empieza_con(p,"for"))
            ultimo.fors++;

        if(empieza_con(p,"while"))
            ultimo.whiles++;

        if(empieza_con(p,"case"))
            ultimo.cases++;

        if(strstr(p,"function ")!=NULL)
            ultimo.funciones++;

        else{

            char *q=strchr(p,'(');

            if(q!=NULL){

                if(q[1]==')')
                    ultimo.funciones++;

            }

        }

        if(strchr(p,'=')!=NULL){

            if(strstr(p,"==")==NULL &&
               strstr(p,"!=")==NULL &&
               strstr(p,"<=")==NULL &&
               strstr(p,">=")==NULL)
                ultimo.variables++;

        }

    }

    fclose(archivo);

    registrar_historial();

    printf("\nAnalisis completado correctamente.\n");

    printf("\nArchivo analizado: %s\n",ultimo.archivo);

    printf("Lineas: %d\n",ultimo.lineas);

    printf("Comentarios: %d\n",ultimo.comentarios);

    printf("Vacias: %d\n",ultimo.vacias);

    printf("Comandos: %d\n",ultimo.comandos);

    pausar();

}

static void mostrar_estadisticas(){

    limpiar_pantalla();

    if(ultimo.lineas==0){

        printf("No se ha analizado ningun script.\n");

        pausar();

        return;

    }

    printf("=====================================\n");
    printf("         ESTADISTICAS\n");
    printf("=====================================\n\n");

    printf("Archivo              : %s\n",ultimo.archivo);

    printf("Total lineas         : %d\n",ultimo.lineas);

    printf("Comentarios          : %d\n",ultimo.comentarios);

    printf("Lineas vacias        : %d\n",ultimo.vacias);

    printf("Comandos             : %d\n",ultimo.comandos);

    printf("Variables            : %d\n",ultimo.variables);

    printf("\n");

    if(ultimo.lineas>0){

        double porcentaje;

        porcentaje=(double)ultimo.comentarios*100.0/ultimo.lineas;

        printf("Porcentaje comentarios: %.2f %%\n",porcentaje);

    }

    printf("\n");

    pausar();

}
static void buscar_estructuras(){

    limpiar_pantalla();

    if(ultimo.lineas==0){

        printf("No se ha analizado ningun script.\n");

        pausar();

        return;

    }

    printf("=====================================\n");
    printf("      ESTRUCTURAS ENCONTRADAS\n");
    printf("=====================================\n\n");

    printf("If         : %d\n",ultimo.ifs);
    printf("For        : %d\n",ultimo.fors);
    printf("While      : %d\n",ultimo.whiles);
    printf("Case       : %d\n",ultimo.cases);
    printf("Funciones  : %d\n",ultimo.funciones);
    printf("Variables  : %d\n",ultimo.variables);

    printf("\n");

    pausar();

}

static void mostrar_primeras_lineas(){

    FILE *archivo;
    char linea[1000];
    int numero=1;

    limpiar_pantalla();

    if(ultimo.lineas==0){

        printf("No se ha analizado ningun script.\n");

        pausar();

        return;

    }

    archivo=fopen(ultimo.archivo,"r");

    if(archivo==NULL){

        printf("No fue posible abrir nuevamente el archivo.\n");

        pausar();

        return;

    }

    printf("=====================================\n");
    printf("     PRIMERAS 20 LINEAS\n");
    printf("=====================================\n\n");

    while(numero<=20 && fgets(linea,sizeof(linea),archivo)!=NULL){

        printf("%3d | %s",numero,linea);

        numero++;

    }

    fclose(archivo);

    printf("\n");

    pausar();

}

static void ver_historial(){

    FILE *archivo;
    char linea[500];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("      HISTORIAL ANALISIS\n");
    printf("=====================================\n\n");

    archivo=fopen("historial/bash.log","r");

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
