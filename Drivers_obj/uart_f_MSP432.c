 //FileName:        uart_f_MSP432.c
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para UART por medio de archivos. Source File.
 //Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"

/* Estructura general del dispositivo. */
UART_DEVICE_STRUCT_PTR uart = NULL;
boolean RX_interruption = FALSE;

const uint32_t BRX[]  = {78, 19, 6};                    // Constantes para la determinaci�n del Baud Rate.
const uint32_t BRFX[] = {2,  8,  8};                    // (Technical Reference Manual - Pag 918).
const uint32_t BRS[]  = {0x0000, 0x0065, 0x0020};       // Definido para 12 MHz (BRCLK). CAMBIE SEGUN DISE�O.

/* Definici�n de los puertos. */
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
* Comments         : Reservaci�n de memoria, llenado de datos,
*                    llama funci�n de configuraci�n de HW.
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

   /* Llamada a la funci�n que configura. */
   result = uart_hw_init(uart_init_ptr, open_name_ptr);

   return(result);
}


/*FUNCTION****************************************************************
*
* Function Name    : uart_hw_init
* Returned Value   : IO_OK or IO_ERR
* Comments         : Configuraci�n de HW.
*
*END*********************************************************************/

uint_32 uart_hw_init (UART_INIT_STRUCT_PTR init_ptr, char _PTR_ open_name_ptr)
{
    /* Se apaga el m�dulo. */
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 , EUSCI_A_CTLW0_SWRST_OFS) = 1;

    /* Forma gen�rica para limpiar interrupciones, sleep, y otros registros inicialmente. */
    EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0 = (EUSCI_A_CMSIS(EUSCI_A0)-> CTLW0 & ~( UCRXEIE | UCBRKIE | UCDORM | UCTXADDR | UCTXBRK)) | EUSCI_A_CTLW0_MODE_0;

    UART_mode(ASYNCHRONOUS);                                             /* M�do As�ncrono habilitado (UART). */
    UART_set_location_pin(init_ptr->selected_port, init_ptr->pins[0]);   /* Selecci�n de pines. */
    UART_set_location_pin(init_ptr->selected_port, init_ptr->pins[1]);   /* Selecci�n de pines. */
    UART_clck_source(init_ptr -> clk);                                   /* Selecci�n de la fuente de reloj. (Bit SMCLK) */
    UART_set_transmision_dir(init_ptr -> direction);                     /* Selecci�n de MSB o LSB en orden de transmisi�n (0 para LSB, 1 para MSB). */
    UART_set_stop_bits(init_ptr -> stop_bits);                           /* N�mero de bits de paro; UCSPB = 0(1 stop bit) OR 1(2 stop bits). */
    UART_set_parity(init_ptr -> parity);                                 /* Paridad entre la comunicaci�n. */

    UART_set_baud_rate((uint8_t) init_ptr -> baud_rate);                 /* Configuraciones posibles: 9600, 38400, 115200. */
    UART_data_bits(init_ptr -> data_bits);                               /* Selecci�n de caracter 8 bits. */
    UART_set_oversampling(init_ptr -> oversampling);                     /* Sobremuestreo (Oversampling) */

    UART_E_char_IE(init_ptr -> interruption_E);                          /* Interrupci�n habilitada por caracteres err�neos. */
    UART_B_char_IE(init_ptr -> interruption_B);                          /* Interrupci�n habilitada por caracteres break.    */

    /* Se enciende el m�dulo. */
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
* Comments         : No hay ninguna funci�n definida para lectura, pero debe existir.
*
*END*********************************************************************************/

_mqx_int uart_read (FILE_PTR_f fd_ptr, char _PTR_ data_ptr, _mqx_int num)
{
   // No hay ninguna definici�n de esta funci�n.
   return IO_OK;
}


/*FUNCTION*******************************************************************
* Function Name    : uart_ioctl
* Returned Value   : IO_OK
* Comments         : En la revisi�n solo se tiene un comando para habilitar
*                    la interrupci�n ante la recepci�n de un caracter.
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
// Funciones de configuraci�n. //
/////////////////////////////////

/*FUNCTION*********************************************************************
*
* Function Name    : UART_set_location_pins
* Returned Value   : IO_OK or IO_ERR
* Comments         : Configuraci�n de pin. Funciona bien con los pines default.
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
 * Overview: Establece el env�o para 7 u 8 bits.
 * Input:  N�mero de bits (0 - 8 bits, 1 - 7 bits).
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
 * Input:  Enum con la selecci�n del reloj.
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
 * Input:  Bool que indique modo s�ncrono (1) o as�ncrono (0).
 * Output: None.
 *****************************************************************************/
void UART_mode(bool synchronization)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0, EUSCI_A_CTLW0_SYNC) = synchronization;
}


/*****************************************************************************
 * Function: UART_B_char_IE
 * Preconditions: None.
 * Overview: Pone en alto la interrupci�n por car�cteres 'break'.
 * Input:  Bool que indique si la interrupci�n se habilita (1) o no (0).
 * Output: None.
 *****************************************************************************/
void UART_B_char_IE(bool interruption)
{
    BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0) -> CTLW0, EUSCI_A_CTLW0_BRKIE_OFS) = interruption;
}

/*****************************************************************************
 * Function: UART_E_char_IE
 * Preconditions: None.
 * Overview: Pone en alto la interrupci�n por caracteres err�neos.
 * Input:  Bool que indique si la interrupci�n se habilita (1) o no (0).
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
 * Overview: Establece el n�mero de bits de paro en la comunicaci�n.
 * Input:  Bool que indique el n�mero de bits de paro,
 *         (1 bit si est� en 0, 2 bits si est� en 1).
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
 * Overview: Establece los pines sobre los cuales se transmitir� y recibir�.
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
 * Input:  Valor entero con las opciones diferentes (que est�n en un enum).
 *         Est�n contemplados para 12 MHz.
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
*    Esta funci�n ayuda a recordar que es necesario parar interrupciones al imprimir
*    una cadena, para un funcionamiento �ptimo.
*
*END***********************************************************************************/

void print(char* message)
{
    Int_disable();
    printf("%s", message);  // Impresi�n de la cadena entrante.
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
