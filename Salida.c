/*
 * Salida.c
 *
 *  Created on: May 29, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarSalidas
* Returned Value   : None.
* Comments         :
*    Decide a partir de las entradas actualizadas las salidas principales,
*    y en ciertos casos, en base a una cuestión de temperatura, la salida del 'fan'.
*
*END***********************************************************************************/
void HVAC_ActualizarSalidas(void)
{
    // Cambia el valor de las salidas de acuerdo a entradas.

    if(EstadoEntradas.FanState == On)                               // Para FAN on.
    {
        FAN_LED_State = 1;
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);
    }

    else if(EstadoEntradas.FanState == Auto)                        // Para FAN automatico.
    {
        switch(EstadoEntradas.SystemState)
        {
        case Off:   ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
                    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
                    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);
                    FAN_LED_State = 0;
                    break;
        case Heat:  HVAC_Heat(); break;
        case Cool:  HVAC_Cool(); break;
        }
    }
}

