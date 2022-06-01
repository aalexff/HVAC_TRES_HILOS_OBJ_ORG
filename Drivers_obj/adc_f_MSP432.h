 //FileName:        adc_f_MSP432.h
 //Dependencies:    None.
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para ADC por medio de archivos. Header File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#ifndef ADC_F_MSP432_H_
#define ADC_F_MSP432_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DEFINICIONES BÁSICAS.

#define ADC_RESOLUTION_DEFAULT      ADC_14bitResolution
#define MAX_ADC_VALUE               16383               // 2 ^ 14 bits. Válido para resolución default.

#define ADC_CHANNEL_RUNNING         (0x01)              // Mientras esta bandera está activa, el canal está corriendo.
#define ADC_CHANNEL_RESUMED         (0x02)              // Mientras esta bandera está activa, el canal está activo y ha sido re-iniciado.

typedef uint_32 ADC_TRIGGER_MASK;                       // Formato de una máscara.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DISPONIBILIDAD DE RECURSOS DE HARDWARE Y SOFTWARE.

#define ADC_TRIGGER_1                  (1)          // 8 triggers son suficientes pero
#define ADC_TRIGGER_2                  (2)          // se pueden agregar más.
#define ADC_TRIGGER_3                  (3)
#define ADC_TRIGGER_4                  (4)
#define ADC_TRIGGER_5                  (5)
#define ADC_TRIGGER_6                  (6)
#define ADC_TRIGGER_7                  (7)
#define ADC_TRIGGER_8                  (8)

#define ADC_CHANNEL_0                  (0x00000001)
#define ADC_CHANNEL_1                  (0x00000002)
#define ADC_CHANNEL_2                  (0x00000004)
#define ADC_CHANNEL_3                  (0x00000008)
#define ADC_CHANNEL_4                  (0x00000010)
#define ADC_CHANNEL_5                  (0x00000020)
#define ADC_CHANNEL_6                  (0x00000040)
#define ADC_CHANNEL_7                  (0x00000080)
#define ADC_CHANNEL_8                  (0x00000100)
#define ADC_CHANNEL_9                  (0x00000200)
#define ADC_CHANNEL_10                 (0x00000400)
#define ADC_CHANNEL_11                 (0x00000800)
#define ADC_CHANNEL_12                 (0x00001000)
#define ADC_CHANNEL_13                 (0x00002000)
#define ADC_CHANNEL_14                 (0x00004000)
#define ADC_CHANNEL_15                 (0x00008000)

#define ADC_CHANNEL_16                 (0x00010000)
#define ADC_CHANNEL_17                 (0x00020000)
#define ADC_CHANNEL_18                 (0x00040000)
#define ADC_CHANNEL_19                 (0x00080000)
#define ADC_CHANNEL_20                 (0x00100000)
#define ADC_CHANNEL_21                 (0x00200000)
#define ADC_CHANNEL_22                 (0x00400000)
#define ADC_CHANNEL_23                 (0x00800000)
#define ADC_CHANNEL_24                 (0x01000000)
#define ADC_CHANNEL_25                 (0x02000000)
#define ADC_CHANNEL_26                 (0x04000000)
#define ADC_CHANNEL_27                 (0x08000000)
#define ADC_CHANNEL_28                 (0x10000000)
#define ADC_CHANNEL_29                 (0x20000000)
#define ADC_CHANNEL_30                 (0x40000000)
#define ADC_CHANNEL_31                 (0x80000000)
#define ADC_MAX_CHANNELS               31

enum Analog_pin
{
    AN0,  AN1,  AN2,  AN3,  AN4,  AN5,  AN6,  AN7,
    AN8,  AN9,  AN10, AN11, AN12, AN13, AN14, AN15,
    AN16, AN17, AN18, AN19, AN20, AN21, AN22, AN23,
    AN_MAX=23,
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CONFIGURACIÓN DEL MÓDULO ADC.

// Re-definiciones para resoluciones posibles.
#define ADC_8bitResolution              ADC14_CTL1_RES__8BIT
#define ADC_10bitResolution             ADC14_CTL1_RES__10BIT
#define ADC_12bitResolution             ADC14_CTL1_RES__12BIT
#define ADC_14bitResolution             ADC14_CTL1_RES__14BIT

// Re-definiciones para voltajes de referencia. El driver está diseñado solo para
// la primera opción, y la segunda en caso de usarse el sensor de temperatura.
#define ADC_VCC_VSS                     ADC14_MCTLN_VRSEL_0
#define ADC_VREF_VSS                    ADC14_MCTLN_VRSEL_1
#define ADC_VeREF_VeREF                 ADC14_MCTLN_VRSEL_14
#define ADC_VeREFBuf_VeREF              ADC14_MCTLN_VRSEL_15

// Re-definiciones para modos de conversión.
#define ADC_SingleChannel               ADC14_CTL0_CONSEQ_0
#define ADC_SequenceOfChannels          ADC14_CTL0_CONSEQ_1
#define ADC_SingleChannelRepeat         ADC14_CTL0_CONSEQ_2
#define ADC_SequenceOfChannelsRepeat    ADC14_CTL0_CONSEQ_3

// Definición de divisiones de tiempo.
#define ADC_PreDiv1             ADC14_CTL0_PDIV__1
#define ADC_PreDiv4             ADC14_CTL0_PDIV__4
#define ADC_PreDiv32            ADC14_CTL0_PDIV__32
#define ADC_PreDiv64            ADC14_CTL0_PDIV__64

#define ADC_CLKDiv1             ADC14_CTL0_DIV__1
#define ADC_CLKDiv2             ADC14_CTL0_DIV__2
#define ADC_CLKDiv3             ADC14_CTL0_DIV__3
#define ADC_CLKDiv4             ADC14_CTL0_DIV__4
#define ADC_CLKDiv5             ADC14_CTL0_DIV__5
#define ADC_CLKDiv6             ADC14_CTL0_DIV__6
#define ADC_CLKDiv7             ADC14_CTL0_DIV__7
#define ADC_CLKDiv8             ADC14_CTL0_DIV__8

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// APLICACIONES DE LAS FUNCIONES DE INICIO Y DE IOCTL.

// Banderas de inicialización.
#define ADC_CHANNEL_MEASURE_LOOP       (0x00) // La medición del canal se hace siempre, bajo un esquema de tiempo (usa módulo timer32_1).
#define ADC_CHANNEL_START_NOW          (0x01) // La medición se empieza a correr (se inicia el timer) desde su inicialización.

#define ADC_CHANNEL_START_TRIGGERED    (0x02) // Realiza una medición desde el inicio.
#define ADC_CHANNEL_MEASURE_ONCE       (0x04) // La medición requiere de un trigger manual.

#define ADC_INTERNAL_TEMPERATURE       (0x08)

// Simboliza el 'no uso' de ningún periodo (ADC se activa manualmente);
// Se pensó para ponerlo cuando el trigger es manual.
#define NONE                            0

// Comandos IOCTL diseñados.
#define IOCTL_ADC_RUN_CHANNEL           (0x10000000)
#define IOCTL_ADC_FIRE_TRIGGER          (0x10000001)
#define IOCTL_ADC_STOP_CHANNEL          (0x10000002)
#define IOCTL_ADC_STOP_CHANNELS         (0x10000003)
#define IOCTL_ADC_PAUSE_CHANNEL         (0x10000004)
#define IOCTL_ADC_PAUSE_CHANNELS        (0x10000005)
#define IOCTL_ADC_RESUME_CHANNEL        (0x10000006)
#define IOCTL_ADC_RESUME_CHANNELS       (0x10000007)
#define IOCTL_ADC_READ_TEMPERATURE      (0x10000008)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CONSTANTES ESPECIALES (SU ENTENDIMIENTO NO ES NECESARIO PARA PODER USAR EL PROGRAMA).

#define ADC_OVERFLOW_ELIMINATED     0x00000010
#define CTL1_START_ADDRESS             (16)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RECURSOS EXTERNOS AL ADC: Sensor de temperatura interno.

#define TEMPERATURE_ANALOG_PIN (AN22)

/* Estructura de inicializaciones. */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct adc_init_struct
{
   uint_32   resolution;
   uint_32   clock_def;

} ADC_INIT_STRUCT, _PTR_ ADC_INIT_STRUCT_PTR;

typedef struct adc_init_channel_struct
{
   uint_16   source;           // Se introduce cualquiera de los valores de las enumeración definida 'ANx'.
   uint_16   flags;            // Donde se introducen las banderas de inicialización.
   uint_32   time_period;      // Si se incluye la bandera '...MEASURE_LOOP', este será el tiempo en uS que tarda en cada lectura.
   ADC_TRIGGER_MASK trigger;   // Máscara que se puede asociar a más canales para hacer la lectura al mismo tiempo usándola.

} ADC_INIT_CHANNEL_STRUCT, _PTR_ ADC_INIT_CHANNEL_STRUCT_PTR;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ESTRUCTURAS ADC Y ADC_CH GENÉRICAS.

typedef struct adc_generic
{
   uint_16            resolution;
   _mqx_int           run;                  // Especifica si el ADC está corriendo.

} ADC_GENERIC, _PTR_ ADC_GENERIC_PTR;

typedef struct adc_channel_generic
{
   _mqx_uint             number;            // Número de canal.
   uint_32               source;            // Fuente de lectura, es decir, 'ANx'.
   uint_32               init_flags;        // Banderas de inicialización del canal.
   uint_32               runtime_flags;     // Banderas de la acción en tiempo del canal.
   _mqx_uint             period;            // Si hay un 'measure loop' como bandera, este tiempo se usa para el canal entre lecturas.
   ADC_TRIGGER_MASK      trigger;           // Máscara de trigger que puede activar alternativamente este canal.

} ADC_CHANNEL_GENERIC, _PTR_ ADC_CHANNEL_GENERIC_PTR;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ESTRUCTURAS ADC Y ADC_CH ESPECIFICAS.

typedef struct adc                                      // Estructura general del dispositivo ADC.
{
   ADC_GENERIC           g;                             // Se incluye una versión genérica del ADC.
   uint_32               results[ADC_MAX_CHANNELS];     // Arreglo de resultados de todos los canales.
} ADC, _PTR_ ADC_PTR;

typedef struct adc_channel
{
   ADC_CHANNEL_GENERIC   g;                             // ADC_genérico sin cambios; pensado para añadir propiedades
} ADC_CHANNEL, _PTR_ ADC_CHANNEL_PTR;                   // (más miembros) dependiendo del hardware.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Funciones básicas para el dispositivo. */

// Controles básicos del dispositivo.
extern _mqx_int adc_open                (FILE_PTR_f, char_ptr, char_ptr);
extern _mqx_int adc_close               (FILE _PTR_);
extern _mqx_int adc_read                (FILE_PTR_f, char_ptr, _mqx_int);
extern _mqx_int adc_ioctl               (FILE_PTR_f, _mqx_uint, pointer);

// Interrupciones del ADC y del timer32_1.
extern void     ADC14_IRQHandler        (void);
extern void     Timer32_Handler         (void);

/* Funciones específicas. */

// Para inicializar el ADC.
extern _mqx_int adc_hw_init             (uint_32 RES, uint_32 CLK_div);
// Para inicializar un canal del ADC.
extern _mqx_int adc_hw_channel_init     (_mqx_uint nr);
// Para disparar un canal; se entra a la función cuando es un trigger manual.
extern _mqx_int adc_trigger             (ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask);
// Para pausar un canal o varios.
extern _mqx_int adc_pause               (ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask);
// Para continuar el temporizador de un canal o varios.
extern _mqx_int adc_resume              (ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask);
// Para parar un canal del ADC.
extern _mqx_int adc_stop                (ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask);
// Devuelve un valor del ADC con previa conversión a temperatura en grados Celsius.
extern _mqx_int adc_temperature         (ADC_CHANNEL_GENERIC_PTR channel, pointer var);
// Obtiene el tiempo actual del módulo timer32_1 que temporiza a los canales.
extern _mqx_int adc_hw_get_time         (void);
// Devuelve TRUE si el ADC está realizando una conversión.
extern  boolean adc_is_busy             (void);

#endif
