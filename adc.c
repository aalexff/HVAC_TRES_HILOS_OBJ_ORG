/*
 * adc.c
 *
 *  Created on: May 19, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : boolean; inicializaci�n correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    el m�dulo general ADC y dos de sus canales; uno para la temperatura, otro para
*    el heartbeat.
*
*END***********************************************************************************/

boolean HVAC_InicialiceADC (void)
{
    // Iniciando ADC y canales.
    ////////////////////////////////////////////////////////////////////

    fd_adc   = fopen_f("adc:",  (const char*) &adc_init);               // M�dulo.
    fd_ch_T =  fopen_f("adc:1", (const char*) &adc_ch_param);           // Canal uno, arranca al instante.
    fd_ch_H =  fopen_f("adc:2", (const char*) &adc_ch_param2);          // Canal dos.

    return (fd_adc != NULL) && (fd_ch_T != NULL) && (fd_ch_H != NULL);  // Valida que se crearon los archivos.
}
