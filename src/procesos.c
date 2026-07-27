#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "procesos.h"
#include "util.h"

void registrar_historial(const char *accion, int pid){

    FILE *archivo;
    time_t t;
    struct tm *tm_info;

    archivo=fopen("historial/procesos.log","a");

    if(archivo==NULL)
        return;

    time(&t);
    tm_info=localtime(&t);

    fprintf(archivo,
            "%02d/%02d/%04d %02d:%02d:%02d - %s PID %d\n",
            tm_info->tm_mday,
            tm_info->tm_mon+1,
            tm_info->tm_year+1900,
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec,
            accion,
            pid);

    fclose(archivo);

}

int existe_pid(int pid){

    char comando[100];

    sprintf(comando,"ps -p %d > /dev/null",pid);

    return system(comando)==0;

}

void listar_procesos() {

    limpiar_pantalla();

    printf("=====================================\n");
    printf("      LISTA DE PROCESOS\n");
    printf("=====================================\n\n");

    system("ps -eo pid,user,%cpu,%mem,comm --sort=-%cpu");

    printf("\n");
	printf("Total de procesos activos: ");

	system("ps -e --no-headers | wc -l");

    pausar();
}

void buscar_proceso() {

    char nombre[100];
    char comando[200];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("     BUSCAR PROCESO\n");
    printf("=====================================\n\n");


    printf("Procesos mas activos:\n\n");

	system("ps -eo pid,%cpu,%mem,comm --sort=-%cpu | head -n 6");

	printf("\n");

    printf("Nombre del proceso: ");
    scanf("%99s", nombre);
    getchar();

    sprintf(comando,"ps -C %s -o pid,user,%%cpu,%%mem,comm",nombre);

    printf("\n");

    system(comando);

    pausar();
}

void finalizar_proceso() {

    int pid;
    char comando[100];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("    FINALIZAR PROCESO\n");
    printf("=====================================\n\n");

    printf("Procesos del usuario:\n\n");

    system("ps -u $USER -o pid,%cpu,%mem,comm --sort=-%cpu | head -n 6");

    printf("\n");

    printf("PID: ");
    scanf("%d", &pid);
    getchar();

    if(!existe_pid(pid)){

    	printf("\nEl proceso no existe.\n");

	    pausar();

    	return;

	}

	int confirmar;

	printf("\nSeguro que desea finalizar el proceso?\n");

	printf("1. Si\n");
	printf("0. No\n");

	printf("Opcion: ");

	scanf("%d",&confirmar);
	getchar();

	if(confirmar==0)
	    return;

    sprintf(comando, "kill %d", pid);

    if(system(comando)==0){

        printf("\nProceso finalizado correctamente.\n");

        registrar_historial("FINALIZAR",pid);

    }
    else{

        printf("\nNo fue posible finalizar el proceso.\n");

    }
}

void suspender_proceso() {

    int pid;
    char comando[100];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("    SUSPENDER PROCESO\n");
    printf("=====================================\n\n");

    printf("Procesos del usuario\n\n");

    system("ps -u $USER -o pid,%cpu,%mem,comm --sort=-%cpu | head -n 6");

    printf("\n");

    printf("PID: ");
    scanf("%d",&pid);
    getchar();

    sprintf(comando,"kill -STOP %d",pid);

    if(system(comando)==0){

        printf("\nProceso suspendido.\n");

        registrar_historial("SUSPENDER",pid);

    }
    else{

        printf("\nNo fue posible suspender el proceso.\n");

    }
}

void reanudar_proceso() {

    int pid;
    char comando[100];

    limpiar_pantalla();

    printf("=====================================\n");
    printf("    REANUDAR PROCESO\n");
    printf("=====================================\n\n");

    printf("Procesos del usuario:\n\n");

    system("ps -u $USER -o pid,%cpu,%mem,comm --sort=-%cpu | head -n 6");

    printf("\n");

    printf("PID: ");
    scanf("%d",&pid);
    getchar();

    sprintf(comando,"kill -CONT %d",pid);

    if(system(comando)==0){

    printf("\nProceso reanudado.\n");

    registrar_historial("REANUDAR",pid);

    }
    else{

        printf("\nNo fue posible reanudar el proceso.\n");

    }
}

void arbol_procesos() {

    limpiar_pantalla();

    printf("=====================================\n");
    printf("      ARBOL DE PROCESOS\n");
    printf("=====================================\n\n");

    system("pstree -p");

    pausar();
}

void ver_historial(){

    limpiar_pantalla();

    printf("=====================================\n");
    printf("   HISTORIAL DE ACCIONES\n");
    printf("=====================================\n\n");

    system("cat historial/procesos.log");

    pausar();
}

void menu_procesos() {

    int opcion;

    do{

	    limpiar_pantalla();
	    printf("Usuario: ");
		system("whoami");

		printf("Fecha: ");			
		system("date '+%d/%m/%Y %H:%M:%S'");

		printf("Procesos activos: ");
		system("ps -e --no-headers | wc -l");

		printf("\n");

        printf("=====================================\n");
        printf("   ADMINISTRADOR DE PROCESOS\n");
        printf("=====================================\n\n");

        printf("1. Listar procesos\n");
        printf("2. Buscar proceso por nombre\n");
        printf("3. Finalizar proceso\n");
        printf("4. Suspender proceso\n");
        printf("5. Reanudar proceso\n");
        printf("6. Mostrar arbol de procesos\n");
        printf("7. Ver historial de acciones\n");
        printf("0. Volver\n\n");

        printf("Seleccione una opcion: ");

        scanf("%d",&opcion);
        getchar();

        switch(opcion){

            case 1:
                listar_procesos();
                break;

            case 2:
                buscar_proceso();
                break;

            case 3:
                finalizar_proceso();
                break;

            case 4:
                suspender_proceso();
                break;

            case 5:
                reanudar_proceso();
                break;

            case 6:
                arbol_procesos();
                break;

            case 7:
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
