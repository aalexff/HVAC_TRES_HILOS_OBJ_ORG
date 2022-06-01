 // FileName:        BSP.h
 // Dependencies:    None.
 // Processor:       MSP432
 // Board:			 MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye la capa de abstracción de los drivers.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         12/2018

#ifndef BSP_H_
#define BSP_H_

#include <ti/devices/msp432p4xx/inc/msp.h>

// Estos tres archivos de cabecera deben ir primero (types, structures y Files.h).
#include "../Drivers_obj/types.h"
#include "../Drivers_obj/structures.h"
#include "../Drivers_obj/Files.h"

#include "../Drivers_obj/adc_f_MSP432.h"
#include "../Drivers_obj/gpio_f_MSP432.h"
#include "../Drivers_obj/int_MSP432.h"
#include "../Drivers_obj/uart_f_MSP432.h"
#include "../Drivers_obj/timer_f_msp432.h"

#define BSP_SYSTEM_CLOCK    (__SYSTEM_CLOCK)

/* Definición de puertos. */
#define PORT_1  0x0001 << 3     // Segun su uso en archivos de GPIO.
#define PORT_2  0x0002 << 3
#define PORT_3  0x0003 << 3
#define PORT_4  0x0004 << 3
#define PORT_5  0x0005 << 3
#define PORT_6  0x0006 << 3

/* Definición de pines. */

#define GPIO_PIN0         (0)
#define GPIO_PIN1         (1)
#define GPIO_PIN2         (2)
#define GPIO_PIN3         (3)
#define GPIO_PIN4         (4)
#define GPIO_PIN5         (5)
#define GPIO_PIN6         (6)
#define GPIO_PIN7         (7)
#define GPIO_PIN(x)       (x)

/* Definición de botones con su respectivo puerto. */

#define BSP_BUTTON1 ((PORT_1 | GPIO_PIN_VALID | GPIO_PIN_IRQ | GPIO_IRQ_EDGE_H_TO_L) | GPIO_PIN1)
#define BSP_BUTTON2 ((PORT_1 | GPIO_PIN_VALID | GPIO_PIN_IRQ | GPIO_IRQ_EDGE_H_TO_L) | GPIO_PIN4)   // Irq, flanco de bajada.

#define BSP_BUTTON3 (PORT_2 | GPIO_PIN_VALID | GPIO_PIN3)
#define BSP_BUTTON4 (PORT_2 | GPIO_PIN_VALID | GPIO_PIN4)
#define BSP_BUTTON5 (PORT_2 | GPIO_PIN_VALID | GPIO_PIN5)
#define BSP_BUTTON6 (PORT_2 | GPIO_PIN_VALID | GPIO_PIN6)
#define BSP_BUTTON7 (PORT_2 | GPIO_PIN_VALID | GPIO_PIN7)

/* Definición de led's con su respectivo puerto. */

#define BSP_LED1 ((PORT_1 | GPIO_PIN_VALID) | GPIO_PIN0)
#define BSP_LED2 ((PORT_2 | GPIO_PIN_VALID) | GPIO_PIN0)
#define BSP_LED3 ((PORT_2 | GPIO_PIN_VALID) | GPIO_PIN1)
#define BSP_LED4 ((PORT_2 | GPIO_PIN_VALID) | GPIO_PIN2)

// Definiciones de apuntadores a función e identificadores de cada uno de los tipos de dispositivos:
// Dispositivos o drivers: gpio, adc, uart y timer.
// Las posibles funciones son abrir archivo (open), cerrarlo (close), leerlo (read) o controlarlo (ioctl).

const static IO_DEVICE_STRUCT instruction_set[] =
{
     {.IDENTIFIER = "gpio:",  .IO_OPEN = gpio_open,  .IO_CLOSE = gpio_close,  .IO_READ = gpio_read,    .IO_IOCTL = gpio_ioctl},
     {.IDENTIFIER = "adc:",   .IO_OPEN = adc_open,   .IO_CLOSE = adc_close,   .IO_IOCTL = adc_ioctl,   .IO_READ = adc_read},
     {.IDENTIFIER = "uart:",  .IO_OPEN = uart_open,  .IO_CLOSE = uart_close,  .IO_IOCTL = uart_ioctl,  .IO_READ = uart_read},
     {.IDENTIFIER = "timer:", .IO_OPEN = timer_open, .IO_CLOSE = timer_close, .IO_IOCTL = timer_ioctl, .IO_READ = timer_read}
};

/* Definiciones de tiempos. */
#define STEP            1000                // Este STEP solo es válido para el ADC.
#define TIME_RESET      50000
#define MILLIS          1000
#define SEC             1000000

#endif /* BSP_H_ */
