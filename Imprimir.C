/*
 * Imprimir.C
 *
 *  Created on: May 29, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    static uint_32 delay = DELAY;
    delay -= DELAY;

    if(delay <= 0 || event == TRUE)
    {
        event = FALSE;
        delay = SEC;

        sprintf(state,"Fan: %s, System: %s, SetPoint: %0.2f\n\r",
                    EstadoEntradas.FanState == On? "On":"Auto",
                    SysSTR[EstadoEntradas.SystemState],
                    SetPoint);
        print(state);

        sprintf(state,"Temperatura Actual: %0.2f°C %0.2f°F  Fan: %s\n\r\n\r",
                    TemperaturaActual,
                    ((TemperaturaActual*9.0/5.0) + 32),
                    FAN_LED_State?"On":"Off");
        print(state);
    }
}

