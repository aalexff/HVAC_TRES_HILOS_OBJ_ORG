/*
 * gpio.c
 *
 *  Created on: May 19, 2022
 *      Author: Alex Armando Figueroa Hernandez - 17061169
 */

#include "HVAC.h"

/* Definición de botones. */
//#define TEMP_PLUS   BSP_BUTTON1     /* Botones de suma y resta al valor deseado, funcionan con interrupciones. */
//#define TEMP_MINUS  BSP_BUTTON2

//#define FAN_ON      BSP_BUTTON3     /* Botones para identificación del estado del sistema. */
/*#define FAN_AUTO    BSP_BUTTON4
#define SYSTEM_COOL BSP_BUTTON5
#define SYSTEM_OFF  BSP_BUTTON6
#define SYSTEM_HEAT BSP_BUTTON7*/

/* Definición de leds. */
//#define FAN_LED     BSP_LED1        /* Leds para denotar el estado de las salidas. */
/*#define HEAT_LED    BSP_LED2
#define HBeat_LED   BSP_LED3
#define COOL_LED    BSP_LED4*/

/*uint_32 data[] =                                                          // Formato de las entradas.
{                                                                                // Se prefirió un solo formato.
     TEMP_PLUS,
     TEMP_MINUS,
     FAN_ON,
     FAN_AUTO,
     SYSTEM_COOL,
     SYSTEM_OFF,
     SYSTEM_HEAT,

     GPIO_LIST_END
};*/

/*const uint_32 fan[] =                                                    // Formato de los leds, uno por uno.
{
     FAN_LED,
     GPIO_LIST_END
};

const uint_32 heat[] =                                                   // Formato de los leds, uno por uno.
{
     HEAT_LED,
     GPIO_LIST_END
};

const uint_32 hbeat[] =                                                         // Formato de los leds, uno por uno.
{
     HBeat_LED,
     GPIO_LIST_END
};

const uint_32 cool[] =                                                   // Formato de los leds, uno por uno.
{
     COOL_LED,
     GPIO_LIST_END
};*/

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas de entrada y salida GPIO.
*
*END***********************************************************************************/
boolean HVAC_InicialiceIO(void)
{
    // Estructuras iniciales de entradas y salidas.
    const uint_32 output_set[] =
    {
         FAN_LED   | GPIO_PIN_STATUS_0,
         HEAT_LED  | GPIO_PIN_STATUS_0,
         HBeat_LED | GPIO_PIN_STATUS_0,
         COOL_LED  | GPIO_PIN_STATUS_0,
         GPIO_LIST_END
    };

    const uint_32 input_set[] =
    {
        TEMP_PLUS,
        TEMP_MINUS,
        FAN_ON,
        FAN_AUTO,
        SYSTEM_COOL,
        SYSTEM_OFF,
        SYSTEM_HEAT,

        GPIO_LIST_END
    };

    // Iniciando GPIO.
    ////////////////////////////////////////////////////////////////////

    output_port =  fopen_f("gpio:write", (char_ptr) &output_set);
    input_port =   fopen_f("gpio:read", (char_ptr) &input_set);

    if (output_port) { ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, NULL); }   // Inicialmente salidas apagadas.
    ioctl (input_port, GPIO_IOCTL_SET_IRQ_FUNCTION, INT_SWI);               // Declarando interrupción.

    return (input_port != NULL) && (output_port != NULL);
}

