 //FileName:        adc_f_MSP432.c
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Driver para ADC por medio de archivos. Source File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"

/* Configuración del driver, canal(es) son independientes al módulo. */
ADC_PTR adc = NULL;
/* Configuraciones dependientes del canal. */
ADC_CHANNEL_PTR adc_ch[ADC_MAX_CHANNELS] = { 0 };

/* Mapeo global de canales con interrupción. */
uint_32 ADC_global_irq_map = 0;

/* Indicadores. */
extern boolean timer_activated[2];
uint_8  ADC_ch_actives               = 0;
boolean ADC_timer_activation    [32] = { 0 };

/* Valor de tiempo asignado a cada trigger de cada canal. */
uint_32 ADC_time_channel        [32] = { 0 };
uint_32 ADC_time_channel_temp   [32] = { 0 };

/* Variables sincrónicas. */
_mqx_uint  time_stopped[32] = { 0 };
_mqx_uint  current_addr = 0;
_mqx_uint  microseconds = 0;

/* Variables de máscara. */
extern _mqx_int temp;
static ADC_TRIGGER_MASK running_mask[16] = { 0 };


/*FUNCTION******************************************************************************
*
* Function Name    : Timer32_Handler
* Returned Value   : None
* Comments         :
*    Valoriza si la cuenta regresiva de los canales termina, la renueva y dispara el adc.
*
*END***********************************************************************************/

void Timer32_Handler(void)
{
    _mqx_int i;

    Int_disable();

    TIMER32_1 -> INTCLR = 0;                                    // Borra bandera de timer32.

    microseconds += STEP;                                       // Aumenta el tiempo tomado.
    if (microseconds >= TIME_RESET)
        microseconds = 0;

    for(i = 0; i <= ADC_MAX_CHANNELS; i++)
        if(ADC_time_channel_temp[i] != 0)
        {
            ADC_time_channel_temp[i] -= STEP;                       // Disminuye la cuenta regresiva de cada canal.
            if(ADC_time_channel_temp[i]/STEP == 0)                  // Al acabar esta cuenta:
            {
                adc->g.run = 1;
                BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;  // Se apaga módulo para reconfigurarlo.
                ADC_timer_activation[i] = TRUE;
                ADC_time_channel_temp[i] = ADC_time_channel[i];     // Se renueva el contador de tiempo al canal.

                ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);    // Se reasigna la dirección que el ADC debe tomar.
                ADC14 -> CTL1 |=  i << CTL1_START_ADDRESS;
                current_addr = i;                                   // Variable sincrónica.

                BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;  // Se enciende el módulo de nuevo.
                BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SC_OFS) =  1;  // Se dispara.
            }
        }

    Int_enable();

   return;
}

/*FUNCTION*****************************************************************************************
*
* Function Name    : ADC14_IRQHandler
* Returned Value   : None
* Comments         :
*    Llena el valor recogido de la conversión en la estructura global en el momento que interrumpe.
*
*END**********************************************************************************************/

void ADC14_IRQHandler(void)
{
    uint_32 flags;
    uint_32 i;

    Int_disable();

    flags = ADC14 -> IFGR0;                         // Limpia bandera de interrupción.
    for(i = 0; i <= ADC_MAX_CHANNELS; i++)          // Averigua canal que provocó la cadena.
        if((1 << i) & flags)
            break;

    ADC14 -> CLRIFGR0 |= (1 << i);
    ADC14 -> CLRIFGR1 |= ADC_OVERFLOW_ELIMINATED;
    adc ->   results[i]= ADC14 -> MEM[i];           // Llena la estructura con el dato resultante.

    Int_enable();

    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : adc_open
* Returned Value   : IO_OK or IO_ERR
* Comments         :
*    Genera los archivos y modificaciones correspondientes en HW para adc y canales.
*
*END***********************************************************************************/

_mqx_int adc_open  (FILE_PTR_f fd_ptr, char_ptr open_name_ptr, char_ptr flags)
{
    char_ptr file_name_ptr = fd_ptr->DEV_PTR->IDENTIFIER;
    _mqx_int status, temp;

    while (*file_name_ptr++ != 0)
       open_name_ptr++;                                                 // Mueve al nombre del archivo.
    file_name_ptr = open_name_ptr;

    // Preparando módulo.
    // Se recibió: "adc:"

    if (*file_name_ptr == 0)
    {
       ADC_INIT_STRUCT_PTR init_from = (ADC_INIT_STRUCT_PTR) flags;

       if (init_from == NULL)
          return IO_ERR;                                                // No hay propiedades.

       if (NULL == adc)
       {
          adc = (ADC_PTR) malloc (sizeof(ADC));                         // Reservación de memoria de la estructura general.
          if (adc == NULL)
             return IO_ERR;
       }
       else
          return IO_ERR;

       for(temp = 0; temp < ADC_MAX_CHANNELS; temp++)
           adc -> results[temp] = 0;

       adc-> g.resolution    = init_from->resolution;                                           // Llenado de la estructura.
       adc-> g.run           = 0;
       if (IO_OK != (status = adc_hw_init(init_from -> resolution, init_from -> clock_def)))    // Cambios en HW.
       {
          free(adc);
          adc = NULL;
       }
       return status;
    }

    // Selección del canal.
    // Se recibió: "adc:ch"

    else
    {
       // Si se trata de un canal, el canal se encuentra dentro de la cadena recibida.
       ADC_INIT_CHANNEL_STRUCT_PTR init_from = (ADC_INIT_CHANNEL_STRUCT_PTR) flags;
       _mqx_uint                   ch = 0;
       _mqx_uint                   radix = 1;

       // Precauciones.
       if (init_from == NULL)
          return IO_ERR;
       if (NULL == adc)
          return IO_ERR;

       while ((*open_name_ptr >= '0') && (*open_name_ptr <= '9'))           // Encuentra dígito de canal en cadena.
          open_name_ptr++;

       if (*open_name_ptr != 0)
          return IO_ERR;

       open_name_ptr--;                                                     // Mueve al último dígito de la cadena.

       do
       {
          if (ADC_MAX_CHANNELS <= (ch += radix * (*open_name_ptr - '0')))
             return IO_ERR;                                                 // Canal excedido.
          radix *= 10;
       }
       while (open_name_ptr-- != file_name_ptr);


       if (adc_ch[ch] == NULL)
       {
          adc_ch[ch] = (ADC_CHANNEL_PTR) malloc (sizeof(ADC_CHANNEL));
          if (adc_ch[ch] == NULL)
             return IO_ERR;
       }
       else
          return IO_ERR;                                                    // Canal ya usado por un archivo.

       // Llenado de estructura.
       adc_ch[ch]-> g.number = ch;
       adc_ch[ch]-> g.source = init_from->source;
       adc_ch[ch]-> g.init_flags = init_from->flags;
       adc_ch[ch]-> g.runtime_flags = init_from -> flags;
       adc_ch[ch]-> g.trigger = init_from->trigger;
       adc_ch[ch]-> g.period = init_from->time_period;

       ADC_ch_actives++;

       running_mask[adc_ch[ch]-> g.trigger] |= 1 << ch;

       if (IO_OK != (status = adc_hw_channel_init(ch)))                     // Esto debería inicializar el HW.
       {
           free(adc_ch[ch]);
           adc_ch[ch] = NULL;
           return status;
       }

       // Almacena contexto del canal en archivo.
       fd_ptr->DEV_DATA_PTR = adc_ch[ch];

       // Si el canal se debe correr en la fase de apertura, se corre trigger especial.
       if (!(init_from->flags & ADC_CHANNEL_START_TRIGGERED))
       {
           BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
           BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SC_OFS) =  1;
       }
    }

    return IO_OK;
}

/*FUNCTION******************************************************************************
*
* Function Name    : adc_hw_init
* Returned Value   : IO_OK
* Comments         :
*    Realiza los cambios respectivos al HW del módulo (no canales).
*
*END***********************************************************************************/

_mqx_int adc_hw_init (uint_32 RES, uint_32 CLK_div)
{
    _mqx_int         i;

    ADC14 -> CTL1 = RES;                                                        // Definición de resolución.

    ADC14 -> CTL0 |= CLK_div | ADC14_CTL0_SHT1__64 | ADC14_CTL0_SHT0__192;      // Definición de la división de reloj.
    BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SHP_OFS) = 1;

    ADC14 -> CTL0 |=  ADC_SingleChannel;                                        // Modo de un solo canal con trigger 'manual'.
    ADC14-> CTL0 |= ADC14_CTL0_SHP;                                             // Se tiene que re-activar el trigger.

    for(i = 0; i < 32; i++)
        BITBAND_PERI(ADC14->MCTL[i], ADC14_MCTLN_EOS_OFS) = 1;                  // Todas las entradas se fijan únicas (no hay secuencia).
    BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ON_OFS) = 1;                           // Enciende el módulo ADC.

    Int_registerInterrupt(INT_ADC14, ADC14_IRQHandler);
    Int_enableInterrupt(INT_ADC14);                                             // Genera la interrupción.

    return IO_OK;
}

/*FUNCTION******************************************************************************
*
* Function Name    : adc_hw_init
* Returned Value   : IO_OK or IO_OK
* Comments         :
*    Realiza los cambios respectivos al HW de un canal.
*
*END***********************************************************************************/

_mqx_int adc_hw_channel_init (_mqx_uint nr)
{
    _mqx_uint i;
    static boolean bandera_interrupt_timer = 0;

    if (adc_ch[nr]->g.source > AN_MAX)
        return IO_ERR;

    for (i = 0; i < ADC_MAX_CHANNELS; i++)
        if ((i != nr) && (adc_ch[i] != NULL))
            if (adc_ch[i]->g.source == adc_ch[nr]->g.source)    // Canal ya usado en un archivo.
                return IO_ERR;

    // Configura GPIO entre otras cosas para el canal.
    BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;

    ADC14 -> IER0 |= 1 << nr;                                   // Establece interrupción en HW,
    ADC_global_irq_map |= 1 << nr;                              // y también la mapea globalmente.

    ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
    ADC14 -> CTL1 |=  nr << CTL1_START_ADDRESS;

    if(adc_ch[nr]->g.init_flags & (ADC_INTERNAL_TEMPERATURE))   // Si se trata del módulo de temperatura:
    {
       REF_A->CTL0 |= REF_A_CTL0_VSEL_3;
       BITBAND_PERI(REF_A->CTL0 , REF_A_CTL0_ON_OFS) = 1;
       BITBAND_PERI(REF_A->CTL0 , REF_A_CTL0_TCOFF_OFS) = 0;
       BITBAND_PERI(ADC14 -> CTL1, ADC14_CTL1_TCMAP_OFS) = 1;   // Habilita el sensor de temperatura.

       ADC14 -> MCTL[nr] = ADC_VREF_VSS | adc_ch[nr]->g.source;
    }

    else
        ADC14 -> MCTL[nr] = ADC_VCC_VSS | adc_ch[nr]->g.source;

    BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;          // Se enciende el módulo.

    switch(adc_ch[nr]->g.source)                                // De acuerdo al pin se activa la opción análogica.
    {
        case AN0: P5SEL1 |= BIT5; P5SEL0 |= BIT5; break;
        case AN1: P5SEL1 |= BIT4; P5SEL0 |= BIT4; break;
        case AN2: P5SEL1 |= BIT3; P5SEL0 |= BIT3; break;
        case AN3: P5SEL1 |= BIT2; P5SEL0 |= BIT2; break;
        case AN4: P5SEL1 |= BIT1; P5SEL0 |= BIT1; break;
        case AN5: P5SEL1 |= BIT0; P5SEL0 |= BIT0; break;

        case AN6: P4SEL1 |= BIT7; P4SEL0 |= BIT7; break;
        case AN7: P4SEL1 |= BIT6; P4SEL0 |= BIT6; break;
        case AN8: P4SEL1 |= BIT5; P4SEL0 |= BIT5; break;
        case AN9: P4SEL1 |= BIT4; P4SEL0 |= BIT4; break;
        case AN10:P4SEL1 |= BIT3; P4SEL0 |= BIT3; break;
        case AN11:P4SEL1 |= BIT2; P4SEL0 |= BIT2; break;
        case AN12:P4SEL1 |= BIT1; P4SEL0 |= BIT1; break;
        case AN13:P4SEL1 |= BIT0; P4SEL0 |= BIT0; break;

        case AN14:P6SEL1 |= BIT1; P6SEL0 |= BIT1; break;
        case AN15:P6SEL1 |= BIT0; P6SEL0 |= BIT0; break;

        case AN16:P9SEL1 |= BIT1; P9SEL0 |= BIT1; break;
        case AN17:P9SEL1 |= BIT0; P9SEL0 |= BIT0; break;

        case AN18:P8SEL1 |= BIT7; P8SEL0 |= BIT7; break;
        case AN19:P8SEL1 |= BIT6; P8SEL0 |= BIT6; break;
        case AN20:P8SEL1 |= BIT5; P8SEL0 |= BIT5; break;
        case AN21:P8SEL1 |= BIT4; P8SEL0 |= BIT4; break;
        case AN22:P8SEL1 |= BIT3; P8SEL0 |= BIT3; break;
        case AN23:P8SEL1 |= BIT2; P8SEL0 |= BIT2; break;
        default: break;
    }

    if(!bandera_interrupt_timer)                                // Esto se hace si no se ha activado la interrupción ya.
    {
        Int_registerInterrupt(INT_T32_INT1, Timer32_Handler);
        Int_enableInterrupt(INT_T32_INT1);
        bandera_interrupt_timer = 1;
    }


    if(adc_ch[nr]->g.init_flags & (ADC_CHANNEL_START_NOW))           // Si se pide que el canal se inicialice desde un inicio.
    {
        if(!(adc_ch[nr]->g.init_flags & (ADC_CHANNEL_MEASURE_ONCE))) // Por timer.
        {
            if(timer_activated[ADC_T] != TRUE)                                              // Timer no cargado.
            {

                TIMER32_1 -> CONTROL |= TIMER32_CONTROL_ENABLE;
                TIMER32_1 -> LOAD = ((__SYSTEM_CLOCK)/(SEC))*(STEP) - 1;                    // Valor a cargar.
                TIMER32_1 -> CONTROL = 0xC2;                                                // 32 bit, periódico, con base de tiempo..
                TIMER32_1 -> CONTROL |= TIMER32_CONTROL_PRESCALE_0;                         // No hay prescaler.
                TIMER32_1 -> CONTROL |= TIMER32_CONTROL_IE;                                 // Habilita interrupción.

                timer_activated[ADC_T] = TRUE;
            }

            else                                                                            // Si ya esta cargado.
            {
                TIMER32_1 -> CONTROL |= TIMER32_CONTROL_IE;                                 // Asegura interrupción.
                TIMER32_1 -> CONTROL |= TIMER32_CONTROL_ENABLE;                             // Asegura timer controlado.

                timer_activated[ADC_T] = TRUE;
            }

            ADC_time_channel        [nr] = adc_ch[nr]->g.period;                            // Establece la cuenta regresiva.
            ADC_time_channel_temp   [nr] = adc_ch[nr]->g.period;
        }
    }

    return IO_OK;
}

/*FUNCTION******************************************************************************************
*
* Function Name    : adc_ioctl
* Returned Value   : int_32
* Comments         :
*    Controla operaciones de pausa, continuación, paro, disparo, etc., de un canal o máscara de ellos.
*
*END***********************************************************************************************/

_mqx_int adc_ioctl (FILE_PTR_f fd_ptr, _mqx_uint cmd, pointer param_ptr)
{
    ADC_CHANNEL_GENERIC_PTR adc_ch;
    adc_ch = (ADC_CHANNEL_GENERIC_PTR) fd_ptr -> DEV_DATA_PTR;

    switch (cmd)
    {
        case IOCTL_ADC_RUN_CHANNEL:
            return adc_trigger(adc_ch, 0);                                 /* Correr un canal en específico. */

        case IOCTL_ADC_FIRE_TRIGGER:                                       /* Correr canales asociados a una máscara. */
            return adc_trigger((ADC_CHANNEL_GENERIC_PTR) NULL_POINTER, (ADC_TRIGGER_MASK) param_ptr);

        case IOCTL_ADC_STOP_CHANNEL:
            return adc_stop(adc_ch, 0);                                    /* Paro de canal específico. */

        case IOCTL_ADC_STOP_CHANNELS:                                      /* Paro canales asociados a una máscara. */
            return adc_stop((ADC_CHANNEL_GENERIC_PTR) NULL_POINTER, (ADC_TRIGGER_MASK) param_ptr);

        case IOCTL_ADC_PAUSE_CHANNEL:
            return adc_pause(adc_ch, 0);                                   /* Pausa canal específico. */

        case IOCTL_ADC_PAUSE_CHANNELS:                                     /* Pausa canales asociados a una máscara. */
            return adc_pause((ADC_CHANNEL_GENERIC_PTR) NULL_POINTER, (ADC_TRIGGER_MASK) param_ptr);

        case IOCTL_ADC_RESUME_CHANNEL:
            return adc_resume(adc_ch, 0);                                  /* Continúa canal específico. */

        case IOCTL_ADC_RESUME_CHANNELS:                                    /* Continua canales asociados a una máscara. */
            return adc_resume((ADC_CHANNEL_GENERIC_PTR) NULL_POINTER, (ADC_TRIGGER_MASK) param_ptr);

        case IOCTL_ADC_READ_TEMPERATURE:
            return adc_temperature(adc_ch, param_ptr);                     /* Obtiene valor de temperatura (c/ conversión). */

        default:
            break;
    }
    return IO_ERR;
}


/*FUNCTION*****************************************************************
*
* Function Name    : adc_trigger
* Returned Value   : IO_OK
* Comments         : Dispara un canal o una máscara de varios canales.
*
*END*********************************************************************/

_mqx_int adc_trigger(ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask)
{
    _mqx_int i, temp;
    _mqx_int canal_inicio =  0;
    _mqx_int canal_final =  0;

    temp = current_addr;

    if (channel)
    {
        // La activación, con el timer corriendo, consiste en llenar un valor a estos arreglos base 1000.
        ADC_time_channel        [channel -> number] = adc_ch[channel -> number]->g.period;
        ADC_time_channel_temp   [channel -> number] = adc_ch[channel -> number]->g.period;

        // Explicado arriba en adc_hw_init. Activación del timer.
        if(!timer_activated[ADC_T])
        {
            TIMER32_1 -> CONTROL |= TIMER32_CONTROL_ENABLE;
            TIMER32_1 -> LOAD = ((__SYSTEM_CLOCK)/(SEC))*(STEP) - 1;
            TIMER32_1 -> CONTROL = 0xC2;
            TIMER32_1 -> CONTROL |= TIMER32_CONTROL_PRESCALE_0;
            TIMER32_1 -> CONTROL |= TIMER32_CONTROL_IE;

            timer_activated[ADC_T] = 1;
        }
        // Llenado de estado.
        channel->runtime_flags |= ADC_CHANNEL_RUNNING | ADC_CHANNEL_RESUMED;
    }

    else
    {
        // Si no entró ningún canal a la función,
        // se desea correr todos los canales asociados a una máscara.
        for(i = 0; i < ADC_MAX_CHANNELS; i++)
        {
            if(running_mask[mask] & 1 << i)
            {
                if(canal_inicio == 0)
                    canal_inicio = i;

                if(canal_final < i)
                canal_final = i;
            }
        }

        if(canal_inicio == canal_final)
        {
            BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;

            if(timer_activated[ADC_T])
             {
                 // Desactivar timer.
                 TIMER32_1 -> CONTROL &= ~TIMER32_CONTROL_IE;
                 TIMER32_1 -> CONTROL &= ~TIMER32_CONTROL_ENABLE;

                 // Establece dirección temporal para disparar.
                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  canal_inicio << CTL1_START_ADDRESS;

                 // Disparo ya con la dirección asignada.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SC_OFS) =  1;

                 // Espera interrupción.
                 usleep(12);

                 // Regresa dirección actual en el timer.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;

                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  temp << CTL1_START_ADDRESS;

                 // Establece control del timer.
                 TIMER32_1 -> CONTROL |= TIMER32_CONTROL_ENABLE;
                 TIMER32_1 -> CONTROL |= TIMER32_CONTROL_IE;

                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;

                 // Llenado de estado.
                 for (i = 0; i < ADC_MAX_CHANNELS; i++)
                 {
                     if (adc_ch[i] && (adc_ch[i]->g.trigger & mask))
                         adc_ch[i]->g.runtime_flags |= ADC_CHANNEL_RUNNING | ADC_CHANNEL_RESUMED;
                 }
             }

             else
             {
                 // Establece dirección temporal para disparar.
                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  (canal_inicio) << CTL1_START_ADDRESS;

                 // Enciende y dispara.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SC_OFS)  = 1;

                 // Espera interrupción.
                 usleep(12);

                 // Regresa dirección actual.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;
                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  temp << CTL1_START_ADDRESS;
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
             }

        }

        // Hay más de un canal, se opta por modo de secuencia.
        else if(canal_final > canal_inicio)
        {
            BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;

            for(i = 0; i < 32; i++)
            {
                if(canal_final == i)
                    BITBAND_PERI(ADC14->MCTL[i], ADC14_MCTLN_EOS_OFS) = 1;
                else
                    BITBAND_PERI(ADC14->MCTL[i], ADC14_MCTLN_EOS_OFS) = 0;
            }

            ADC14 -> CTL0 &= ~ADC14_CTL0_CONSEQ_3;
            ADC14 -> CTL0 |= ADC_SequenceOfChannels;

            if(timer_activated[ADC_T])
             {
                 TIMER32_1 -> CONTROL &= ~TIMER32_CONTROL_IE;
                 TIMER32_1 -> CONTROL &= ~TIMER32_CONTROL_ENABLE;

                 // Dirección temporal.
                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  (canal_inicio) << CTL1_START_ADDRESS;
                 ADC14-> CTL0  |= ADC14_CTL0_MSC;

                 // Dispara con dirección definida.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SC_OFS) =  1;

                 // Espera interrupción.
                 usleep(20);

                 // Regresa dirección actual de timer.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;
                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  temp << CTL1_START_ADDRESS;
                 ADC14-> CTL0  &= ~ADC14_CTL0_MSC;

                 // Regresa control al timer.
                 TIMER32_1 -> CONTROL |= TIMER32_CONTROL_ENABLE;
                 TIMER32_1 -> CONTROL |= TIMER32_CONTROL_IE;

                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
             }

             else
             {
                 // Dirección temporal.
                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  (canal_final) << CTL1_START_ADDRESS;

                 // Disparo.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_SC_OFS)  = 1;

                 // Espera interrupción.
                 usleep(12);

                 // Regresa dirección.
                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;

                 ADC14 -> CTL1 &=  ~(0x1F << CTL1_START_ADDRESS);
                 ADC14 -> CTL1 |=  temp << CTL1_START_ADDRESS;

                 BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
             }

         // Regresa modalidad single channel.
            for(i = 0; i < 32; i++)
                BITBAND_PERI(ADC14->MCTL[i], ADC14_MCTLN_EOS_OFS) = 1;

            ADC14 -> CTL0 &= ~ADC14_CTL0_CONSEQ_3;
            ADC14 -> CTL0 |=  ADC_SingleChannel;
        }
    }

    usleep(10);
    return IO_OK;
}

/*FUNCTION***********************************************************************
*
* Function Name    : adc_pause
* Returned Value   : IO_OK
* Comments         : Pausa un canal o máscara que puede pausar varios de ellos.
*
*END****************************************************************************/

_mqx_int adc_pause(ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask)
{
    _mqx_int i;

    Int_disable();

    if (channel)
    {
        channel->runtime_flags &= ~ADC_CHANNEL_RESUMED;
        time_stopped[channel -> number] = ADC_time_channel_temp [channel -> number];    // Guarda el tiempo.
        ADC_time_channel_temp   [channel -> number] = 0;                                // Y luego apaga la cuenta regresiva.

    }
    else
    {
        for(i = 0; i < ADC_MAX_CHANNELS; i++)
            if(running_mask[mask] & 1 << i)
            {
                time_stopped[i] = ADC_time_channel_temp [i];
                ADC_time_channel_temp   [i] = 0;
            }

        for (i = 0; i < ADC_MAX_CHANNELS; i++)
        {
            if (adc_ch[i] && (adc_ch[i]->g.trigger & mask))
                adc_ch[i]->g.runtime_flags &= ~ADC_CHANNEL_RESUMED;
        }
    }

    Int_enable();
    return IO_OK;
}

/*FUNCTION**************************************************************************
*
* Function Name    : adc_resume
* Returned Value   : IO_OK
* Comments         : Continúa un canal o máscara que puede continuar varios de ellos.
*
*END********************************************************************************/

_mqx_int adc_resume(ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask)
{
    _mqx_int i;

    Int_disable();

    // Canal.
    if (channel)
    {
        channel->runtime_flags |= ADC_CHANNEL_RESUMED;
        if(time_stopped[channel -> number] != 0)
        {
            ADC_time_channel_temp   [channel -> number] = time_stopped[channel -> number];
            time_stopped[channel -> number] = 0;
        }
    }

    // Máscara.
    else
    {
        for(i = 0; i < ADC_MAX_CHANNELS; i++)
            if(running_mask[mask] & 1 << i)
                if(time_stopped[i] != 0)                            // Asegura no estar continuando más de una vez.
                {
                    ADC_time_channel_temp [i] = time_stopped[i];
                    time_stopped[i] = 0;
                }

        for (i = 0; i < ADC_MAX_CHANNELS; i++)
        {
            if (adc_ch[i] && (adc_ch[i]->g.trigger & mask))
                adc_ch[i]->g.runtime_flags |= ADC_CHANNEL_RESUMED;
        }
    }

    Int_enable();
    return IO_OK;
}

/*FUNCTION*********************************************************************
*
* Function Name    : adc_stop
* Returned Value   : IO_OK
* Comments         : Paro de un canal o máscara que puede parar varios de ellos.
*
*END***************************************************************************/

_mqx_int adc_stop(ADC_CHANNEL_GENERIC_PTR channel, ADC_TRIGGER_MASK mask)
{
    _mqx_int i, confirm_general_stop = 0;

    Int_disable();

    // Canal.
    if (channel)
    {
        channel->runtime_flags &= ~ADC_CHANNEL_RUNNING;
        ADC_time_channel_temp   [channel -> number] = 0;

        for(i = 0; i < ADC_MAX_CHANNELS; i++)
            if(running_mask[mask] & 1 << i)
                confirm_general_stop = 1;

        if(confirm_general_stop == 0)
            adc->g.run = 0;
    }

    // Máscara.
    else
    {
        for(i = 0; i < ADC_MAX_CHANNELS; i++)
            if(running_mask[mask] & 1 << i)
                ADC_time_channel_temp   [i] = 0;

        for (i = 0; i < ADC_MAX_CHANNELS; i++)
            if (adc_ch[i] && (adc_ch[i]->g.trigger & mask))
                    adc_ch[i]->g.runtime_flags &= ~ADC_CHANNEL_RUNNING;

        adc->g.run = 0;
    }

    Int_enable();
    return IO_OK;
}

/*FUNCTION*************************************************************************
*
* Function Name    : adc_close
* Returned Value   : IO_OK
* Comments         : Cierre de archivo de ADC, canales y cierre de interrupciones.
*
*END*******************************************************************************/

_mqx_int adc_close (FILE _PTR_ fd_ptr)
{
    int i;

    free(fd_ptr);

    if(adc != NULL)
        free(adc);

    for(i = 0; i < ADC_MAX_CHANNELS; i++)
        if(adc_ch[i] != NULL)
            free(adc_ch[i]);

    if(timer_activated[ADC_T])
    {
        TIMER32_1 -> CONTROL &= ~TIMER32_CONTROL_IE;
        TIMER32_1 -> LOAD = 0XFFFFFFFF;
        timer_activated[ADC_T] = FALSE;
    }

    BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = FALSE;
    ADC14 -> IER0 = 0x00;

    return IO_OK;
}

/*FUNCTION*****************************************************************
*
* Function Name    : adc_read
* Returned Value   : IO_OK
* Comments         : Lectura de un valor de un canal por cada 4 bytes.
*
*END*********************************************************************/

_mqx_int adc_read  (FILE_PTR_f fd_ptr, char_ptr data_ptr, _mqx_int num)
{
    ADC_CHANNEL_GENERIC_PTR ch_conf;
    uint_32_ptr dst;
    _mqx_int temp, while_flag;

    Int_disable();

    ch_conf = (ADC_CHANNEL_GENERIC_PTR) fd_ptr->DEV_DATA_PTR;
    dst =  (uint_32_ptr) data_ptr;
    temp = (adc -> results[ch_conf -> number]); // Obtención de la última lectura del canal.

    while_flag = num/sizeof(_mqx_int);

    // Vaciado en un entero de 32 bits mínimo a través de apuntadores.
    while (while_flag)
    {
        *dst++ = temp;
        while_flag--;
    }

    Int_enable();

    return IO_OK;
}

/*FUNCTION*****************************************************************
*
* Function Name    : adc_temperature
* Returned Value   : float con temperatura.
* Comments         : Lectura de un valor de un canal por cada 4 bytes.
*
*END************************************************************************/

_mqx_int adc_temperature(ADC_CHANNEL_GENERIC_PTR channel, pointer var)
{
    float   temp = 0.0;
    float*  ptr = (float *) var;
    uint16_t cal30 = TLV->ADC14_REF2P5V_TS30C;  // Registros.
    uint16_t cal85 = TLV->ADC14_REF2P5V_TS85C;  // Registros.
    float calDiff = cal85 - cal30;
    temp =  (((((adc -> results[channel ->number]) - cal30) * 55) / calDiff) + 30.0f);
    *ptr = temp;
    return IO_OK;
}

/*FUNCTION**********************************************************************
*
* Function Name    : adc_is_busy
* Returned Value   : TRUE or FALSE
* Comments         : Devuelve si está ocupado el ADC en una conversión.
*
*END****************************************************************************/

boolean adc_is_busy(void)
{
    return BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_BUSY_OFS);
}

/*FUNCTION**********************************************************************
*
* Function Name    : adc_hw_get_time
* Returned Value   : Entero de 32 bits con tiempo.
* Comments         : Regresa el tiempo actual del timer (load).
*
*END****************************************************************************/

_mqx_int adc_hw_get_time(void)
{
    if(timer_activated[ADC_T])
        return TIMER32_1 -> LOAD;
    else
        return -1;
}
