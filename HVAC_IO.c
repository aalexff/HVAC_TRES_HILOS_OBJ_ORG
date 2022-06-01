 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a trav�s de estados y objetos.
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupci�n habilitada, registrada e inicializaci�n de m�dulos.
 * Overview: Funci�n que es llamada cuando se genera
 *           la interrupci�n del bot�n SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    Int_clear_gpio_flags(input_port);

    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[0] & GPIO_PIN_STATUS) == 0)        // Lectura de los pines, �ndice cero es TEMP_PLUS.
        HVAC_SetPointUp();

    else if((data[1] & GPIO_PIN_STATUS) == 0)   // Lectura de los pines, �ndice uno es TEMP_MINUS.
        HVAC_SetPointDown();

    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heat
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser mayor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la funci�n.
*
*END***********************************************************************************/
void HVAC_Heat(void)
{
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &heat);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);

    if(TemperaturaActual < SetPoint)                    // El fan se debe encender si se quiere una temp. m�s alta.
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
*    en 'auto' y este modo debe estar activado para entrar a la funci�n.
*
*END***********************************************************************************/
void HVAC_Cool(void)
{
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &cool);

    if(TemperaturaActual > SetPoint)                        // El fan se debe encender si se quiere una temp. m�s baja.
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
* Function Name    : HVAC_SetPointUp
* Returned Value   : None.
* Comments         :
*    Sube el valor deseado (set point). Llamado por interrupci�n a causa del SW1.
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
*    Baja el valor deseado (set point). Llamado por interrupci�n a causa del SW2.
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
