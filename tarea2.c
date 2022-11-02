/*
    Alejandro Henriquez     20.857.754-9
    Sebastian Valdes        20.986.948-9
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#define MAX_CHAR 100

/* Explicacion archivo txt
        
Nombre - HoraComienzoTurno - DuracionDescanso - TiempoRestanteDespuesDescanso
 Juan          6                    1                        10
 Luis          7                    2                         4
 Manuel        9                    1                        11
 Rocio         3                    3                         8

*/

// Variables globales 
sem_t mutex_clock;
sem_t mutex_comprobacion;

int cantidad_trabajadores = 0;
int contador = 0;
int tiempo = -1;

/*Funcion que se encarga de ir avanzando el tiempo del reloj*/
void *comenzar(){ 
    while(cantidad_trabajadores > 0){  
        sem_wait(&mutex_clock);
        tiempo++;   
        if (tiempo > 30)break;
    }
}

/*Funcion que maneja toda la informacion del trabajador*/
void *trabajador(char *linea) {

    /*======= Metodo para obtener los datos del trabajador =======*/
    char *datos[4];
    char *token;
    const char s[2] = " ";
    token = strtok(linea, s);
    int i=0;
    while(token != NULL){
         datos[i] = token;
         token = strtok (NULL,s);
         i++;
    }

    char *nombre = datos[0];
    int comienzo_descanso = atoi(datos[1]);
    int horas_descanso = atoi(datos[2]);
    int descanso_restante = atoi(datos[3]);
    /* ============================================================ */
    
    /* Flags */
    int llega_trabajo = 1;
    int sale_colacion = 1;
    int vuelve_colacion = 1;
    int sale_trabajo = 1;
    contador++;

    int tiempo_anterior = -1;

    /*While que se encargara de ir comprobando cada horario del trabajador*/
    while (1){
        sem_wait(&mutex_comprobacion);   

            /*if y else if para hacer check de horario de salida y entrada*/
            if (tiempo == 0 && llega_trabajo==1 ){
                printf("Llega %s Tiempo %d\n",nombre,tiempo);
                llega_trabajo--;
            }
            else if ((tiempo == comienzo_descanso) && sale_colacion==1){
                printf("Sale %s Tiempo %d\n",nombre,tiempo);
                sale_colacion--;
            }
            else if ((tiempo == (comienzo_descanso + horas_descanso))&& vuelve_colacion == 1){ 
                printf("Llega %s Tiempo %d\n",nombre,tiempo);  
                vuelve_colacion--;  
            }
            else if ((tiempo == (comienzo_descanso + horas_descanso + descanso_restante))&& sale_trabajo == 1){
                printf("Sale %s Tiempo %d\n",nombre,tiempo);
                sale_trabajo--;
            }
            /*                                                            */

            /*Compara para ver si el trabajador ya se reviso*/
            if (tiempo_anterior < tiempo){
                tiempo_anterior++;
                contador++;        
            }

            /* Si se comprobo a todos los trabajadores, el reloj avanza en 1 */
            if (contador == cantidad_trabajadores){
                sem_post(&mutex_clock);
                contador=0;
            }
        sem_post(&mutex_comprobacion); 
    }

}

void coordinador (){
    printf("\n---------Inicio Codigo---------\n\n");
    //Paso 1 -> Lectura del archivo
    FILE *f;
    char linea[MAX_CHAR];
    f= fopen("Horarios.txt","r");
    if (f==NULL) return;

    //Paso 2 -> Extraccion de los datos   
    while (fgets(linea, MAX_CHAR, f)){
        cantidad_trabajadores++;
    }

    fflush(f);
    rewind(f);

    /* Se almacenan los datos en una lista de datos */
    char **lista_datos = (char**)malloc(cantidad_trabajadores*sizeof(char*));
    int i = 0;
    while (fgets(linea, MAX_CHAR, f)){
        char *datos = (char*) malloc (sizeof(char) * sizeof(linea));
        strcpy(datos,linea);
        lista_datos[i] = datos;
        i++;
    }

    /*Se crean las hebras correspondientes para cada trabajador*/
    pthread_t threads[cantidad_trabajadores];
    for (int t = 0; t < cantidad_trabajadores; t++){
        pthread_create(&threads[t],NULL, (void*)trabajador, (char*) lista_datos[t]);  //atado aqui es trabajador parece
    }

    /*Se crea la hebra encargada de controlar el reloj*/
    pthread_t control_tiempo;  
    pthread_create(&control_tiempo,NULL,(void *)comenzar, NULL); 
    pthread_join(control_tiempo,NULL);

    /*====================================================*/
    printf("\n---------Fin Codigo---------\n");
    fclose(f);

}

void main (){
    sem_init(&mutex_clock,0,0);
    sem_init(&mutex_comprobacion,0,1);
    coordinador();
    sem_destroy(&mutex_clock);
    sem_destroy(&mutex_comprobacion);
}

