 //FileName:        gpio_f_MSP432.c
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para GPIO por medio de archivos. Source File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"

GPIO_PIN_MAP gpio_global_pin_map = {.memory8[0] = 0, .memory8[1] = 0, .memory8[2] = 0,
                                    .memory8[3] = 0, .memory8[4] = 0, .memory8[5] = 0},
            gpio_global_irq_map =  {.memory8[0] = 0, .memory8[1] = 0, .memory8[2] = 0,
                                    .memory8[3] = 0, .memory8[4] = 0, .memory8[5] = 0};

/*FUNCTION***************************************************************************
*
* Function Name    : gpio_open
* Returned Value   : IO_OK or IO_ERR
* Comments         :
*    Prepara los archivos para su uso. Reservación de memoria y llenado de formatos.
*
*END********************************************************************************/

_mqx_int gpio_open  (FILE_PTR_f fd_ptr, char_ptr open_name_ptr, char_ptr flags)
{
    _mqx_int            i;
    uint_8              pin;
    uint_32             addr;

    GPIO_DEV_DATA_PTR   dev_data_ptr;
    GPIO_PIN_STRUCT _PTR_ pin_table = (GPIO_PIN_STRUCT _PTR_) flags;

    if (NULL == (dev_data_ptr = (GPIO_DEV_DATA_PTR) malloc(sizeof(GPIO_DEV_DATA)))) // Reservación de memoria de datos.
        return ERR_FUNC;

    // Inicialización en cero's.
    dev_data_ptr -> irq_func = 0;
    for(i = 0; i < (MAX_PORTS); i++)
    {
       dev_data_ptr-> pin_map.memory8 [i] = 0;
       dev_data_ptr-> irq_map.memory8[i] = 0;
       dev_data_ptr-> irq_edge_map.memory8[i] = 0;
    }
    dev_data_ptr -> type = 0;


    fd_ptr -> DEV_DATA_PTR = (pointer) dev_data_ptr;    // Esta información, para GPIO se guarda en el apuntador vacío del disp.

    /* Checar puertos y pines. */

    Int_disable();                                                          // Conviene desactivar interrupciones.
    for (; *pin_table != GPIO_LIST_END; pin_table++)
    {
        if (*pin_table & GPIO_PIN_VALID)                                    // Valida pin.
        {
            addr = (*pin_table & GPIO_PIN_ADDR) >> 3;                       // Encuentra puerto.
            pin = 1 << (*pin_table & 0x07);                                 // Prepara mascara de bit.

            if (addr > 0 && addr <= MAX_PORTS)                              // Valida puerto.
                if (! (gpio_global_pin_map.memory8[addr-1] & pin))          // El pin es usado ya en otro archivo?
                {
                    if (*pin_table & GPIO_PIN_IRQ)
                    {
                        if (!(gpio_global_irq_map.memory8[addr-1] & pin))
                        {                                                       // Con o sin función de interrupción.
                            dev_data_ptr->irq_map.memory8[addr-1] |= pin;       // Marca pin para uso en interrupción.
                            dev_data_ptr->pin_map.memory8[addr-1] |= pin;

                            if(*pin_table & GPIO_IRQ_EDGE_H_TO_L)               // Ante interrupción, modo de flanco.
                                dev_data_ptr->irq_edge_map.memory8[addr-1]|= pin;
                            continue;
                        }
                    }
                    else
                    {
                        dev_data_ptr->pin_map.memory8[addr-1] |= pin;       // Solo marca pin, sin interrupción.
                        continue;
                    }
                }
        }

        free(dev_data_ptr); // Libera memoria temporal al haber procedimiento incorrecto.
        Int_enable();       // Reanudación de interrupciones.
        return IO_ERR;
    }

    if (IO_OK != gpio_cpu_open(fd_ptr, open_name_ptr, flags))   // Configura realmente pines en HW.
    {
            free(dev_data_ptr);
            return IO_ERR;
    }

    /* Ahora se llena el uso de bits pero en la estructura general. */
    for (i = 0; i < MAX_PORTS; i++)
    {
        gpio_global_pin_map.memory8[i] |= dev_data_ptr->pin_map.memory8[i];
        gpio_global_irq_map.memory8[i] |= dev_data_ptr->irq_map.memory8[i];
    }

    Int_enable();       // Reanudación de interrupciones.
    return IO_OK;
}

/*FUNCTION*****************************************************************
*
* Function Name    : gpio_cpu_open
* Returned Value   : IO_OK or error
* Comments         :
*    Implementa la verdadera inicialización en HW del GPIO.
*
*END*********************************************************************/

_mqx_int gpio_cpu_open (FILE_PTR_f fd_ptr, char_ptr file_name, char_ptr param_ptr)
{
   static _mqx_int   bandera = 0;
   _mqx_int          i;
   GPIO_DEV_DATA_PTR dev_data_ptr = (GPIO_DEV_DATA_PTR) fd_ptr->DEV_DATA_PTR;


   GPIO_PIN_STRUCT _PTR_   pin_table;
   uint_32                 addr;
   uint_8                  pin;
   GPIO_PIN_MAP_PTR        temp_pin_map_ptr;

   /* Determina accesso de GPIO (I o O). */
    if ((file_name != NULL) && (*file_name != 0))
    {
        if (!strncmp(file_name, "gpio:write", 11))
            dev_data_ptr->type = DEV_OUTPUT;
        else if (!strncmp(file_name, "gpio:output", 12))
            dev_data_ptr->type = DEV_OUTPUT;
        else if (!strncmp(file_name, "gpio:read", 10))
            dev_data_ptr->type = DEV_INPUT;
        else if (!strncmp(file_name, "gpio:input", 11))
            dev_data_ptr->type = DEV_INPUT;
        else
            return IO_ERR;                                  /* Error. */
    }
    else
            return IO_ERR;                                  /* Error. */

    if ((param_ptr != NULL) && (dev_data_ptr->type == DEV_OUTPUT))
    {
        // Reservación de memoria.
        if (NULL == (temp_pin_map_ptr = (GPIO_PIN_MAP_PTR) malloc(sizeof(GPIO_PIN_MAP))))
            return IO_ERR;

        // Inicialización en cero's.
        for(i = 0; i < (MAX_PORTS); i++)
        {
           temp_pin_map_ptr-> memory8[i] = 0;
        }

        // Prepara mapeo de pin.
        for (pin_table = (GPIO_PIN_STRUCT _PTR_) param_ptr; *pin_table != GPIO_LIST_END; pin_table++)
        {
            addr = (*pin_table & GPIO_PIN_ADDR) >> 3;           /* Puerto. */
            pin = 1 << (*pin_table & 0x07);                     /* Máscara de bit. */
            if (*pin_table & GPIO_PIN_STATUS)
                temp_pin_map_ptr->memory8[addr-1] |= pin;
        }

        /* Aplicación de la salida inicial según respuesta recibida. */

        P1 -> OUT = temp_pin_map_ptr->memory8[0];
        P2 -> OUT = temp_pin_map_ptr->memory8[1];
        P3 -> OUT = temp_pin_map_ptr->memory8[2];
        P4 -> OUT = temp_pin_map_ptr->memory8[3];
        P5 -> OUT = temp_pin_map_ptr->memory8[4];
        P6 -> OUT = temp_pin_map_ptr->memory8[5];
        P7 -> OUT = temp_pin_map_ptr->memory8[6];
        P8 -> OUT = temp_pin_map_ptr->memory8[7];
        P9 -> OUT = temp_pin_map_ptr->memory8[8];
        P10 -> OUT =temp_pin_map_ptr->memory8[9];

       free(temp_pin_map_ptr);
    }

    if (dev_data_ptr->type == DEV_OUTPUT)               // Tipo salida.
    {
       P1 -> DIR |= dev_data_ptr->pin_map.memory8[0];
       P2 -> DIR |= dev_data_ptr->pin_map.memory8[1];
       P3 -> DIR |= dev_data_ptr->pin_map.memory8[2];
       P4 -> DIR |= dev_data_ptr->pin_map.memory8[3];
       P5 -> DIR |= dev_data_ptr->pin_map.memory8[4];
       P6 -> DIR |= dev_data_ptr->pin_map.memory8[5];
       P7 -> DIR |= dev_data_ptr->pin_map.memory8[6];
       P8 -> DIR |= dev_data_ptr->pin_map.memory8[7];
       P9 -> DIR |= dev_data_ptr->pin_map.memory8[8];
       P10 -> DIR |=dev_data_ptr->pin_map.memory8[9];

   }

   else                                                 // Tipo entrada.
   {
       P1 -> DIR &= ~dev_data_ptr->pin_map.memory8[0];
       P2 -> DIR &= ~dev_data_ptr->pin_map.memory8[1];
       P3 -> DIR &= ~dev_data_ptr->pin_map.memory8[2];
       P4 -> DIR &= ~dev_data_ptr->pin_map.memory8[3];
       P5 -> DIR &= ~dev_data_ptr->pin_map.memory8[4];
       P6 -> DIR &= ~dev_data_ptr->pin_map.memory8[5];
       P7 -> DIR &= ~dev_data_ptr->pin_map.memory8[6];
       P8 -> DIR &= ~dev_data_ptr->pin_map.memory8[7];
       P9 -> DIR &= ~dev_data_ptr->pin_map.memory8[8];
       P10 -> DIR &=~dev_data_ptr->pin_map.memory8[9];

       P1 -> REN |= dev_data_ptr->pin_map.memory8[0];  // Habilitar resistencias de pull-up.
       P2 -> REN |= dev_data_ptr->pin_map.memory8[1];
       P3 -> REN |= dev_data_ptr->pin_map.memory8[2];
       P4 -> REN |= dev_data_ptr->pin_map.memory8[3];
       P5 -> REN |= dev_data_ptr->pin_map.memory8[4];
       P6 -> REN |= dev_data_ptr->pin_map.memory8[5];
       P7 -> REN |= dev_data_ptr->pin_map.memory8[6];
       P8 -> REN |= dev_data_ptr->pin_map.memory8[7];
       P9 -> REN |= dev_data_ptr->pin_map.memory8[8];
       P10 -> REN |=dev_data_ptr->pin_map.memory8[9];

       P1 -> OUT |= dev_data_ptr->pin_map.memory8[0];
       P2 -> OUT |= dev_data_ptr->pin_map.memory8[1];  // Esto es necesario en algunas tarjetas...
       P3 -> OUT |= dev_data_ptr->pin_map.memory8[2];
       P4 -> OUT |= dev_data_ptr->pin_map.memory8[3];
       P5 -> OUT |= dev_data_ptr->pin_map.memory8[4];
       P6 -> OUT |= dev_data_ptr->pin_map.memory8[5];
       P3 -> OUT |= dev_data_ptr->pin_map.memory8[6];
       P4 -> OUT |= dev_data_ptr->pin_map.memory8[7];
       P5 -> OUT |= dev_data_ptr->pin_map.memory8[8];
       P6 -> OUT |= dev_data_ptr->pin_map.memory8[9];

       P1 -> IFG &= ~dev_data_ptr->pin_map.memory8[0];  // Elimina alguna posible bandera de interrupción.
       P2 -> IFG &= ~dev_data_ptr->pin_map.memory8[1];
       P3 -> IFG &= ~dev_data_ptr->pin_map.memory8[2];
       P4 -> IFG &= ~dev_data_ptr->pin_map.memory8[3];
       P5 -> IFG &= ~dev_data_ptr->pin_map.memory8[4];
       P6 -> IFG &= ~dev_data_ptr->pin_map.memory8[5];
       P3 -> IFG &= ~dev_data_ptr->pin_map.memory8[6];
       P4 -> IFG &= ~dev_data_ptr->pin_map.memory8[7];
       P5 -> IFG &= ~dev_data_ptr->pin_map.memory8[8];
       P6 -> IFG &= ~dev_data_ptr->pin_map.memory8[9];

       if(bandera == 0)                                 // En un inicio, es correcto eliminar banderas en registros.
       {
           P1 -> IFG = 0x00;    P1-> IE = dev_data_ptr-> irq_map.memory8[0]; P1 -> IES =  dev_data_ptr-> irq_edge_map.memory8[0];
           P2 -> IFG = 0x00;    P2-> IE = dev_data_ptr-> irq_map.memory8[1]; P2 -> IES =  dev_data_ptr-> irq_edge_map.memory8[1];
           P3 -> IFG = 0x00;    P3-> IE = dev_data_ptr-> irq_map.memory8[2]; P3 -> IES =  dev_data_ptr-> irq_edge_map.memory8[2];
           P4 -> IFG = 0x00;    P4-> IE = dev_data_ptr-> irq_map.memory8[3]; P4 -> IES =  dev_data_ptr-> irq_edge_map.memory8[3];
           P5 -> IFG = 0x00;    P5-> IE = dev_data_ptr-> irq_map.memory8[4]; P5 -> IES =  dev_data_ptr-> irq_edge_map.memory8[4];
           P6 -> IFG = 0x00;    P6-> IE = dev_data_ptr-> irq_map.memory8[5]; P6 -> IES =  dev_data_ptr-> irq_edge_map.memory8[5];
           P7 -> IFG = 0x00;    P7-> IE = dev_data_ptr-> irq_map.memory8[6]; P7 -> IES =  dev_data_ptr-> irq_edge_map.memory8[6];
           P8 -> IFG = 0x00;    P8-> IE = dev_data_ptr-> irq_map.memory8[7]; P8 -> IES =  dev_data_ptr-> irq_edge_map.memory8[7];
           P9 -> IFG = 0x00;    P9-> IE = dev_data_ptr-> irq_map.memory8[8]; P9 -> IES =  dev_data_ptr-> irq_edge_map.memory8[8];
           P10-> IFG = 0x00;    P10->IE = dev_data_ptr-> irq_map.memory8[9]; P10-> IES =  dev_data_ptr-> irq_edge_map.memory8[9];
           bandera = 1;
       }

       else
       {
           P1-> IE |= dev_data_ptr-> irq_map.memory8[0]; P1 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[0];
           P2-> IE |= dev_data_ptr-> irq_map.memory8[1]; P2 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[1];
           P3-> IE |= dev_data_ptr-> irq_map.memory8[2]; P3 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[2];
           P4-> IE |= dev_data_ptr-> irq_map.memory8[3]; P4 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[3];
           P5-> IE |= dev_data_ptr-> irq_map.memory8[4]; P5 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[4];
           P6-> IE |= dev_data_ptr-> irq_map.memory8[5]; P6 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[5];
           P7-> IE |= dev_data_ptr-> irq_map.memory8[6]; P7 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[6];
           P8-> IE |= dev_data_ptr-> irq_map.memory8[7]; P8 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[7];
           P9-> IE |= dev_data_ptr-> irq_map.memory8[8]; P9 -> IES |=  dev_data_ptr-> irq_edge_map.memory8[8];
           P10->IE |= dev_data_ptr-> irq_map.memory8[9]; P10-> IES |=  dev_data_ptr-> irq_edge_map.memory8[9];
       }
   }

    return IO_OK;

}


/*FUNCTION*****************************************************************
*
* Function Name    : gpio_cpu_ioctl
* Returned Value   : depends on IOCTL command
* Comments         :
*    Implementa funciones de control para el GPIO.
*
*END*********************************************************************/

_mqx_int gpio_ioctl (FILE_PTR_f fd_ptr, _mqx_uint cmd, pointer param_ptr)
{
    _mqx_int            i;
    GPIO_DEV_DATA_PTR  dev_data_ptr = (GPIO_DEV_DATA_PTR) fd_ptr -> DEV_DATA_PTR;

       switch (cmd)
       {

           case GPIO_IOCTL_ADD_PINS:
           {
               // Agregar pines al archivo.
               GPIO_PIN_STRUCT _PTR_  pin_table;
               uint_32        addr;
               uint_8         pin;

               // Checar si no son usados ya por otro puerto.
               Int_disable();
               for (pin_table = (GPIO_PIN_STRUCT _PTR_) param_ptr; *pin_table != GPIO_LIST_END; pin_table++)
               {
                   if (*pin_table & GPIO_PIN_VALID)                                            // Validación bit.
                   {
                       addr = (*pin_table & GPIO_PIN_ADDR) >> 3;                               // Puerto.
                       pin = 1 << (*pin_table & 0x07);                                         // Máscara de bit.
                       if (addr < MAX_PORTS)                                                   // Validación puerto.
                           if (!(gpio_global_pin_map.memory8[addr-1] & pin))                   // Chequeo.
                               continue;                                                       // Siguiente chequeo de pin.
                   }                                                                           // Algún problema ocurrió.
                   Int_enable();
                   return IO_ERR;
               }

               // Chequeo correcto, se procede a agregar los pines.
               for (pin_table = (GPIO_PIN_STRUCT _PTR_) param_ptr; *pin_table != GPIO_LIST_END; pin_table++)
               {
                   addr = (*pin_table & GPIO_PIN_ADDR)  >> 3;                                   // Puerto.
                   pin = 1 << (*pin_table & 0x07);                                              // Máscara de bit.
                   dev_data_ptr->pin_map.memory8[addr-1] |= pin;                                // Validación puerto.
                   gpio_global_pin_map.memory8[addr-1] |= pin;                                  // Marcar pin como global.
               }

               if (dev_data_ptr->type == DEV_OUTPUT)                                            // Tipo salida.
               {
                  P1 -> DIR |= dev_data_ptr->pin_map.memory8[0];
                  P2 -> DIR |= dev_data_ptr->pin_map.memory8[1];
                  P3 -> DIR |= dev_data_ptr->pin_map.memory8[2];
                  P4 -> DIR |= dev_data_ptr->pin_map.memory8[3];
                  P5 -> DIR |= dev_data_ptr->pin_map.memory8[4];
                  P6 -> DIR |= dev_data_ptr->pin_map.memory8[5];
                  P7 -> DIR |= dev_data_ptr->pin_map.memory8[6];
                  P8 -> DIR |= dev_data_ptr->pin_map.memory8[7];
                  P9 -> DIR |= dev_data_ptr->pin_map.memory8[8];
                  P10-> DIR |= dev_data_ptr->pin_map.memory8[9];
              }

              else                                                                              // Tipo entrada.
              {
                  P1 -> DIR &= ~dev_data_ptr->pin_map.memory8[0];
                  P2 -> DIR &= ~dev_data_ptr->pin_map.memory8[1];
                  P3 -> DIR &= ~dev_data_ptr->pin_map.memory8[2];
                  P4 -> DIR &= ~dev_data_ptr->pin_map.memory8[3];
                  P5 -> DIR &= ~dev_data_ptr->pin_map.memory8[4];
                  P6 -> DIR &= ~dev_data_ptr->pin_map.memory8[5];
                  P7 -> DIR &= ~dev_data_ptr->pin_map.memory8[6];
                  P8 -> DIR &= ~dev_data_ptr->pin_map.memory8[7];
                  P9 -> DIR &= ~dev_data_ptr->pin_map.memory8[8];
                  P10-> DIR &= ~dev_data_ptr->pin_map.memory8[9];

                  P1 -> REN |= dev_data_ptr->pin_map.memory8[0];                                // Habilitar resistencias de pull-up.
                  P2 -> REN |= dev_data_ptr->pin_map.memory8[1];
                  P3 -> REN |= dev_data_ptr->pin_map.memory8[2];
                  P4 -> REN |= dev_data_ptr->pin_map.memory8[3];
                  P5 -> REN |= dev_data_ptr->pin_map.memory8[4];
                  P6 -> REN |= dev_data_ptr->pin_map.memory8[5];
                  P6 -> REN |= dev_data_ptr->pin_map.memory8[5];
                  P7 -> REN |= dev_data_ptr->pin_map.memory8[6];
                  P8 -> REN |= dev_data_ptr->pin_map.memory8[7];
                  P9 -> REN |= dev_data_ptr->pin_map.memory8[8];
                  P10-> REN |= dev_data_ptr->pin_map.memory8[9];

                  P1 -> OUT |= dev_data_ptr->pin_map.memory8[0];                                // Esto es necesario en algunas
                  P2 -> OUT |= dev_data_ptr->pin_map.memory8[1];                                // tarjetas ...
                  P3 -> OUT |= dev_data_ptr->pin_map.memory8[2];
                  P4 -> OUT |= dev_data_ptr->pin_map.memory8[3];
                  P5 -> OUT |= dev_data_ptr->pin_map.memory8[4];
                  P6 -> OUT |= dev_data_ptr->pin_map.memory8[5];
                  P7 -> OUT |= dev_data_ptr->pin_map.memory8[6];
                  P8 -> OUT |= dev_data_ptr->pin_map.memory8[7];
                  P9 -> OUT |= dev_data_ptr->pin_map.memory8[8];
                  P10-> OUT |= dev_data_ptr->pin_map.memory8[9];
              }

              Int_enable();                                         // Renueva interrupciones.
           }
           break;

           // Poner en alto varias o todos las salidas (cuando entra de parámetro un NULL) del archivo.
           case GPIO_IOCTL_WRITE_LOG1:
           {

               if (dev_data_ptr->type != DEV_OUTPUT)
                   return IO_ERR;
               if (param_ptr == NULL)                               // Comando al archivo entero.
               {
                   Int_disable();
                   P1 -> OUT |= dev_data_ptr->pin_map.memory8[0];
                   P2 -> OUT |= dev_data_ptr->pin_map.memory8[1];
                   P3 -> OUT |= dev_data_ptr->pin_map.memory8[2];
                   P4 -> OUT |= dev_data_ptr->pin_map.memory8[3];
                   P5 -> OUT |= dev_data_ptr->pin_map.memory8[4];
                   P6 -> OUT |= dev_data_ptr->pin_map.memory8[5];
                   P6 -> OUT |= dev_data_ptr->pin_map.memory8[5];
                   P7 -> OUT |= dev_data_ptr->pin_map.memory8[6];
                   P8 -> OUT |= dev_data_ptr->pin_map.memory8[7];
                   P9 -> OUT |= dev_data_ptr->pin_map.memory8[8];
                   P10-> OUT |= dev_data_ptr->pin_map.memory8[9];
                   Int_enable();
                   break;
               }

               // De otro modo, solo las salidas marcadas.
               else
               {
                   GPIO_PIN_STRUCT _PTR_  pin_table;
                   uint_32                addr;
                   uint_8                 pin;
                   GPIO_PIN_MAP_PTR       temp_pin_map_ptr;

                   if (NULL == (temp_pin_map_ptr = (GPIO_PIN_MAP_PTR) malloc(sizeof(GPIO_PIN_MAP))))
                       return IO_ERR;

                   Int_disable();
                   for(i = 0; i < (MAX_PORTS); i++)
                   {
                      temp_pin_map_ptr-> memory8[i] = 0;
                   }

                   // Ya comentado arriba.
                   for (pin_table = (GPIO_PIN_STRUCT _PTR_) param_ptr; *pin_table != GPIO_LIST_END; pin_table++)
                   {
                       if (*pin_table & GPIO_PIN_VALID) {
                           addr = (*pin_table & GPIO_PIN_ADDR) >> 3;
                           pin = 1 << (*pin_table & 0x07);
                           if (addr < MAX_PORTS)
                               if (dev_data_ptr->pin_map.memory8[addr-1] & pin)
                               {
                                   temp_pin_map_ptr->memory8[addr-1] |= pin;
                                   continue;
                               }
                       }

                       Int_enable();                        // Renueva interrupciones.
                       free(temp_pin_map_ptr);
                       return IO_ERR;
                   }

                   // Se aplica nuevo mapa.

                   P1 -> OUT |= temp_pin_map_ptr->memory8[0];
                   P2 -> OUT |= temp_pin_map_ptr->memory8[1];
                   P3 -> OUT |= temp_pin_map_ptr->memory8[2];
                   P4 -> OUT |= temp_pin_map_ptr->memory8[3];
                   P5 -> OUT |= temp_pin_map_ptr->memory8[4];
                   P6 -> OUT |= temp_pin_map_ptr->memory8[5];
                   P7 -> OUT |= temp_pin_map_ptr->memory8[6];
                   P8 -> OUT |= temp_pin_map_ptr->memory8[7];
                   P9 -> OUT |= temp_pin_map_ptr->memory8[8];
                   P10-> OUT |= temp_pin_map_ptr->memory8[9];

                   Int_enable();                                // Renueva interrupciones.
                   free(temp_pin_map_ptr);
               }
           }
           break;

           // Apagar varios o todos los led's (cuando entra de parámetro un NULL) del archivo.
           case GPIO_IOCTL_WRITE_LOG0:
           {
               if (dev_data_ptr->type != DEV_OUTPUT)
                   return IO_ERR;


               if (param_ptr == NULL)                               // Comando al archivo entero.
               {
                   Int_disable();
                   P1 -> OUT &= ~dev_data_ptr->pin_map.memory8[0];
                   P2 -> OUT &= ~dev_data_ptr->pin_map.memory8[1];
                   P3 -> OUT &= ~dev_data_ptr->pin_map.memory8[2];
                   P4 -> OUT &= ~dev_data_ptr->pin_map.memory8[3];
                   P5 -> OUT &= ~dev_data_ptr->pin_map.memory8[4];
                   P6 -> OUT &= ~dev_data_ptr->pin_map.memory8[5];
                   P7 -> OUT &= ~dev_data_ptr->pin_map.memory8[6];
                   P8 -> OUT &= ~dev_data_ptr->pin_map.memory8[7];
                   P9 -> OUT &= ~dev_data_ptr->pin_map.memory8[8];
                   P10 ->OUT &= ~dev_data_ptr->pin_map.memory8[9];
                   Int_enable();                                    // Renueva interrupciones.
                   break;
               }

               // De otro modo, solo los led's marcados.
               else
               {
                   GPIO_PIN_STRUCT _PTR_  pin_table;
                   uint_32                addr;
                   uint_8                 pin;
                   GPIO_PIN_MAP_PTR       temp_pin_map_ptr;

                   if (NULL == (temp_pin_map_ptr = (GPIO_PIN_MAP_PTR) malloc(sizeof(GPIO_PIN_MAP))))
                       return IO_ERR;

                   for(i = 0; i < (MAX_PORTS); i++)
                   {
                      temp_pin_map_ptr-> memory8[i] = 0;
                   }

                   Int_disable();

                   // Ya comentado arriba.
                   for (pin_table = (GPIO_PIN_STRUCT _PTR_) param_ptr; *pin_table != GPIO_LIST_END; pin_table++)
                   {
                       if (*pin_table & GPIO_PIN_VALID)
                       {
                           addr = (*pin_table & GPIO_PIN_ADDR) >> 3;
                           pin = 1 << (*pin_table & 0x07);
                           if (addr < MAX_PORTS)
                               if (dev_data_ptr->pin_map.memory8[addr-1] & pin)
                               {
                                   temp_pin_map_ptr->memory8[addr-1] |= pin;
                                   continue;
                               }
                       }

                       Int_enable();                            // Renueva interrupciones.
                       free(temp_pin_map_ptr);
                       return IO_ERR;
                   }

                   // Aplicar nuevo mapa.

                   P1 -> OUT &= ~temp_pin_map_ptr->memory8[0];
                   P2 -> OUT &= ~temp_pin_map_ptr->memory8[1];
                   P3 -> OUT &= ~temp_pin_map_ptr->memory8[2];
                   P4 -> OUT &= ~temp_pin_map_ptr->memory8[3];
                   P5 -> OUT &= ~temp_pin_map_ptr->memory8[4];
                   P6 -> OUT &= ~temp_pin_map_ptr->memory8[5];
                   P7 -> OUT &= ~temp_pin_map_ptr->memory8[6];
                   P8 -> OUT &= ~temp_pin_map_ptr->memory8[7];
                   P9 -> OUT &= ~temp_pin_map_ptr->memory8[8];
                   P10-> OUT &= ~temp_pin_map_ptr->memory8[9];
                   Int_enable();                                // Renueva interrupciones.
                   free(temp_pin_map_ptr);
               }
           }
           break;

           // Lee el estado de los pines de entrada.
           case GPIO_IOCTL_READ:
           {
               GPIO_PIN_STRUCT _PTR_  pin_table;
               uint_32                addr;
               uint_8                 pin;
               uint_8                 value;

               if (dev_data_ptr->type != DEV_INPUT)
                   return IO_ERR;
               if (param_ptr == NULL)                                                   // No hay de donde leer.
                   return IO_ERR;

               Int_disable();

               // Checar si todos los pines que se demandan leer están dentro del archivo.
               for (pin_table = (GPIO_PIN_STRUCT _PTR_) param_ptr; *pin_table != GPIO_LIST_END; pin_table++)
               {
                 if (*pin_table & GPIO_PIN_VALID)                                       // Checar validación de pin.
                   {
                       addr = (*pin_table & GPIO_PIN_ADDR) >> 3;                        // Puerto.
                       pin = 1 << (*pin_table & 0x07);                                  // Máscara de bit.
                       if (addr < MAX_PORTS)                                            // Fuera de rango?
                           if (dev_data_ptr->pin_map.memory8[addr-1] & pin)
                           {
                               switch(addr)
                               {
                                   case 1: value = (uint_8)(P1 -> IN) & (pin); break;
                                   case 2: value = (uint_8)(P2 -> IN) & (pin); break;
                                   case 3: value = (uint_8)(P3 -> IN) & (pin); break;
                                   case 4: value = (uint_8)(P4 -> IN) & (pin); break;
                                   case 5: value = (uint_8)(P5 -> IN) & (pin); break;
                                   case 6: value = (uint_8)(P6 -> IN) & (pin); break;
                                   case 7: value = (uint_8)(P7 -> IN) & (pin); break;
                                   case 8: value = (uint_8)(P8 -> IN) & (pin); break;
                                   case 9: value = (uint_8)(P9 -> IN) & (pin); break;
                                   case 10:value = (uint_8)(P10 -> IN)& (pin); break;
                                   default:value = 0;                          break;

                               }

                               if(value)
                                   *pin_table |= GPIO_PIN_STATUS;                    // Pone en alto pin_status de la lista.
                               else
                                   *pin_table &= ~GPIO_PIN_STATUS;                   // Pone en bajo pin_status de la lista.
                               continue;
                           }
                   }
                                                                                     // Algún problema ocurrió.
                 Int_enable();
                   return IO_ERR;
               }
               Int_enable();
           }
           break;

           // Establece función que se activará por todos los pines de todos los puertos con el bit irq_map.memory8[i].
            case GPIO_IOCTL_SET_IRQ_FUNCTION:
           {
               if (dev_data_ptr->type == DEV_OUTPUT)
                   return IO_ERR;                                             // No hay interrupciones en salidas.

               dev_data_ptr->irq_func = param_ptr;

               Int_disable();
               if (param_ptr != NULL)
               {
                   // Se relacionan todos los puertos involucrados con una sola función.
                   for(i = 0; i < MAX_PORTS; i++)
                   {
                       if((dev_data_ptr-> irq_map.memory8[i]) != 0)
                       {
                          Int_registerInterrupt(i + INT_PORT1, (void(*)(void)) dev_data_ptr->irq_func);
                          Int_enableInterrupt(i + INT_PORT1);
                       }
                   }
               }
               Int_enable();    // Se reanudan las interrupciones.

           }
           break;

           default:
               return IO_ERR;
       }

       return IO_OK;
}

/*FUNCTION*****************************************************************
*
* Function Name    : gpio_close
* Returned Value   : IO_OK or IO_ERR
* Comments         :
*    Cierre de archivo GPIO, con su implicación en estructuras generales.
*
*END*********************************************************************/

_mqx_int gpio_close (FILE _PTR_ fd_ptr)
{
    _mqx_int               i;
    GPIO_DEV_DATA_PTR      dev_data_ptr;
    FILE_f                 struct_file[1];
    FILE_PTR_f             struct_file_ptr;

    Int_disable();

    fread(struct_file, sizeof(struct_file),1, fd_ptr);
    rewind(fd_ptr);

    struct_file_ptr = struct_file;
    dev_data_ptr = (GPIO_DEV_DATA_PTR) struct_file_ptr -> DEV_DATA_PTR;

    ioctl (fd_ptr, GPIO_IOCTL_SET_IRQ_FUNCTION, NULL); // Retira función IRQ.

    // Excluye pines del mapeo global.
    for (i = 0; i < MAX_PORTS; i++)
    {
        gpio_global_pin_map.memory8[i] &= ~dev_data_ptr->pin_map.memory8[i];
        gpio_global_irq_map.memory8[i] &= ~dev_data_ptr->irq_map.memory8[i];
    }

    Int_enable();

    free(fd_ptr);

    return IO_OK;
}

/*FUNCTION*****************************************************************
*
* Function Name    : gpio_read
* Returned Value   : IO_OK or IO_ERR
* Comments         :
*    No hay definición de esta función, GPIO se controla por IOCTL.
*
*END*********************************************************************/

_mqx_int gpio_read  (FILE_PTR_f fd_ptr, char_ptr data_ptr, _mqx_int num)
{
    // No hay definición de gpio_read. Solo el adc y el timer tiene definición de esta función.
    return 0;
}
