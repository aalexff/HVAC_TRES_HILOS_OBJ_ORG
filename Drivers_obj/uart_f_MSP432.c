 //FileName:        uart_f_MSP432.c
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para UART por medio de archivos. Source File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"

/* Estructura general del dispositivo. */
UART_DEVICE_STRUCT_PTR uart = NULL;
boolean RX_interruption = FALSE;

const uint32_t BRX[]  = {78, 19, 6};                    // Constantes para la determinación del Baud Rate.
const uint32_t BRFX[] = {2,  8,  8};                    // (Technical Reference Manual - Pag 918).
const uint32_t BRS[]  = {0x0000, 0x0065, 0x0020};       // Definido para 12 MHz (BRCLK). CAMBIE SEGUN DISEÑO.

/* Definición de los puertos. */
static const uint32_t GPIO_PORT_TO_BASE[] =
{   0x00,
    0x40004C00,
    0x40004C01,
    0x40004C20,
    0x40004C21,
    0x40004C40,
    0x40004C41,
    0x40004C60,
    0x40004C61,
    0x40004C80,
    0x40004C81,
    0x40004D20
};


/*FUNCTION****************************************************************
*
* Function Name    : uart_open
* Returned Value   : IO_OK or IO_ERR
* Comments         : Reservación de memoria, llenado de datos,
*                    llama función de configuración de HW.
*END**********************************************************************/

_mqx_int uart_open (FILE_PTR_f fd_ptr, char _PTR_ open_name_ptr, char _PTR_ flags)
{
   UART_INIT_STRUCT_PTR uart_init_ptr = (UART_INIT_STRUCT_PTR) flags;
   _mqx_uint            result = IO_OK;
   static bool                 flag = 0;

   // Llenado en la estructura UART (solo lo importante).
   if(flag == 0)
   {
       uart = (UART_DEVICE_STRUCT_PTR) malloc(sizeof(UART_DEVICE_STRUCT));
       flag = 1;
   }

   switch(uart_init_ptr -> baud_rate)
   {
       case BR_9600:   uart -> baud_rate = 9600;   break;
       case BR_38400:  uart -> baud_rate = 38400;  break;
       case BR_115200: uart -> baud_rate = 115200; break;
       default: return(IO_ERR);
   }

   uart -> selected_port = uart_init_ptr -> selected_port;
   uart -> pins[0] = uart_init_ptr -> pins[0];
   uart -> pins[1] = uart_init_ptr -> pins[1];
   uart -> clk = uart_init_ptr -> clk;

   /* Llamada a la función que configura. */
   result = uart_hw_init(uart_init_ptr, open_name_ptr);

   return(result);
}


/*FUNCTION****************************************************************
*
* Function Name    : uart_hw_init
* Returned Value   : IO_OK or IO_ERR
* Comments         : Configuración de HW.
*
*END*********************************************************************/

uint_32 uart_hw_init (UART_INIT_STRUCT_PTR init_ptr, char _PTR_ open_name_ptr)
{
    /* Se apaga el módulo. */
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 , EUSCI_A_CTLW0_SWRST_OFS) = 1;

    /* Forma genérica para limpiar interrupciones, sleep, y otros registros inicialmente. */
    EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 = (EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0 & ~( UCRXEIE | UCBRKIE | UCDORM | UCTXADDR | UCTXBRK)) | EUSCI_A_CTLW0_MODE_0;

    UART_mode(ASYNCHRONOUS);                                             /* Módo Asíncrono habilitado (UART). */
    UART_set_location_pin(init_ptr->selected_port, init_ptr->pins[0]);   /* Selección de pines. */
    UART_set_location_pin(init_ptr->selected_port, init_ptr->pins[1]);   /* Selección de pines. */
    UART_clck_source(init_ptr -> clk);                                   /* Selección de la fuente de reloj. (Bit SMCLK) */
    UART_set_transmision_dir(init_ptr -> direction);                     /* Selección de MSB o LSB en orden de transmisión (0 para LSB, 1 para MSB). */
    UART_set_stop_bits(init_ptr -> stop_bits);                           /* Número de bits de paro; UCSPB = 0(1 stop bit) OR 1(2 stop bits). */
    UART_set_parity(init_ptr -> parity);                                 /* Paridad entre la comunicación. */

    UART_set_baud_rate((uint8_t) init_ptr -> baud_rate);                 /* Configuraciones posibles: 9600, 38400, 115200. */
    UART_data_bits(init_ptr -> data_bits);                               /* Selección de caracter 8 bits. */
    UART_set_oversampling(init_ptr -> oversampling);                     /* Sobremuestreo (Oversampling) */

    UART_E_char_IE(init_ptr -> interruption_E);                          /* Interrupción habilitada por caracteres erróneos. */
    UART_B_char_IE(init_ptr -> interruption_B);                          /* Interrupción habilitada por caracteres break.    */

    /* Se enciende el módulo. */
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 , EUSCI_A_CTLW0_SWRST_OFS) = 0;

   return(IO_OK);
}


/*FUNCTION****************************************************************
*
* Function Name    : uart_close
* Returned Value   : IO_OK
* Comments         : Cierre del archivo y de la estructura general.
*
*END**********************************************************************/

_mqx_int uart_close (FILE _PTR_ fd_ptr)
{
   if(uart != NULL)
   free(uart);
   free(fd_ptr);

   return(IO_OK);
}


/*FUNCTION***************************************************************************
*
* Function Name    : uart_read
* Returned Value   : IO_OK
* Comments         : No hay ninguna función definida para lectura, pero debe existir.
*
*END*********************************************************************************/

_mqx_int uart_read (FILE_PTR_f fd_ptr, char _PTR_ data_ptr, _mqx_int num)
{
   // No hay ninguna definición de esta función.
   return IO_OK;
}


/*FUNCTION*******************************************************************
* Function Name    : uart_ioctl
* Returned Value   : IO_OK
* Comments         : En la revisión solo se tiene un comando para habilitar
*                    la interrupción ante la recepción de un caracter.
*END************************************************************************/

_mqx_int uart_ioctl (FILE_PTR_f fd_ptr, _mqx_uint cmd, pointer param_ptr)
{
   switch(cmd)
   {
       case IO_IOCTL_SERIAL_IRQ_FUNCTION:
           Int_registerInterrupt(INT_EUSCIA0, (void(*)(void)) param_ptr);
           Int_enableInterrupt(INT_EUSCIA0);
           EUSCI_A0 -> IE = EUSCI_A_IE_RXIE;
           RX_interruption = TRUE;
           break;

       default: return IO_ERR;
   }

   return IO_OK;
}

/////////////////////////////////
// Funciones de configuración. //
/////////////////////////////////

/*FUNCTION*********************************************************************
*
* Function Name    : UART_set_location_pins
* Returned Value   : IO_OK or IO_ERR
* Comments         : Configuración de pin. Funciona bien con los pines default.
*
*END***************************************************************************/

void UART_set_location_pins(uint32_t selected_port,uint32_t selected_pins)
{
    HWREG16(selected_port + OFS_PADIR) &= ~(selected_pins);
    HWREG16(selected_port + OFS_PASEL0) |= (selected_pins);
    HWREG16(selected_port + OFS_PASEL1) &= ~(selected_pins);
}

/****************************************************************************
 * Function: UART_data_bits
 * Preconditions: None.
 * Overview: Establece el envío para 7 u 8 bits.
 * Input:  Número de bits (0 - 8 bits, 1 - 7 bits).
 * Output: None.
 *****************************************************************************/

void UART_data_bits(bool data_bits)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0, UC7BIT) = data_bits;
}


/****************************************************************************
 * Function: UART_clck_source
 * Preconditions: None.
 * Overview: Establece de donde se toma el reloj del puerto.
 * Input:  Enum con la selección del reloj.
 * Output: None.
 *****************************************************************************/
void UART_clck_source(Clk_source source)
{
    switch(source)
    {
        case U_CLK:  EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 = (EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 & ~UCSSEL_3) | EUSCI_A_CTLW0_SSEL__UCLK;  break;
        case A_CLK:  EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 = (EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 & ~UCSSEL_3) | EUSCI_A_CTLW0_SSEL__ACLK;  break;
        case SM_CLK: EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 = (EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 & ~UCSSEL_3) | EUSCI_A_CTLW0_SSEL__SMCLK; break;
        default: break;
    }
}


/*****************************************************************************
 * Function: UART_mode
 * Preconditions: None.
 * Overview: Establece de donde se toma el reloj del puerto.
 * Input:  Bool que indique modo síncrono (1) o asíncrono (0).
 * Output: None.
 *****************************************************************************/
void UART_mode(bool synchronization)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0, EUSCI_A_CTLW0_SYNC) = synchronization;
}


/*****************************************************************************
 * Function: UART_B_char_IE
 * Preconditions: None.
 * Overview: Pone en alto la interrupción por carácteres 'break'.
 * Input:  Bool que indique si la interrupción se habilita (1) o no (0).
 * Output: None.
 *****************************************************************************/
void UART_B_char_IE(bool interruption)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0, EUSCI_A_CTLW0_BRKIE_OFS) = interruption;
}

/*****************************************************************************
 * Function: UART_E_char_IE
 * Preconditions: None.
 * Overview: Pone en alto la interrupción por caracteres erróneos.
 * Input:  Bool que indique si la interrupción se habilita (1) o no (0).
 * Output: None.
 *****************************************************************************/
void UART_E_char_IE(bool interruption)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0, EUSCI_A_CTLW0_RXEIE_OFS) = interruption;
}

/*****************************************************************************
 * Function: UART_set_oversampling
 * Preconditions: None.
 * Overview: Establece si se debe contemplar sobremuestreo.
 * Input:  Bool que indique si se contempla sobremuestreo.
 * Output: None.
 *****************************************************************************/
void UART_set_oversampling(bool oversampling)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> MCTLW, EUSCI_A_MCTLW_OS16_OFS) = oversampling;
}


/****************************************************************************
 * Function: UART_set_parity
 * Preconditions: None.
 * Overview: Establece la paridad (ninguna, impar, par).
 * Input:  Bool que indique el estado deseado.
 * Output: None.
 *****************************************************************************/
void UART_set_parity(char parity)
{
    if(parity > 0)
    {
        BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0, UCPEN_OFS) = 1;
        BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0, UCPAR_OFS) = parity-1;
    }
    else
        BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0, UCPEN_OFS) = 0;
}


/****************************************************************************
 * Function: UART_set_stop_bits
 * Preconditions: None.
 * Overview: Establece el número de bits de paro en la comunicación.
 * Input:  Bool que indique el número de bits de paro,
 *         (1 bit si está en 0, 2 bits si está en 1).
 * Output: None.
 *****************************************************************************/
void UART_set_stop_bits(bool stop_bits)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0, UCSPB_OFS) = stop_bits;
}


/****************************************************************************
 * Function: UART_set_transmision_dir
 * Preconditions: None.
 * Overview: Establece si se debe contemplar sobremuestreo.
 * Input:  Bool que indique si se debe imprimir primero LSB (0) o MSB (1).
 * Output: None.
 *****************************************************************************/
void UART_set_transmision_dir(bool direction)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0, UCMSB_OFS) = direction;
}


/****************************************************************************
 * Function: UART_set_location_pin
 * Preconditions: None.
 * Overview: Establece los pines sobre los cuales se transmitirá y recibirá.
 * Input:  Puerto (int), seguido de los pines en una sola variable.
 * Output: None.
 *****************************************************************************/
void UART_set_location_pin(uint32_t selected_port, uint32_t selected_pins)
{
    if(selected_port %2 != 0)  // Si el puerto es impar.
    {
        HWREG16(GPIO_PORT_TO_BASE[selected_port] + OFS_PADIR) &= ~ (1 << (selected_pins+1));
        HWREG16(GPIO_PORT_TO_BASE[selected_port] + OFS_PASEL0) |=  (1 << (selected_pins+1));
        HWREG16(GPIO_PORT_TO_BASE[selected_port] + OFS_PASEL1) &= ~(1 << (selected_pins+1));
    }

    else
    {
        HWREG16((DIO_PORT_Odd_Interruptable_Type*)GPIO_PORT_TO_BASE[selected_port+1] + OFS_PADIR) &= ~ (1 << (selected_pins+1));
        HWREG16((DIO_PORT_Odd_Interruptable_Type*)GPIO_PORT_TO_BASE[selected_port+1] + OFS_PASEL0) |=  (1 << (selected_pins+1));
        HWREG16((DIO_PORT_Odd_Interruptable_Type*)GPIO_PORT_TO_BASE[selected_port+1] + OFS_PASEL1) &= ~(1 << (selected_pins+1));
    }
}


/***************************************************************************
 * Function: UART_set_baud_rate
 * Preconditions: None.
 * Overview: Establece un baud rate de las opciones disponibles.
 * Input:  Valor entero con las opciones diferentes (que están en un enum).
 *         Están contemplados para 12 MHz.
 * Output: None.
 *****************************************************************************/
void UART_set_baud_rate(uint8_t standard)
{
    if(standard < MAX_BAUD_RATE_CHANNELS)
    {
       EUSCI_A_CMSIS(EUSCI_A0)-> BRW = BRX[standard];
       EUSCI_A_CMSIS(EUSCI_A0)-> MCTLW = ((BRS[standard] << 8)  + (BRFX[standard] << 4) +  EUSCI_A_MCTLW_OS16);
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : print
* Returned Value   : None.
* Comments         :
*    Esta función ayuda a recordar que es necesario parar interrupciones al imprimir
*    una cadena, para un funcionamiento óptimo.
*
*END***********************************************************************************/

void print(char* message)
{
    Int_disable();
    printf("%s", message);  // Impresión de la cadena entrante.
    Int_enable();
}

// FUNCIONES ESPECIALES A REDEFINIR.
// Funciones redefinidas para poder usar printf.

int fputc(int _c, register FILE* _fp)
{
  while(!(UCA0IFG & UCTXIFG));
  UCA0TXBUF = (unsigned char) _c;

  return((unsigned char)_c);
}

int fputs(const char* _ptr, register FILE* _fp)
{
  unsigned int i, longitud;

  longitud = strlen(_ptr);

  for(i = 0; i < longitud; i++)
  {
      while(!(UCA0IFG & UCTXIFG));
      UCA0TXBUF = (unsigned char) _ptr[i];
  }

  return longitud;
}
