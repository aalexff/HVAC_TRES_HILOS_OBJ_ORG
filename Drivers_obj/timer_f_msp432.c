 //FileName:        timer_f_msp432.c
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Definición de funciones del módulo timer32. Source File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"    

// Tiempo que escala en pasos de la variable step hasta llegar al periodo deseado de cada unidad.
_mqx_uint  microsecs = 0;

// Estructuras generales, son reservadas dinámicamente.
TIMER_PTR timer = NULL;                                         // Timer general.
TIMER_UNIT_DATA_PTR timer_units[MAX_TIMER_UNITS] = { 0 };       // Unidades.

// Banderas al entrar a estados iniciales y bandera para funcionamiento de interrupciones.
boolean timer_activated[2]              = {FALSE, FALSE};
boolean bandera_stop                    = FALSE;

// Arreglo con tiempos pausados (si los hay).
uint_32 time_paused_left[MAX_TIMER_UNITS] = { 0 };

// Arreglo con estados anteriores.
uint_32 last_state[MAX_TIMER_UNITS] = { STOPPED };

/*FUNCTION******************************************************************************
*
* Function Name    : Timer_Handler
* Returned Value   : None.
* Comments         :
*    Función de interrupción del módulo timer32_2 para corrida de las unidades abiertas
*    de los timers.
*
*END***********************************************************************************/

void Timer_Handler(void)
{
    _mqx_int i;

    Int_disable();                                              // Desactiva interrupciones.
    TIMER32_2 -> INTCLR = 0;                                    // Borra bandera de timer32_2.

    microsecs += timer -> step;                                 // Aumenta el tiempo tomado.
    if (microsecs >= TIME_RESET)
        microsecs = 0;


    if(timer_activated[SOLO_TIMER] == TRUE)                     // Si este timer está activado:
    {
        for(i = 0; i < MAX_TIMER_UNITS; i++)                    // Evalúa cada una de las unidades.
             {
                 if(timer_units[i] == NULL)                     // No hay necesidad de entrar si no se ha reservado memoria.
                     continue;

                 // Si el tiempo se debe parar, debe venir de un estado de RUN o PAUSED o inmediatamente de un STOP.
                 if(timer_units[i] -> state == STOPPED && (last_state[i] == RUN || last_state[i] == PAUSED
                         || bandera_stop == FALSE))
                 {
                     // Vacía todos los recursos en cero's.
                     timer -> time[SECONDS][i] = 0;
                     timer -> time[MINUTES][i] = 0;
                     timer -> time[HOURS][i]   = 0;
                     timer -> time_left[i]   = SEC;

                     // Coloca al timer en estado de STOPPED.
                     last_state[i] = STOPPED;
                     timer_units[i] -> state = STOPPED;
                     timer_units[i] -> period_flag =   FALSE;
                     bandera_stop = TRUE;

                     return;
                 }

                 // Si el estado tiene que pausarse, debe venir del estado RUN.
                 if(timer_units[i] -> state == PAUSED && last_state[i] == RUN)
                 {
                     time_paused_left[i] = timer -> time_left[i];   // Recuerda el tiempo restante al periodo.
                     last_state[i] = PAUSED;                        // Pausa de la unidad.
                     timer_units[i] -> state = PAUSED;

                     return;
                 }

                 // En realidad solo hay 3 estados: RUN, PAUSED y STOPPED; RESUMED solo es un estado temporal,
                 // que regresa a RUN.
                 else if(timer_units[i] -> state == RESUMED)
                 {
                     if(last_state[i] == STOPPED)
                     {
                         timer_units[i] -> state = STOPPED;
                         return;
                     }

                     else if(last_state[i] != RUN)
                         timer -> time_left[i] = (time_paused_left[i] > 0)? time_paused_left[i]: SEC;

                     time_paused_left[i] = 0;               // Limpia el valor pausado.
                     timer_units[i] -> state = RUN;         // Pone al estado en corrida de nuevo.
                     last_state[i] = RUN;                   // Lo mismo para el estado anterior.
                 }

                 // Estado de corrida principal.
                 if(timer_units[i] -> state == RUN)
                 {
                       if(last_state[i] == STOPPED)
                       {
                             last_state[i] = RUN;           // Renueva el estado anterior como RUN.
                             timer_units[i] -> state = RUN;
                       }

                       if(timer -> time_left[i] != 0)
                       {
                           bandera_stop = FALSE;

                           // Disminuye la cuenta regresiva.
                           timer -> time_left[i] -= timer -> step;
                           if((timer -> time_left[i])/(timer -> step) == 0)                    // Al acabar esta cuenta, entra.
                           {
                               // Actualización y valores.
                               last_state[i] = RUN;

                               timer -> time_left[i] = timer -> time_full[i];                  // Se renueva contador (tiempo).
                               timer -> time[SECONDS][i] += ((timer -> time_full[i]) / SEC);

                               if(timer -> time[SECONDS][i] >= 60)                             // Actualiza segundos.
                               {
                                   timer -> time[SECONDS][i] = 0;
                                   timer -> time[MINUTES][i] += 1;

                                   if(timer -> time[MINUTES][i] >= 60)                         // Actualiza minutos.
                                   {
                                       timer -> time[MINUTES][i] = 0;
                                       timer -> time[HOURS][i] += 1;

                                       if(timer -> time[HOURS][i] >= 24)                       // Actualiza horas.
                                           timer -> time[HOURS][i] = 0;
                                   }
                               }

                               // Zona de banderas y toggles de periodo en caso de que estén permitidos.
                               if(timer_units[i] -> flags[P_TOGGLE] == TRUE)
                               {
                                   if(timer_units[i] -> period_toggle == TRUE)
                                        timer_units[i] -> period_toggle = FALSE;
                                    else
                                        timer_units[i] -> period_toggle = TRUE;
                               }

                               // Ha ocurrido un periodo.
                               if(timer_units[i] -> flags[P_FLAG] == TRUE)
                                   timer_units[i] -> period_flag = TRUE;

                               // Si se llega a la cuenta máxima prescrita, reinicia el conteo y activa bandera.
                               if (timer -> time[HOURS][i] >= (timer_units[i] -> max[HOURS]))
                                   if(timer -> time[MINUTES][i] >= (timer_units[i] -> max[MINUTES]))
                                       if(timer -> time[SECONDS][i] >= (timer_units[i] -> max[SECONDS]))
                                       {
                                           // Reinicio.
                                           timer -> time[SECONDS][i] = 0;
                                           timer -> time[MINUTES][i] = 0;
                                           timer -> time[HOURS][i] = 0;

                                           // Zona de banderas y acciones al terminar la cuenta.
                                           if(timer_units[i] -> flags[P_END] == TRUE)
                                           {
                                               timer_units[i] -> period_end  = TRUE;
                                               timer_units[i] -> period_flag = FALSE;
                                           }

                                           // Si tiene la etiqueta 'MAX AND STOPPED', entrará al if.
                                           if(timer_units[i] -> flags[M_STOPPED] == TRUE)
                                           {
                                               timer_units[i] -> state = STOPPED;
                                               last_state[i] =     STOPPED;
                                           }

                                       }    // Fin if(timer -> time[HOURS]...
                           }    // Fin if(timer -> time_left/step ...
                      }     // Fin if(timer -> time_left != 0 ...
                 }  // Fin if (timer_units[i] -> state == RUN ...
             }  // Fin for(i = 0 ...
    }   // Fin if(timer_activated ...

    // Renueva las interrupciones.
    Int_enable();

   return;
}


/*FUNCTION******************************************************************************
*
* Function Name    : clean timer
* Returned Value   : None.
* Comments         :
*    Limpia todos los valores; se usa cuando reserva dinámicamente las estructuras
*    principales del módulo y unidades y tienen valores desconocidos (basura).
*
*END***********************************************************************************/

void clean_timer (void)
{
    uint_32 i, j;

    if(timer == NULL)
        return;

    // Limpieza en cero's.
    for(i = 0; i < 3; i++)
        for(j = 0; j < MAX_TIMER_UNITS; j++)
        {
            timer -> time[i][j] = 0;
            if(i == 0)
            {
                timer -> time_full[j] = 0;  // Limpieza de tiempos menores al periodo.
                timer -> time_left[j] = 0;
            }
        }
    return;
}


/*FUNCTION******************************************************************************
*
* Function Name    : timer_open
* Returned Value   : int de inicialización correcta.
* Comments         :
*    Reserva memoria e inicializa los recursos de los drivers.
*
*END***********************************************************************************/

_mqx_int timer_open  (FILE_PTR_f fd_ptr, char_ptr open_name_ptr, char_ptr flags)
{
    char_ptr file_name_ptr = fd_ptr->DEV_PTR->IDENTIFIER;
    _mqx_int flag;

    while (*file_name_ptr++ != 0)
       open_name_ptr++;                                 // Mueve al nombre del archivo.
    file_name_ptr = open_name_ptr;

    // Inicialización del módulo principal "timer:".
    if (*file_name_ptr == 0)
    {
        TIMER_INIT_STRUCT_PTR init_from = (TIMER_INIT_STRUCT_PTR) flags;

        if (timer == NULL)
        {
           timer = (TIMER_PTR) malloc (sizeof(TIMER));  // Reserva memoria dinámica.
           if (timer == NULL)
              return IO_ERR;
        }
        else
           return IO_ERR;

        clean_timer();                                  // Limpieza general.

        timer -> state_gral = init_from -> run_now;     // El estado depende de la configuración recibida.
        if((init_from -> step) > MINIMUM_LIMIT_STEP)    // Debe haber un mínimo óptimo para el step.
            timer -> step = init_from -> step;

        fd_ptr -> DEV_DATA_PTR = (pointer) init_from;   // La configuración se guarda en el archivo FILE.

        if(timer -> state_gral == RUN)
            flag = timer_hw_init();                     // Inicializa el HW del timer.

        return flag;
    }

    // Inicialización de una unidad: "timer:n" para n < MAX_TIMER_UNITS.
    else
    {
        TIMER_UNIT_INIT_STRUCT_PTR init_from = (TIMER_UNIT_INIT_STRUCT_PTR) flags;
        _mqx_uint                   ch = 0;
        _mqx_uint                   radix = 1;

        if (init_from == NULL)
           return IO_ERR;                                                      // No hay parámetros.

       if (NULL == timer)                                                      // No se ha inicializado el módulo principal.
           return IO_ERR;

          while ((*open_name_ptr >= '0') && (*open_name_ptr <= '9'))
              open_name_ptr++;

          if (*open_name_ptr != 0)
             return IO_ERR;

          open_name_ptr--;                                                     // Mueve al último dígito de la cadena.

          do
          {
             if (MAX_TIMER_UNITS <= (ch += radix * (*open_name_ptr - '0')))
                return IO_ERR;                                                 // Número excedido.
             radix *= 10;
          }
          while (open_name_ptr-- != file_name_ptr);

          if(ch > MAX_TIMER_UNITS)
              return IO_ERR;

          if (timer_units[ch] == NULL)                                         // Reservación dinámica de la unidad.
          {
              timer_units[ch] = (TIMER_UNIT_DATA_PTR) malloc (sizeof(TIMER_UNIT_DATA));
             if (timer_units[ch] == NULL)
                return IO_ERR;
          }
          else
             return IO_ERR;                                                    // Unidad ya usada por un archivo.

        // Con esto inicia la configuración deseada.
        timer_units[ch] -> num = ch;

        // Banderas y toggles.
        timer_units[ch] -> flags[M_STOPPED] = (init_from -> flags & (1 << M_STOPPED)) != 0;
        timer_units[ch] -> flags[P_FLAG]    = (init_from -> flags & (1 << P_FLAG))    != 0;
        timer_units[ch] -> flags[P_TOGGLE]  = (init_from -> flags & (1 << P_TOGGLE))  != 0;
        timer_units[ch] -> flags[P_END]     = (init_from -> flags & (1 << P_END))     != 0;
        timer_units[ch] -> period_flag   = FALSE;
        timer_units[ch] -> period_end    = FALSE;
        timer_units[ch] -> period_toggle = FALSE;

        // Tiempo máximo.
        timer_units[ch] -> max[SECONDS] = (init_from -> max[SECONDS]);
        timer_units[ch] -> max[MINUTES] = (init_from -> max[MINUTES]);
        timer_units[ch] -> max[HOURS]   = (init_from -> max[HOURS]);

        // Periodo y estado.
        timer_units[ch] -> scale = init_from -> time_period;
        timer_units[ch] -> state = init_from -> state;

        // Tiempo lleno y si está en run, tiempo por recorrer.
        timer -> time_full[ch] = (init_from -> time_period)*SEC;
        if(timer_units[ch] -> state == RUN)
            timer -> time_left[ch] = (init_from -> time_period)*SEC;

       // Pasa a formar parte del archivo.
       fd_ptr -> DEV_DATA_PTR = (pointer) timer_units[ch];

       return IO_OK;
    }
}


/*FUNCTION******************************************************************************
*
* Function Name    : timer_hw_init
* Returned Value   : int de inicialización correcta.
* Comments         :
*    Presenta funciones interrupciones y arranca el timer a través de los registros.
*
*END***********************************************************************************/

 _mqx_int timer_hw_init (void)
{
     static boolean bandera_interrupt_time = FALSE;

     if(!bandera_interrupt_time)
     {
         Int_registerInterrupt(INT_T32_INT2, Timer_Handler);
         Int_enableInterrupt(INT_T32_INT2);
         bandera_interrupt_time = 1;
     }

     TIMER32_2 -> CONTROL |= TIMER32_CONTROL_ENABLE;
     TIMER32_2 -> LOAD = ((__SYSTEM_CLOCK)/(SEC))*(STEP) - 1;                    // Valor a cargar.
     TIMER32_2 -> CONTROL = 0xC2;                                                // 32 bit, periódicamente, con base de tiempo.
     TIMER32_2 -> CONTROL |= TIMER32_CONTROL_PRESCALE_0;                         // No hay prescaler.
     TIMER32_2 -> CONTROL |= TIMER32_CONTROL_IE;                                 // Habilita interrupción.
     timer_activated[SOLO_TIMER] = 1;

     return IO_OK;
}


 /*FUNCTION******************************************************************************
 *
 * Function Name    : timer_ioctl
 * Returned Value   : entero de control correcto.
 * Comments         :
 *    Controla las unidades cambiándolos, o actualizando banderas o cadenas.
 *
 *END***********************************************************************************/

_mqx_int timer_ioctl (FILE_PTR_f fd_ptr, _mqx_uint cmd, pointer param_ptr)
{
   //uint_32 num = (uint_32) param_ptr;                                              // Pensado para futuras definiciones.
   TIMER_UNIT_DATA_PTR dev_data_ptr = (TIMER_UNIT_DATA_PTR) fd_ptr -> DEV_DATA_PTR;  // Recoge instrucción.

   Int_disable();                                                                    // Inhabilita interrupciones.

   if(param_ptr != NULL)                                                             // Si se recibe algo diferente de NULL.
   {                                                                                 // Se desea modificar timer principal.
       switch (cmd)                                                                  // Modifica timer principal.
       {
           case IOCTL_TIMER_RUN:    timer -> state_gral = RUN;      break;
           case IOCTL_TIMER_RESUME: timer -> state_gral = RESUMED;  break;
           case IOCTL_TIMER_PAUSE:  timer -> state_gral = PAUSED;   break;
           case IOCTL_TIMER_STOP:   timer -> state_gral = STOPPED;  break;
           default:                                                 break;
       }
   }

   else if((dev_data_ptr -> num) < (MAX_TIMER_UNITS + 1))                             // Si null es recibido.
   {
       switch (cmd)                                                                   // Modifica unidad.
       {
           case IOCTL_TIMER_RUN:    timer_units[dev_data_ptr -> num] -> state = RUN;                break;
           case IOCTL_TIMER_RESUME: timer_units[dev_data_ptr -> num] -> state = RESUMED;            break;
           case IOCTL_TIMER_PAUSE:  timer_units[dev_data_ptr -> num] -> state = PAUSED;             break;
           case IOCTL_TIMER_STOP:   timer_units[dev_data_ptr -> num] -> state = STOPPED;
                                    timer_units[dev_data_ptr -> num] -> period_flag =   FALSE;
                                    timer_units[dev_data_ptr -> num] -> period_toggle = FALSE;      break;
           default:                                                                                 break;
       }

       fd_ptr -> DEV_DATA_PTR = (pointer) timer_units[dev_data_ptr -> num];          // Actualiza archivo.
   }

   Int_enable();                                                                    // Renueva interrupciones.
   return IO_OK;
}


/*FUNCTION******************************************************************************
*
* Function Name    : timer_close
* Returned Value   : entero de control correcto.
* Comments         :
*    Cierra los archivos correspondientes de memoria.
*
*END***********************************************************************************/

_mqx_int timer_close (FILE _PTR_ fd_ptr)
{
    uint_32 i;

    Int_disable();
    if(timer_activated[SOLO_TIMER])
    {
        TIMER32_2 -> CONTROL &= ~TIMER32_CONTROL_IE;    // Apaga el timer32_2.
        TIMER32_2 -> LOAD = 0XFFFFFFFF;
        timer_activated[SOLO_TIMER] = FALSE;
    }

    if(timer != NULL)
        free(timer);

    for(i = 0; i < MAX_TIMER_UNITS; i++)                // Cierra las unidades abiertas.
    {
        if(timer_units[i] != NULL)
            free(timer_units[i]);
    }

    free(fd_ptr);                                       // Cierra el archivo.

    Int_enable();
    return IO_OK;
}

/*FUNCTION******************************************************************************
*
* Function Name    : timer_read
* Returned Value   : entero de control correcto.
* Comments         :
*    Lectura de banderas, cadenas y tiempo de una unidad.
*
*END***********************************************************************************/

_mqx_int timer_read  (FILE_PTR_f fd_ptr, char_ptr data_ptr, _mqx_int num)
{
    uint_32_ptr dir;
    char_ptr    dir_string;
    TIMER_UNIT_DATA_PTR dev_data_ptr = (TIMER_UNIT_DATA_PTR) fd_ptr -> DEV_DATA_PTR;

    // Dependiendo del formato, se leerá un numérico o una cadena.

    if(num < T_STRING)
        dir =  (uint_32_ptr) data_ptr;
    else if(num == T_STRING)
        dir_string = (char_ptr) data_ptr;
    else
        return IO_ERR;

    // Desactiva interrupciones.
    Int_disable();

    switch(num)
    {
        // Lectura de period_flag y limpieza de esta bandera.
        case P_FLAG:        *dir = timer_units[dev_data_ptr -> num] -> period_flag;
                            timer_units[dev_data_ptr -> num] -> period_flag = 0;            break;

        // Lectura de period_toggle.
        case P_TOGGLE:      *dir = timer_units[dev_data_ptr -> num] -> period_toggle;       break;

        // Lectura de period_end (cuando se llegue al tiempo máximo).
        case P_END:         *dir = timer_units[dev_data_ptr -> num] -> period_end;
                            timer_units[dev_data_ptr -> num] -> period_end = 0;             break;

        // Lectura de milisegundos (base 1000).
        case T_MILLIS:      *dir = (timer -> time_left[dev_data_ptr -> num])/1000;          break;

        // Lectura de un tiempo.
        case T_SECONDS:     *dir = timer -> time[SECONDS][dev_data_ptr -> num];             break;
        case T_MINUTES:     *dir = timer -> time[MINUTES][dev_data_ptr -> num];             break;
        case T_HOURS:       *dir = timer -> time[HOURS]  [dev_data_ptr -> num];             break;

        // Lectura del tiempo pero en formato de una cadena.
        case T_STRING:      sprintf(dir_string, "%02d:%02d:%02d\n\r",
                            timer -> time[HOURS][dev_data_ptr -> num],
                            timer -> time[MINUTES][dev_data_ptr -> num],
                            timer -> time[SECONDS][dev_data_ptr -> num]);                   break;
        default:                                                                            break;
    }

    fd_ptr -> DEV_DATA_PTR = (pointer) timer_units[dev_data_ptr -> num];                    // Actualiza archivo.
    Int_enable();                                                                           // Renueva interrupciones.

    return IO_OK;
}
