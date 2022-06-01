 //FileName:        Files.c
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Tratamiento de archivos por funciones de apertura, cierre, control y lectura. Source File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"

#define GPIO_FILE   0
#define ADC_FILE    1
#define UART_FILE   2
#define TIME_FILE   3
#define MAX_FILES   3

// Indicación del formato de archivo: puede ser gpio, adc o uart.
FILES_GENERATED files_active = {.gen_files[0] = "gpi",
                                .gen_files[1] = "adc",
                                .gen_files[2] = "uar",
                                .gen_files[3] = "tim",
                                .ext = ".bin"          };

/*FUNCTION*-------------------------------------------------------------------
 * Function: gen_name_file
 * Preconditions: None.
 * Overview: Establece un nombre en cadena string único para un dispositivo.
 * Output: None.
*END*----------------------------------------------------------------------*/

void gen_name_file (int file_type, char_ptr message)
{
    static int iter_name[] = {NO_FILE, NO_FILE, NO_FILE, NO_FILE};
    char form[5]           = ".bin";
    char num [2]           = "";

    (iter_name[file_type])++;
    num[0] = ((char) (iter_name[file_type])) + '0';

    strcpy(message, files_active.gen_files[file_type]);
    strcat(message, num);
    strcat(message, form);
}

/*FUNCTION*-------------------------------------------------------------------
 * Function: fopen_f
 * Preconditions: None.
 * Overview: Creación de un objeto que se guarda en un archivo.
 * Output: Tipo de dato FILE.
*END*----------------------------------------------------------------------*/

FILE _PTR_ fopen_f (const char _PTR_ open_type_ptr, const char _PTR_ open_mode_ptr)
{
    FILE_f                      file[1];
    FILE_PTR_f                  file_ptr = file;
    IO_DEVICE_STRUCT_PTR        dev_ptr;
    _mqx_int                    result;

    char _PTR_                  dev_name_ptr;
    char _PTR_                  tmp_ptr;
    char                        name[9];

    int                         match = 0;
    FILE                        *fptr;

    dev_ptr =  (IO_DEVICE_STRUCT_PTR) &instruction_set[match];

    // Encontrar el tipo de dispositivo.
    while (match <= MAX_FILES)
    {
          dev_name_ptr =  dev_ptr ->  IDENTIFIER;
          tmp_ptr      = (char _PTR_) open_type_ptr;
          while (*tmp_ptr && *dev_name_ptr && (*tmp_ptr == *dev_name_ptr))
          {
             ++tmp_ptr;
             ++dev_name_ptr;
          }

          if (*dev_name_ptr == '\0')
             break;

          dev_ptr = (IO_DEVICE_STRUCT_PTR) &instruction_set[++match];
    }

    // Dependiendo del tipo de archivo, se llama a una función diferente.
    file_ptr -> DEV_PTR = dev_ptr;
    if (dev_ptr->IO_OPEN != NULL_POINTER)
    {
          result = (*dev_ptr->IO_OPEN)(file_ptr, (char _PTR_) open_type_ptr, (char _PTR_) open_mode_ptr);
          if (result != OPEN_OK)
          {
              return(NULL_POINTER);
          }
    }

    // Genera el nombre.
    gen_name_file(match, name);

    Int_disable();

    // Dependiendo del tipo de archivo, se crea el archivo.
    switch(match)
    {
        case GPIO_FILE: fptr = fopen(name,  MODE);  break;
        case ADC_FILE : fptr = fopen(name , MODE);  break;
        case UART_FILE: fptr = fopen(name,  MODE);  break;
        case TIME_FILE: fptr = fopen(name,  MODE);  break;
        default: fptr = NULL_POINTER;               break;
    }

    fwrite(file, sizeof(file), 1, fptr);    // Escribir dentro del archivo.
    rewind(fptr);                           // De vuelta al inicio del archivo.
    Int_enable();

    return fptr;
}

/*FUNCTION*------------------------------------------------------------------------
 * Function: ioctl
 * Preconditions: None.
 * Overview: Control de los recursos que conforman el objeto dentro de un archivo.
 * Output: IO_OK o IO_ERR.
*END*-----------------------------------------------------------------------------*/

_mqx_int ioctl (FILE _PTR_ file_ptr, _mqx_uint cmd,  pointer param_ptr)
{
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   FILE_f                 struct_file[1];
   FILE_PTR_f             struct_file_ptr;
   _mqx_uint              result = IO_OK;

   Int_disable();
       fread(struct_file, sizeof(struct_file),1, file_ptr); // Leer el fichero.
       rewind(file_ptr);                                    // Apuntador W/R de nuevo al inicio.
   Int_enable();

   struct_file_ptr = struct_file;

   if (struct_file_ptr == NULL)
      return(IO_ERR);

   dev_ptr = struct_file_ptr->DEV_PTR;

   Int_disable();
       // Llamar a la función IOCTL correspondiente.
       if (dev_ptr->IO_IOCTL != NULL)
          result = (*dev_ptr->IO_IOCTL)(struct_file_ptr, cmd, param_ptr);
       else
          result = IO_ERR;

       fwrite(struct_file_ptr, sizeof(struct_file_ptr), 1, file_ptr);       // Escribir dentro del archivo también se puede,
       rewind(file_ptr);                                                    // siempre y cuando se tenga un rewind ...
   Int_enable();                                                            // ... (apuntador de nuevo al inicio).

   return(result);

}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : fread_f
* Returned Value   : _mqx_int
* Comments         :
*    La lectura toma un apuntador y guarda los valores pedidos en su dirección.
*
*END*----------------------------------------------------------------------*/

_mqx_int fread_f (FILE _PTR_ file_ptr, pointer data_ptr, _mqx_int num)
{
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   FILE_f                 struct_file[1];
   FILE_PTR_f             struct_file_ptr;
   _mqx_uint              flag = IO_OK;

   Int_disable();
       fread(struct_file, sizeof(struct_file),1, file_ptr);     // Leer el archivo.
       rewind(file_ptr);                                        // Apuntador W/R de nuevo al inicio.
   Int_enable();

   struct_file_ptr = struct_file;

   if (struct_file_ptr == NULL)
      return(IO_ERR);

   dev_ptr = struct_file_ptr->DEV_PTR;

   if (dev_ptr->IO_READ == NULL)
   {
      struct_file_ptr->ERROR = IO_ERR;
      return(IO_ERR);
   }

   // Llamado a la función apuntada.
   flag = ((*dev_ptr->IO_READ)(struct_file_ptr, data_ptr, num)) == 0;
   return(flag);
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : fclose_f
* Returned Value   : _mqx_int
* Comments         : Cierre de archivos.
*END*----------------------------------------------------------------------*/

_mqx_int fclose_f (FILE _PTR_ file_ptr)
{
   IO_DEVICE_STRUCT_PTR   dev_ptr;
   _mqx_uint              result;
   FILE_f                 struct_file[1];
   FILE_PTR_f             struct_file_ptr;

   Int_disable();
       fread(struct_file, sizeof(struct_file),1, file_ptr); // Leer el fichero.
       rewind(file_ptr);                                    // Apuntador W/R de nuevo al inicio.
   Int_enable();

   struct_file_ptr = struct_file;

   if (struct_file_ptr == NULL)
      return(IO_ERR);

   dev_ptr = struct_file_ptr->DEV_PTR;
   if (dev_ptr->IO_CLOSE == NULL)
   {
       free(file_ptr);
       return(IO_ERR);
   }

   result = (*dev_ptr->IO_CLOSE)(file_ptr);                 // Abrir la función del cierre del archivo.
   return(result);
}
