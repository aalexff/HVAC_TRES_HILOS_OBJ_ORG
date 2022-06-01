/*
 * Entrada.c
 *
 *  Created on: May 29, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgirán
*    las salidas.
*
*END***********************************************************************************/

void HVAC_ActualizarEntradas(void)
{
    static bool ultimos_estados[] = {FALSE, FALSE, FALSE, FALSE, FALSE};        //PARA CONTROL DE EVENTOS

    ioctl(fd_ch_T, IOCTL_ADC_READ_TEMPERATURE, (pointer) &TemperaturaActual);   // Actualiza valor de temperatura.
    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[2] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)        // Cambia el valor de las entradas FAN.
    {
        EstadoEntradas.FanState = On;
        EstadoEntradas.SystemState = FanOnly;
        if(ultimos_estados[0] == FALSE)                 //SI ULTIMO ESTADO FANSTATE NO ES ON
            event = TRUE;                               //BANDERA OCURRIO EVENTO

        ultimos_estados[0] = TRUE;                      //FANSTATE ON
        ultimos_estados[1] = FALSE;                     //FANSTATE AUTO
    }

    else if((data[3] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)   // Cambia el valor de las entradas SYSTEM.
    {
        EstadoEntradas.FanState = Auto;                //PON fanstate en AUTO
        if(ultimos_estados[1] == FALSE)                //SI ULTIMO ESTADO FANSTATE NO ES AUTO
            event = TRUE;                              //BANDERA OCURRIO EVENTO

       ultimos_estados[0] = FALSE;                     //FANSTATE QUITAR ON
       ultimos_estados[1] = TRUE;                      //FANSTATE ULTIMO ESTADO PONER AUTO

        if((data[4] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)   // Y así sucesivamente para el resto de pines.
        {
            EstadoEntradas.SystemState = Cool;
            if(ultimos_estados[2] == FALSE)
                event = TRUE;
            ultimos_estados[2] = TRUE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = FALSE;
        }
        else if((data[5] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
        {
            EstadoEntradas.SystemState = Off;
            if(ultimos_estados[3] == FALSE)
                event = TRUE;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = TRUE;
            ultimos_estados[4] = FALSE;
        }
        else if((data[6] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
        {
            EstadoEntradas.SystemState = Heat;
            if(ultimos_estados[4] == FALSE)
                event = TRUE;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = TRUE;
        }
        else
        {
            EstadoEntradas.SystemState = Off;       //SI NO SE PRESIONO CUALQUIERA DELOS BOTONES PASADOS PON SystemState en OFF
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = FALSE;
        }
    }
}


