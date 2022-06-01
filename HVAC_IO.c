 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados y objetos.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"


/* Variables sobre las cuales se maneja el sistema. */

float TemperaturaActual = 20;  // Temperatura.
float SetPoint = 25.0;         // V. Deseado.

char state[MAX_MSG_SIZE];      // Cadena a imprimir.

bool toggle = 0;               // Toggle para el heartbeat.
_mqx_int delay;                // Delay aplicado al heartbeat.
bool event = FALSE;

bool FAN_LED_State = 0;                                     // Estado led_FAN.
const char* SysSTR[] = {"Cool","Off","Heat","Only Fan"};    // Control de los estados.

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    Int_clear_gpio_flags(input_port);

    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[0] & GPIO_PIN_STATUS) == 0)        // Lectura de los pines, índice cero es TEMP_PLUS.
        HVAC_SetPointUp();

    else if((data[1] & GPIO_PIN_STATUS) == 0)   // Lectura de los pines, índice uno es TEMP_MINUS.
        HVAC_SetPointDown();

    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgirán
*    las salidas.
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{
    static bool ultimos_estados[] = {FALSE, FALSE, FALSE, FALSE, FALSE};        //PARA CONTROL DE EVENTOS

    ioctl(fd_ch_T, IOCTL_ADC_READ_TEMPERATURE, (pointer) &TemperaturaActual);   // Actualiza valor de temperatura.
    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[2] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)        // Cambia el valor de las entradas FAN.
    {
        EstadoEntradas.FanState = On;
        EstadoEntradas.SystemState = FanOnly;
        if(ultimos_estados[0] == FALSE)                 //SI ULTIMO ESTADO FANSTATE NO ES ON
            event = TRUE;                               //BANDERA OCURRIO EVENTO

        ultimos_estados[0] = TRUE;                      //FANSTATE ON
        ultimos_estados[1] = FALSE;                     //FANSTATE AUTO
    }

    else if((data[3] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)   // Cambia el valor de las entradas SYSTEM.
    {
        EstadoEntradas.FanState = Auto;                //PON fanstate en AUTO
        if(ultimos_estados[1] == FALSE)                //SI ULTIMO ESTADO FANSTATE NO ES AUTO
            event = TRUE;                              //BANDERA OCURRIO EVENTO

       ultimos_estados[0] = FALSE;                     //FANSTATE QUITAR ON
       ultimos_estados[1] = TRUE;                      //FANSTATE ULTIMO ESTADO PONER AUTO

        if((data[4] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)   // Y así sucesivamente para el resto de pines.
        {
            EstadoEntradas.SystemState = Cool;
            if(ultimos_estados[2] == FALSE)
                event = TRUE;
            ultimos_estados[2] = TRUE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = FALSE;
        }
        else if((data[5] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
        {
            EstadoEntradas.SystemState = Off;
            if(ultimos_estados[3] == FALSE)
                event = TRUE;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = TRUE;
            ultimos_estados[4] = FALSE;
        }
        else if((data[6] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
        {
            EstadoEntradas.SystemState = Heat;
            if(ultimos_estados[4] == FALSE)
                event = TRUE;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = TRUE;
        }
        else
        {
            EstadoEntradas.SystemState = Off;       //SI NO SE PRESIONO CUALQUIERA DELOS BOTONES PASADOS PON SystemState en OFF
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = FALSE;
        }
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarSalidas
* Returned Value   : None.
* Comments         :
*    Decide a partir de las entradas actualizadas las salidas principales,
*    y en ciertos casos, en base a una cuestión de temperatura, la salida del 'fan'.
*
*END***********************************************************************************/
void HVAC_ActualizarSalidas(void)
{
    // Cambia el valor de las salidas de acuerdo a entradas.

    if(EstadoEntradas.FanState == On)                               // Para FAN on.
    {
        FAN_LED_State = 1;
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);
    }

    else if(EstadoEntradas.FanState == Auto)                        // Para FAN automatico.
    {
        switch(EstadoEntradas.SystemState)
        {
        case Off:   ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
                    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
                    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);
                    FAN_LED_State = 0;
                    break;
        case Heat:  HVAC_Heat(); break;
        case Cool:  HVAC_Cool(); break;
        }
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heat
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser mayor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la función.
*
*END***********************************************************************************/
void HVAC_Heat(void)
{
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &heat);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);

    if(TemperaturaActual < SetPoint)                    // El fan se debe encender si se quiere una temp. más alta.
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        FAN_LED_State = 1;
    }
    else
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
        FAN_LED_State = 0;
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Cool
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser menor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la función.
*
*END***********************************************************************************/
void HVAC_Cool(void)
{
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &cool);

    if(TemperaturaActual > SetPoint)                        // El fan se debe encender si se quiere una temp. más baja.
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        FAN_LED_State = 1;
    }
    else
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
        FAN_LED_State = 0;
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heartbeat
* Returned Value   : None.
* Comments         :
*    Función que prende y apaga una salida para notificar que el sistema está activo.
*    El periodo en que se hace esto depende de una entrada del ADC en esta función.
*
*END***********************************************************************************/
void HVAC_Heartbeat(void)               // Función de 'alive' del sistema.
{
   _mqx_int val;
   boolean flag = 0;
   static boolean bandera_inicial = 0;

   if(bandera_inicial == 0)
   {
       // Entrando por primera vez, empieza a correr el canal de heartbeat.
       ioctl (fd_ch_H, IOCTL_ADC_RUN_CHANNEL, NULL);
       bandera_inicial = 1;
   }

   // Valor se guarda en val, flag nos dice si fue exitoso.
   flag =  (fd_adc && fread_f(fd_ch_H, &val, sizeof(val))) ? 1 : 0;

   if(flag != TRUE)
   {
       printf("Error al leer archivo. Cierre del programa\r\n");
       exit(1);
   }

    delay = 15000 + (100 * val / 4);            // Lectura del ADC por medio de la función.
    //Nota: delay no puede ser mayor a 1,000,000 ya que luego se generan problemas en usleep.

    if(toggle)
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &hbeat);
    else
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &hbeat);

    toggle ^= 1;                             // Toggle.

    usleep(delay);                           // Delay marcado por el heart_beat.
    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    static uint_32 delay = DELAY;
    delay -= DELAY;

    if(delay <= 0 || event == TRUE)
    {
        event = FALSE;
        delay = SEC;

        sprintf(state,"Fan: %s, System: %s, SetPoint: %0.2f\n\r",
                    EstadoEntradas.FanState == On? "On":"Auto",
                    SysSTR[EstadoEntradas.SystemState],
                    SetPoint);
        print(state);

        sprintf(state,"Temperatura Actual: %0.2f°C %0.2f°F  Fan: %s\n\r\n\r",
                    TemperaturaActual,
                    ((TemperaturaActual*9.0/5.0) + 32),
                    FAN_LED_State?"On":"Off");
        print(state);
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_SetPointUp
* Returned Value   : None.
* Comments         :
*    Sube el valor deseado (set point). Llamado por interrupción a causa del SW1.
*
*END***********************************************************************************/
void HVAC_SetPointUp(void)
{
    SetPoint += 0.5;
    event = TRUE;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_SetPointDown
* Returned Value   : None.
* Comments         :
*    Baja el valor deseado (set point). Llamado por interrupción a causa del SW2.
*
*END***********************************************************************************/
void HVAC_SetPointDown(void)
{
    SetPoint -= 0.5;
    event = TRUE;
}


void control_De_Hilos(void){
    static uint_32 delay = DELAY;
        delay -= DELAY;

    if(delay <= 0 || event == TRUE){
        event=FALSE;
        delay=SEC;
        if((EstadoEntradas.FanState==Auto) && (EstadoEntradas.SystemState==Off)){
            control_hilos_desactivados();
            sprintf(state,"Salida y HeartBeat apagados\n\r");
            print(state);
        }
        else{
            control_hilos_activados();
        }
    }
}
