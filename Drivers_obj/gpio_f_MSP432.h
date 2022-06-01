 //FileName:        gpio_f_MSP432.h
 //Dependencies:    None.
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para GPIO por medio de archivos. Header File.
 //Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 //Updated:         12/2018

#ifndef GPIO_F_MSP432_H_
#define GPIO_F_MSP432_H_

/* Definiciones de configuraci�n para inicializaci�n. */

#define GPIO_LIST_END           0x00000000
#define GPIO_PIN_VALID          0x80000000
#define GPIO_IRQ_EDGE_H_TO_L    0x10000000
#define GPIO_PIN_STATUS_0       0x00000000
#define GPIO_PIN_STATUS_1       0x40000000
#define GPIO_PIN_IRQ            0x20000000

/* Definiciones para el sistema, no se usan como tal, sirven como referencia. */
#define GPIO_PIN_ADDR           0x00FFFFFF
#define GPIO_PIN_STATUS         0x40000000

/* Comandos IOCTL. */

#define GPIO_IOCTL_ADD_PINS         13
#define GPIO_IOCTL_WRITE_LOG0       14
#define GPIO_IOCTL_WRITE_LOG1       15
#define GPIO_IOCTL_READ             16
#define GPIO_IOCTL_SET_IRQ_FUNCTION 17

#define MAX_PORTS 10                // Aunque en realidad, solo 6 puertos est�n plasmados ya en la tarjeta.
typedef uint_32 GPIO_PIN_STRUCT;    // Su estructura basta con 32 bits.

/*
 *  Estructura de mapeo de GPIO
 *  de los pines que se van 'ocupando' en los archivos.
 */

typedef struct pin_mapping
{
    uint_32 memory8 [MAX_PORTS];

} GPIO_PIN_MAP, _PTR_ GPIO_PIN_MAP_PTR;

/*
 *  Estructura de mapeo de GPIO
 *  de los pines con prop�sito de interrupci�n.
 */

typedef struct irq_mapping
{
    uint_32 memory8 [MAX_PORTS];

} GPIO_IRQ_MAP, _PTR_ GPIO_IRQ_MAP_PTR;

/* Necesario saber si es entrada o salida. */
typedef enum
{
  DEV_INPUT,
  DEV_OUTPUT
} device_type;

/*
 *  Estructura gpio_device_struct.
 *  Funci�n de interrupci�n, mapeo de pines, cuales tienen interrupci�n,
 *  flanco de interrupci�n, y el tipo (entrada o salida).
 */

typedef struct gpio_device_struct
{
    pointer                             irq_func;
    GPIO_PIN_MAP                        pin_map;
    GPIO_IRQ_MAP                        irq_map;
    GPIO_IRQ_MAP                        irq_edge_map;
    uint_32                             type;

} GPIO_DEV_DATA, _PTR_ GPIO_DEV_DATA_PTR;

/* Funciones b�sicas pata el dispositivo. */

extern _mqx_int gpio_open       (FILE_PTR_f fd_ptr, char_ptr open_name_ptr, char_ptr flags);
extern _mqx_int gpio_cpu_open   (FILE_PTR_f fd_ptr, char_ptr file_name, char_ptr param_ptr);
extern _mqx_int gpio_close      (FILE _PTR_ fd_ptr);
extern _mqx_int gpio_read       (FILE_PTR_f fd_ptr, char_ptr data_ptr, _mqx_int num);
extern _mqx_int gpio_ioctl      (FILE_PTR_f fd_ptr, _mqx_uint cmd, pointer param_ptr);

#endif /* GPIO_F_MSP432_H_ */
