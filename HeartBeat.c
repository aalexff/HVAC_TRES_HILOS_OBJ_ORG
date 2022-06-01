/*
 * HeartBeat.c
 *
 *  Created on: May 29, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heartbeat
* Returned Value   : None.
* Comments         :
*    Función que prende y apaga una salida para notificar que el sistema está activo.
*    El periodo en que se hace esto depende de una entrada del ADC en esta función.
*
*END***********************************************************************************/
void HVAC_Heartbeat(void)               // Función de 'alive' del sistema.
{
   _mqx_int val;
   boolean flag = 0;
   static boolean bandera_inicial = 0;

   if(bandera_inicial == 0)
   {
       // Entrando por primera vez, empieza a correr el canal de heartbeat.
       ioctl (fd_ch_H, IOCTL_ADC_RUN_CHANNEL, NULL);
       bandera_inicial = 1;
   }

   // Valor se guarda en val, flag nos dice si fue exitoso.
   flag =  (fd_adc && fread_f(fd_ch_H, &val, sizeof(val))) ? 1 : 0;

   if(flag != TRUE)
   {
       printf("Error al leer archivo. Cierre del programa\r\n");
       exit(1);
   }

    delay = 15000 + (100 * val / 4);            // Lectura del ADC por medio de la función.
    //Nota: delay no puede ser mayor a 1,000,000 ya que luego se generan problemas en usleep.

    if(toggle)
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &hbeat);
    else
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &hbeat);

    toggle ^= 1;                             // Toggle.

    usleep(delay);                           // Delay marcado por el heart_beat.
    return;
}


