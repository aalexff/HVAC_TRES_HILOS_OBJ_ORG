 // FileName:        Threads.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Activa las funciones de los hilos Entradas_Thread, Salidas_Thread y HeartBeat_Thread.
 //                  Main del proyecto.
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

extern void *Entradas_Thread(void *arg0);   // Threads que arrancar�n inicialmente.
extern void *Salidas_Thread(void *arg0);
extern void *HeartBeat_Thread(void *arg0);

int main(void)
{
    pthread_t           entradas_thread, salidas_thread, heartbeat_thread;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

   pthread_attr_init(&pAttrs);                                  /* Reinicio de par�metros. */

   detachState = PTHREAD_CREATE_DETACHED;                       // Los recursos se liberar�n despu�s del t�rmino del thread.
   retc = pthread_attr_setdetachstate(&pAttrs, detachState);    // Adem�s, al hilo no se le puede unir otro (join).
   if (retc != 0) { while (1); }

   /**********************
   ** Entradas Thread   **
   **********************/

   priParam.sched_priority = 3;                                             // Mayor prioridad a la tarea principal.
   retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE1);            // As� se determinar�a el tama�o del stack.
   if (retc != 0) { while (1); }
   pthread_attr_setschedparam(&pAttrs, &priParam);
   retc = pthread_create(&entradas_thread, &pAttrs, Entradas_Thread, NULL); // Creaci�n del thread.
   if (retc != 0) { while (1); }

   /**********************
    ** Salidas Thread   **
    **********************/

    priParam.sched_priority = 1;
    retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE2);          // As� se determinar�a el tama�o del stack.
    if (retc != 0) { while (1); }
    pthread_attr_setschedparam(&pAttrs, &priParam);
    retc = pthread_create(&salidas_thread, &pAttrs, Salidas_Thread, NULL); // Creaci�n del thread.
    if (retc != 0) { while (1); }


   /**********************
    ** Heartbeat Thread  *
    **********************/

    pthread_attr_init(&pAttrs);                                                 /* Reinicio de par�metros. */
    priParam.sched_priority = 1;
    retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE3);               // As� se determinar�a el tama�o del stack.
    if (retc != 0) { while (1); }
    pthread_attr_setschedparam(&pAttrs, &priParam);
    retc = pthread_create(&heartbeat_thread, &pAttrs, HeartBeat_Thread, NULL);  // Creaci�n del thread.
    if (retc != 0) { while (1); }

   /* Arranque del sistema. */
   BIOS_start();
   return (0);
}

/*
 * Tarea 2, unidad 3
Bas�ndose en el c�digo del archivo ejemplo.c, modifique el c�digo
del HVAC_3_Hilos para que los hilos de Salidas_Thread y
HeartBeat_Thread se bloqueen despu�s de que el FAN se quede en
AUTO y el SYSTEM en OFF. Una vez que se bloquean ambas tareas, se
manda un mensaje a consola y a terminal que
dice Salida y HeatBeat apagados. Una vez que el sistema sale de este
estado, Salidas_Thread y HeartBeat_Thread se vuelven a correr.
*/

//Hilo para iniciar hilos
void control_hilos_activados(void){
    //Task Set priority: ti_sysbios_knl_Task_setPri
    Task_setPri(((pthread_Obj*)Salidas_Thread)->task, 1);
    /*xdc__CODESECT(ti_sysbios_knl_Task_setPri__E, "ti_sysbios_knl_Task_setPri")
__extern xdc_Int ti_sysbios_knl_Task_setPri__E( ti_sysbios_knl_Task_Handle instp, xdc_Int newpri);*/
    Task_setPri(((pthread_Obj*)HeartBeat_Thread)->task, 1);
    return;
}
//Hilo para cerrar hilos
void control_hilos_desactivados(void){
    //Task Set priority: ti_sysbios_knl_Task_setPri
    Task_setPri(((pthread_Obj*)Salidas_Thread)->task, -1);
    Task_setPri(((pthread_Obj*)HeartBeat_Thread)->task, -1);
    return;
}

