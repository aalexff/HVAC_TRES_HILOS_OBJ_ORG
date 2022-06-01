/*
 * adc.c
 *
 *  Created on: May 19, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

// Estructuras iniciales.

const ADC_INIT_STRUCT adc_init =
{
    ADC_RESOLUTION_DEFAULT,                                                     // Resolución.
    ADC_CLKDiv8                                                                 // División de reloj.
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param =
{
    TEMPERATURE_ANALOG_PIN,                                                      // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP | ADC_CHANNEL_START_NOW | ADC_INTERNAL_TEMPERATURE, // Banderas de inicialización (temperatura)
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_1                                                                // Trigger lógico que puede activar este canal.
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param2 =
{
    AN1,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_2                                                                // Trigger lógico que puede activar este canal.
};

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    el módulo general ADC y dos de sus canales; uno para la temperatura, otro para
*    el heartbeat.
*
*END***********************************************************************************/

boolean HVAC_InicialiceADC (void)
{
    // Iniciando ADC y canales.
    ////////////////////////////////////////////////////////////////////

    fd_adc   = fopen_f("adc:",  (const char*) &adc_init);               // Módulo.
    fd_ch_T =  fopen_f("adc:1", (const char*) &adc_ch_param);           // Canal uno, arranca al instante.
    fd_ch_H =  fopen_f("adc:2", (const char*) &adc_ch_param2);          // Canal dos.

    return (fd_adc != NULL) && (fd_ch_T != NULL) && (fd_ch_H != NULL);  // Valida que se crearon los archivos.
}
