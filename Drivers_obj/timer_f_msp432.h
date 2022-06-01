 //FileName:        timer_f_msp432.h
 //Dependencies:    None
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Descripción general de constantes, macros y configuraciones del módulo TIMER32. Header File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018


#ifndef TIMER_F_MSP432_H_
#define TIMER_F_MSP432_H_

// Definiciones de indexes para los arreglos que albergan el tiempo.
#define SECONDS                 2
#define MINUTES                 1
#define HOURS                   0

// Definiciones del uso del timer.
#define ADC_T                   0
#define SOLO_TIMER              1

// Definiciones de límites de recursos.
#define MAX_TIMER_UNITS         8
#define MINIMUM_LIMIT_STEP      999
#define STR_TIMER_LENGTH        12
#define DEFAULT_STEP            1000

// Definiciones de estados.
#define RUN                     0
#define PAUSED                  1
#define STOPPED                 2
#define RESUMED                 3

// Definiciones de configuraciones para estructura unit_init.
#define MAX_AND_STOPPED         0x00000001
#define PERIOD_FLAG             0x00000002
#define PERIOD_TOGGLE           0x00000004
#define PERIOD_END              0x00000008
#define ALL                     MAX_AND_STOPPED | PERIOD_FLAG | PERIOD_TOGGLE | PERIOD_END

// Definiciones de banderas en formato numérico simple.
#define M_STOPPED               0
#define P_FLAG                  1
#define P_TOGGLE                2
#define P_END                   3
#define T_MILLIS                4
#define T_SECONDS               5
#define T_MINUTES               6
#define T_HOURS                 7
#define T_STRING                8

#define MAX_TIMER_FLAGS         4

// Posibles instrucciones del timer/cronómetro en IOCTL.
#define IOCTL_TIMER_GENERAL   0x7000FFFF
#define IOCTL_TIMER_RUN       0x70000000
#define IOCTL_TIMER_STOP      0x70000001
#define IOCTL_TIMER_RESUME    0x70000002
#define IOCTL_TIMER_PAUSE     0x70000003

// Estructura de inicialización de timer principal:
// Contiene el estado actual del timer general, así como el step de resta sucesiva.
typedef struct _timer_struct
{
   uint_32   run_now;
   uint_32   step;

} TIMER_INIT_STRUCT, _PTR_ TIMER_INIT_STRUCT_PTR;

// Estructura de inicialización de una unidad: Necesita el estado inicial, el periodo de esta unidad,
// el límite máximo de tiempo, así como las banderas de inicialización.
typedef struct _timer_init_struct
{
   uint_32   state;                                     // Estado inicial de la unidad.
   uint_32   time_period;                               // Periodo de actualización.
   uint_32   max[3];                                    // Máximo tiempo de cronometraje; puede o no reiniciar el tiempo.

   uint_32   flags;                                     // Las banderas pueden incluir: tener bandera cada periodo,
                                                        // un toggle, y/o una bandera de finalización, asi como decidir
                                                        // si tras cumplirse el tiempo máximo se debe reiniciar.

} TIMER_UNIT_INIT_STRUCT, _PTR_ TIMER_UNIT_INIT_STRUCT_PTR;

// Estructura principal de timer: Contiene el estado general y el step, así como todos los tiempos
// de todas las unidades 'vivas' contando el tiempo en segundos, y los periodos de reinicio y resta de tiempo.
typedef struct _timer_file
{
   uint_32               state_gral;                   // El estado puede ser run, paused o stopped.
   uint_32               step;                         // Paso sobre el cual se van realizando las restas sucesivas.

   uint_32               time_full[MAX_TIMER_UNITS];   // Arreglo de resultados de todos los canales.
   uint_32               time_left[MAX_TIMER_UNITS];   // Tiempo restante para que se cumpla el periodo.

   uint_32               time[3][MAX_TIMER_UNITS];     // Tiempo actual de cada unidad.

} TIMER, _PTR_ TIMER_PTR;

// Estructura de cada unidad: alberga principalmente la configuración, como el estado, periodo y tiempo máximo
// respectivo a esta y solo esta unidad, sino también las configuraciones y banderas y toggles que se activan.
typedef struct _timer_unit_file
{
   uint_32               num;                           // Número de la unidad; proporciona fácil identificación.
   uint_32               state;                         // Estado puede ser run, paused o stopped.
   uint_32               scale;                         // Equivalente a time_period.

   uint_32               max[3];                        // Valor máximo a la cual llega la unidad de cronómetro.

   boolean               period_flag;                   // Bandera que anuncia que se ha cumplido un periodo.
   boolean               period_toggle;                 // Toggle en base a periodo.
   boolean               period_end;                    // Bandera que anuncia que se ha cumplido el tiempo máximo.

   boolean               flags[MAX_TIMER_FLAGS];        // Banderas respectivas de la unidad cronómetro.

} TIMER_UNIT_DATA, _PTR_ TIMER_UNIT_DATA_PTR;

// Función para limpiar (poner en cero's) en un inicio los valores de la estructura.
extern  void clean_timer (void);
// Función para inicializar en HW el timer.
extern  _mqx_int timer_hw_init (void);

// Funciones para abrir, cerrar, controlar y leer los objetos del timer.
extern _mqx_int timer_open       (FILE_PTR_f fd_ptr, char_ptr open_name_ptr, char_ptr flags);
extern _mqx_int timer_close      (FILE _PTR_ fd_ptr);
extern _mqx_int timer_read       (FILE_PTR_f fd_ptr, char_ptr data_ptr, _mqx_int num);
extern _mqx_int timer_ioctl      (FILE_PTR_f fd_ptr, _mqx_uint cmd, pointer param_ptr);

// Interrupción. Para timer32_2 (módulo 2).
extern void Timer_Handler(void);

#endif /* TIMER_F_MSP432_H_ */
