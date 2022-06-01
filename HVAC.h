 // FileName:        HVAC.h
 // Dependencies:    None
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye librerías, define ciertas macros, así como llevar un control de versiones.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#ifndef _hvac_h_
#define _hvac_h_

#pragma once

#define __MSP432P401R__
#define  __SYSTEM_CLOCK    48000000 // Frecuencias funcionales recomendadas: 12, 24 y 48 Mhz.

/* Archivos de cabecera importantes. */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Archivos de cabecera POSIX. */
#include <pthread.h>
#include <semaphore.h>
#include <ti/posix/tirtos/_pthread.h>
#include <ti/sysbios/knl/Task.h>

/* Archivos de cabecera RTOS. */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>

/* Archivos de cabecera de drivers de Objetos. */
#include "Drivers_obj/BSP.h"

// Definiciones del estado 'normal' de los botones externos a la tarjeta (solo hay dos botones).
#define GND 0
#define VCC 1
#define NORMAL_STATE_EXTRA_BUTTONS GND  // Aqui se coloca GND o VCC.

// Definiciones del sistema.
#define MAX_MSG_SIZE 64
#define MAX_ADC_VALUE 16383
#define MAIN_UART (uint32_t)(EUSCI_A0)

// Definiciones del RTOS.
#define THREADSTACKSIZE1 1500
#define THREADSTACKSIZE2 1500
#define THREADSTACKSIZE3 1500

// Definición de delay para threads de entradas y salidas.
#define DELAY 4000

/* Enumeradores para la descripción del sistema. */

enum FAN        // Para el fan (abanico).
{
    On,
    Auto,
};

enum SYSTEM     // Para el sistema cuando FAN está en auto (cool, off y heat, o no considerar ninguno y usar fan only).
{
    Cool,
    Off,
    Heat,
    FanOnly,
};

struct EstadoEntradas       // Estructura que incluye el estado de las entradas.
{
    uint8_t  SystemState;
    uint8_t     FanState;
} EstadoEntradas;

/* Funciones. */

/* Función de interrupción para botones de setpoint. */
extern void INT_SWI(void);

/* Funciones de inicialización. */
extern boolean HVAC_InicialiceIO   (void);
extern boolean HVAC_InicialiceADC  (void);
extern boolean HVAC_InicialiceUART (void);

/* Funciones principales. */
extern void HVAC_ActualizarEntradas(void);
extern void HVAC_ActualizarSalidas(void);
extern void HVAC_Heartbeat(void);
extern void HVAC_PrintState(void);

/* Funciones para los estados Heat y Cool. */
extern void HVAC_Heat(void);
extern void HVAC_Cool(void);

/* Funciones para incrementar o disminuir setpoint. */
extern void HVAC_SetPointUp(void);
extern void HVAC_SetPointDown(void);

/* Función especial que imprime el mensaje asegurando que no habrá interrupciones y por ende,
 * un funcionamiento no óptimo.                                                             */
extern void print(char* message);
extern void control_hilos_activados(void);
extern void control_hilos_desactivados(void);
extern void control_De_Hilos(void)

//Ubicación de archivos.
extern FILE _PTR_ input_port = NULL, _PTR_ output_port = NULL;                  // Entradas y salidas.
extern FILE _PTR_ fd_adc = NULL, _PTR_ fd_ch_T = NULL, _PTR_ fd_ch_H = NULL;    // ADC: ch_T -> Temperature, ch_H -> Pot.
extern FILE _PTR_ fd_uart = NULL;                                               // Comunicación serial asíncrona.

/* Definición de botones. */
#define TEMP_PLUS   BSP_BUTTON1     /* Botones de suma y resta al valor deseado, funcionan con interrupciones. */
#define TEMP_MINUS  BSP_BUTTON2

#define FAN_ON      BSP_BUTTON3     /* Botones para identificación del estado del sistema. */
#define FAN_AUTO    BSP_BUTTON4
#define SYSTEM_COOL BSP_BUTTON5
#define SYSTEM_OFF  BSP_BUTTON6
#define SYSTEM_HEAT BSP_BUTTON7

/* Definición de leds. */
#define FAN_LED     BSP_LED1        /* Leds para denotar el estado de las salidas. */
#define HEAT_LED    BSP_LED2
#define HBeat_LED   BSP_LED3
#define COOL_LED    BSP_LED4

// Estructuras iniciales GPIO.

extern uint_32 data[] =                                                          // Formato de las entradas.
{                                                                                // Se prefirió un solo formato.
     TEMP_PLUS,
     TEMP_MINUS,
     FAN_ON,
     FAN_AUTO,
     SYSTEM_COOL,
     SYSTEM_OFF,
     SYSTEM_HEAT,

     GPIO_LIST_END
};
extern const uint_32 fan[] =                                                    // Formato de los leds, uno por uno.
{
     FAN_LED,
     GPIO_LIST_END
};
extern const uint_32 heat[] =                                                   // Formato de los leds, uno por uno.
{
     HEAT_LED,
     GPIO_LIST_END
};
extern const uint_32 hbeat[] =                                                         // Formato de los leds, uno por uno.
{
     HBeat_LED,
     GPIO_LIST_END
};
extern const uint_32 cool[] =                                                   // Formato de los leds, uno por uno.
{
     COOL_LED,
     GPIO_LIST_END
};

// Estructuras iniciales ADC.

extern const ADC_INIT_STRUCT adc_init =
{
    ADC_RESOLUTION_DEFAULT,                                                     // Resolución.
    ADC_CLKDiv8                                                                 // División de reloj.
};

extern const ADC_INIT_CHANNEL_STRUCT adc_ch_param =
{
    TEMPERATURE_ANALOG_PIN,                                                      // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP | ADC_CHANNEL_START_NOW | ADC_INTERNAL_TEMPERATURE, // Banderas de inicialización (temperatura)
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_1                                                                // Trigger lógico que puede activar este canal.
};

extern const ADC_INIT_CHANNEL_STRUCT adc_ch_param2 =
{
    AN1,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_2                                                                // Trigger lógico que puede activar este canal.
};

/* Variables sobre las cuales se maneja el sistema. */

extern float TemperaturaActual = 20;  // Temperatura.
extern float SetPoint = 25.0;         // V. Deseado.

extern char state[MAX_MSG_SIZE];      // Cadena a imprimir.

extern bool toggle = 0;               // Toggle para el heartbeat.
extern _mqx_int delay;                // Delay aplicado al heartbeat.
extern bool event = FALSE;

extern bool FAN_LED_State = 0;                                     // Estado led_FAN.
extern const char* SysSTR[] = {"Cool","Off","Heat","Only Fan"};    // Control de los estados.

#endif
