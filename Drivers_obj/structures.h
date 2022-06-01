 //FileName:        structures.h
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Definición de estructuras y mapeos en drivers. Header File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include "../Drivers_obj/types.h"

/*
 *  Estructura file_struct.
 *  Esta es la forma de la estructura que guarda cada archivo
 *  tipo FILE, conteniendo un formato de estructura io_device_struct,
 *  datos relacionados al dispositivo único en contexto, y manejador de errores
 *  (este último, con poco o nulo uso).
 */

typedef struct file_struct
{
    struct io_device_struct _PTR_       DEV_PTR;
    pointer                             DEV_DATA_PTR;
    _mqx_uint                           ERROR;

} FILE_f, _PTR_ FILE_PTR_f;


/*
 *  Estructura io_device_struct
 *  Un dispositivo I/O (GPIO, ADC, UART, TIMER) debe tener su identificador,
 *  y sus propias funciones de acuerdo al mismo.
 */

typedef struct io_device_struct
{
    char_ptr                IDENTIFIER;
    _mqx_int                (_CODE_PTR_ IO_OPEN) (FILE_PTR_f, char_ptr, char_ptr);
    _mqx_int                (_CODE_PTR_ IO_CLOSE)(FILE _PTR_);
    _mqx_int                (_CODE_PTR_ IO_READ) (FILE_PTR_f, char_ptr, _mqx_int);
    _mqx_int                (_CODE_PTR_ IO_IOCTL)(FILE_PTR_f, _mqx_uint, pointer);

} IO_DEVICE_STRUCT, _PTR_ IO_DEVICE_STRUCT_PTR;

#endif /* STRUCTURES_H_ */
