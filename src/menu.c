#include <stdio.h>

#include "menu.h"
#include "procesos.h"
#include "archivos.h"
#include "comandos.h"
#include "backup.h"
#include "bash_parser.h"
#include "descargas.h"
#include "util.h"

void menu_principal() {

    int opcion;

    do {

        limpiar_pantalla();

        printf("=============================================\n");
        printf("         ADMIN LINUX - PROGRAMACION DE SISTEMAS\n");
        printf("=============================================\n\n");

        printf("1. Administrador de procesos\n");
        printf("2. Shell de archivos\n");
        printf("3. Comandos Linux\n");
        printf("4. Respaldos\n");
        printf("5. Analizador Bash\n");
        printf("6. Cola de descargas\n");
        printf("0. Salir\n\n");

        printf("Seleccione una opcion: ");

        scanf("%d",&opcion);
        getchar();

        switch(opcion){

            case 1:
                menu_procesos();
                break;

            case 2:
                menu_archivos();
                break;

            case 3:
                menu_comandos();
                break;

            case 4:
                menu_backup();
                break;

            case 5:
                menu_bash();
                break;

            case 6:
                menu_descargas();
                break;

            case 0:
                printf("\nHasta luego.\n");
                break;

            default:
                printf("\nOpcion invalida.\n");
                pausar();
        }

    }while(opcion!=0);

}
