 //FileName:        Files.h
 //Dependencies:    None.
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Tratamiento de archivos por funciones de apertura, cierre, control y lectura. Header File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#ifndef FILES_H_
#define FILES_H_

// DEFINICIONES BÁSICAS.

#define NONE        0

#define GPIO_FILE   0
#define ADC_FILE    1
#define UART_FILE   2

#define FILE_READY  1
#define NO_FILE     0

#define OPEN_OK     0
#define MODE        "wb+"

#define FUNC_OK     0
#define ERR_FUNC    1

#define IO_OK      0
#define IO_ERR     1

#define NULL_POINTER ((void *)0)

/*
 * Estructura files_generated con el formato
 * para crear los dispositivos en forma de archivo.
 */

typedef struct files_generated
{
   char_ptr        gen_files[4];
   const char_ptr  ext;

} FILES_GENERATED;

// Funciones basicas.

// Generador de nombre para un archivo (que sean siempre distintos).
extern void gen_name_file (int file_type, char_ptr message);

// A estas funciones se acceden antes de entrar a cada función específica de un dispositivo.

extern FILE _PTR_ fopen_f (const char _PTR_ open_type_ptr, const char _PTR_ open_mode_ptr);
extern _mqx_int   ioctl (FILE _PTR_ file_ptr, _mqx_uint cmd,  pointer param_ptr);
extern _mqx_int   fclose_f (FILE _PTR_ file_ptr);
extern _mqx_int   fread_f (FILE _PTR_ file_ptr, pointer data_ptr, _mqx_int num);

#endif /* FILES_H_ */
