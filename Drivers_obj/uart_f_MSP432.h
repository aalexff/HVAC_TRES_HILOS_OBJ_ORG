 //FileName:        uart_f_MSP432.h
 //Dependencies:    None.
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para UART por medio de archivos. Header File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#ifndef UART_F_MSP432_H_
#define UART_F_MSP432_H_

// DEFINICIÓN DE CONSTANTES ESPECIFICAS.

/* Uso de pines UART 1.1 y 1.2. */
#define PINES_UART_DEFAULT ( 0x0001 << 2 | 0x0001 << 3)
#define PORT_UART_DEFAULT  ( 0x40004C00 )

/* Macros para la re-definición de funciones del sistema. */
#define EUSCI_A_CMSIS(x) ((EUSCI_A_Type *) x)
#define EUSCI_B_CMSIS(x) ((EUSCI_B_Type *) x)

/* Fuente de reloj. */
#define UCLK    0
#define ACLK    1
#define SMCLK   2

/* Modo de comunicación. */
#define EIGHT_BITS    0
#define SEVEN_BITS    1

/* Dirección de transmisión. */
#define LSB_FIRST 0
#define MSB_FIRST 1

/* Numero de bits de paro. */
#define ONE_STOP_BIT 0
#define TWO_STOP_BIT 1

/* Oversampling. */
#define NO_OVERSAMPLE 0
#define OVERSAMPLE   1

/* Activa o no interrupciones. */
#define NO_INTERRUPTION 0
#define INTERRUPTION    1

/* Modo de comunicación. */
#define ASYNCHRONOUS     0
#define SYNCHRONOUS      1

/* Modo de comunicación. */
#define EIGHT_BITS    0
#define SEVEN_BITS    1

/* Numero de bits de paro. */
#define ONE_STOP_BIT 0
#define TWO_STOP_BIT 1

/* Oversampling. */
#define NO_OVERAMPLE 0
#define OVERSAMPLE   1

/* Activa o no interrupciones. */
#define NO_INTERRUPTION 0
#define INTERRUPTION    1

/* Modo de comunicación. */
#define ASYNCHRONOUS     0
#define SYNCHRONOUS      1

/* Baud Rate's en el arreglo. */
#define MAX_BAUD_RATE_CHANNELS 3

/* Comando(s) IOCTL. */
#define IO_IOCTL_SERIAL_IRQ_FUNCTION     0x20000001

/* Definción predeterminada. */
#define MAIN_UART                   (uint32_t)(EUSCI_A0)

 // Enum que relaciona la fuente de reloj como opciones.
typedef enum
{
   U_CLK = 0,
   A_CLK,
   SM_CLK
}  Clk_source;

// Enum que relaciona 3 opciones distinas de baud rate.
typedef enum
{
   BR_9600 = 0,
   BR_38400,
   BR_115200
}  Baud_Rate;

// Enum que relaciona tipos de paridad.
typedef enum
{
   NO_PARITY = 0,
   ODD,
   EVEN
} Parity;

/*
 *  Estructura uart_init_struct que contendrá todas las definiciones
 *  de la configuración de la comunicación serial (inicialización).
 */

typedef struct uart_init_struct
{
  _mqx_int selected_port;
  _mqx_int pins[2];
  Clk_source clk;
  Baud_Rate baud_rate;

  _mqx_int parity;
  boolean  data_bits;
  boolean  oversampling;
  boolean  stop_bits;
  boolean  direction;

  boolean  interruption_B;
  boolean  interruption_E;

} UART_INIT_STRUCT, _PTR_ UART_INIT_STRUCT_PTR;

/*
 *  Estructura uart_init_struct que contendrá lo más importante
 *  en la configuración del dispositivo.
 */

typedef struct uart_struct
{
  _mqx_int selected_port;
  _mqx_int pins[2];
  Clk_source clk;
  _mqx_int baud_rate;

} UART_DEVICE_STRUCT, _PTR_ UART_DEVICE_STRUCT_PTR;

// FUNCIONES PRINCIPALES.

extern _mqx_int uart_open  (FILE_PTR_f, char_ptr, char_ptr);
extern _mqx_int uart_close (FILE _PTR_);
extern _mqx_int uart_read  (FILE_PTR_f, char_ptr, _mqx_int);
extern _mqx_int uart_ioctl (FILE_PTR_f, _mqx_uint, pointer);


// FUNCION(ES) DE CONFIGURACIÓN.
extern uint_32 uart_hw_init (UART_INIT_STRUCT_PTR init_ptr, char _PTR_ open_name_ptr);

// FUNCIONES ESPECÍFICAS.

/* Establece el número de bits de datos. */
extern void UART_data_bits(bool data_bits);
/* Establece de donde se toma el reloj del puerto. */
extern void UART_clck_source(Clk_source source);
/* Establece de donde se toma el reloj del puerto. */
extern void UART_mode(bool synchronization);
/* Pone en alto la interrupción por carácteres 'break'. */
extern void UART_B_char_IE(bool interruption);
/* Pone en alto la interrupción por carácteres erróneos. */
extern void UART_E_char_IE(bool interruption);
/* Establece si se debe contemplar sobremuestreo. */
extern void UART_set_oversampling(bool oversampling);
/* Establece si hay paridad o no. */
extern void UART_set_parity(char parity);
/* Establece el número de bits de paro en la comunicación. */
extern void UART_set_stop_bits(bool stop_bits);
/* Establece la dirección de la transmisión (puede ser primero MSB, o LSB) */
extern void UART_set_transmision_dir(bool direction);
/* Establece los pines sobre los cuales se transmitirá y recibirá el UART. */
extern void UART_set_location_pin(uint32_t selected_port,uint32_t selected_pins);
/* Establece un baud rate de las opciones disponibles. */
extern void UART_set_baud_rate(uint8_t standard);
/* Forma óptima de imprimir apagando interrupciones. */
extern void print(char* message);

// Hay que redefinir estas funciones.
int fputc(int _c, register FILE* _fp);
int fputs(const char* _ptr, register FILE* _fp);


// NOTA IMPORTANTE: EVITE (DESHABILITE) INTERRUPCIONES AL USAR DIRECTAMENTE "PRINTF".

#endif
