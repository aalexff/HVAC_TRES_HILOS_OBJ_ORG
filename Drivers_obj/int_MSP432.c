 //FileName:        int_MSP432.h
 //Dependencies:    system.h
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Tratamiento y sincronización de los drivers propuestos. Comentarios en ingles. Source File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 //Updated:         12/2018

#include "HVAC.h"

#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
static __no_init void (*g_pfnRAMVectoring[NUM_INTERRUPTS+1])(void) @ "VTABLE";
#elif defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(g_pfnRAMVectoring, 1024)
#pragma DATA_SECTION(g_pfnRAMVectoring, ".vtable")
void (*g_pfnRAMVectoring[NUM_INTERRUPTS + 1])(void);
#else
static __attribute__((section("vtable")))
void (*g_pfnRAMVectoring[NUM_INTERRUPTS+1])(void) __attribute__((aligned(1024)));
#endif


//*****************************************************************************
// This is a mapping between interrupt number (for the peripheral interrupts  *
// only) and the register that contains the interrupt enable for that         *
// interrupt.                                                                 *
//*****************************************************************************
static const uint32_t g_pulEnRegs[] =
{ NVIC_EN0_R, NVIC_EN1_R };

//*****************************************************************************
// This is a mapping between interrupt number (for the peripheral interrupts  *
// only) and the register that contains the interrupt disable for that        *
// interrupt.                                                                 *
//*****************************************************************************
static const uint32_t g_pulDisRegs[] =
{ NVIC_DIS0_R, NVIC_DIS1_R };


extern GPIO_PIN_MAP gpio_global_pin_map, gpio_global_irq_map;
extern boolean RX_interruption;

static void IntDefaultHandler(void)
{
    // Loop infinito.
    while (1);
}

/*FUNCTION******************************************************************************
*
* Function Name    : Int_disable
* Returned Value   : None
* Comments         :
*    Función para deshabilitar convenientemente las interrupciones.
*
*END***********************************************************************************/
void Int_disable  (void)
{
    // Impide interrupciones en un momento en particular.

    // GPIO.
    P1 -> IE = 0x00;
    P2 -> IE = 0x00;
    P3 -> IE = 0x00;
    P4 -> IE = 0x00;
    P5 -> IE = 0x00;
    P6 -> IE = 0x00;

    //ADC con timer.
    ADC14 -> IER0 = 0x00;
    TIMER32_1 -> CONTROL &= ~TIMER32_CONTROL_IE;

    // Timer como objeto.
    TIMER32_2 -> CONTROL &= ~TIMER32_CONTROL_IE;

    // UART_RX.
    EUSCI_A0 -> IE &= ~EUSCI_A_IE_RXIE;

}

/*FUNCTION******************************************************************************
*
* Function Name    : Int_clear_gpio_flags
* Returned Value   : None
* Comments         :
*    Función para poner en bajo las banderas de interrupción de los objetos de gpio.
*
*END***********************************************************************************/
void Int_clear_gpio_flags(FILE _PTR_ file_ptr)
{
    FILE_f                 fd[1];
    FILE_PTR_f             fd_ptr;

    if(file_ptr != NULL)
    {
        fread(fd, sizeof(fd),1, file_ptr);
        rewind(file_ptr);

        fd_ptr = fd;
        GPIO_DEV_DATA_PTR  dev_data_ptr = (GPIO_DEV_DATA_PTR) fd_ptr -> DEV_DATA_PTR;

        if (dev_data_ptr -> type == DEV_INPUT)
        {
            P1 -> IFG &= ~dev_data_ptr->irq_map.memory8[0];
            P2 -> IFG &= ~dev_data_ptr->irq_map.memory8[1];
            P3 -> IFG &= ~dev_data_ptr->irq_map.memory8[2];
            P4 -> IFG &= ~dev_data_ptr->irq_map.memory8[3];
            P5 -> IFG &= ~dev_data_ptr->irq_map.memory8[4];
            P6 -> IFG &= ~dev_data_ptr->irq_map.memory8[5];
        }
    }

    P1 -> IFG = 0x00;
    P2 -> IFG = 0x00;
    P3 -> IFG = 0x00;
    P4 -> IFG = 0x00;
    P5 -> IFG = 0x00;
    P6 -> IFG = 0x00;

}

/*FUNCTION******************************************************************************
*
* Function Name    : Int_enable
* Returned Value   : None
* Comments         :
*    Función para habilitar convenientemente las interrupciones.
*
*END***********************************************************************************/

void Int_enable (void)
{
    // Habilitan interrupciones si ya estaban antes.

    // ADC
    ADC14 -> IER0 = ADC_global_irq_map;

    //GPIO
    P1 -> IE |= gpio_global_irq_map.memory8[0];
    P2 -> IE |= gpio_global_irq_map.memory8[1];
    P3 -> IE |= gpio_global_irq_map.memory8[2];
    P4 -> IE |= gpio_global_irq_map.memory8[3];
    P5 -> IE |= gpio_global_irq_map.memory8[4];
    P6 -> IE |= gpio_global_irq_map.memory8[5];

    // Timer (ADC) y cronómetros.
    if(timer_activated[ADC_T])
        TIMER32_1 -> CONTROL |= TIMER32_CONTROL_IE;

    if(timer_activated[SOLO_TIMER])
        TIMER32_2 -> CONTROL |= TIMER32_CONTROL_IE;

    //UART_RX
    if(RX_interruption == TRUE)
        EUSCI_A0 -> IE |= EUSCI_A_IE_RXIE;

}

/*******************************************************
 *  El resto de las funciones son propuestas por TI.  *
 *******************************************************/

void Int_enableInterrupt(uint32_t interruptNumber)
{
    // Determine the interrupt to enable.
    if (interruptNumber == FAULT_MPU)
        SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;                                                    // Enable the MemManage interrupt.

    else if (interruptNumber == FAULT_BUS)
        SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;                                                    // Enable the bus fault interrupt.

    else if (interruptNumber == FAULT_USAGE)
        SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;                                                    // Enable the usage fault interrupt.

    else if (interruptNumber == FAULT_SYSTICK)
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;                                                   // Enable the System Tick interrupt.

    else if (interruptNumber >= 16)
        HWREG32 (g_pulEnRegs[(interruptNumber - 16) / 32]) = 1 << ((interruptNumber - 16) & 31);    // Enable the general interrupt.
}

void Int_disableInterrupt(uint32_t interruptNumber)
{
    // Determine the interrupt to disable.
    if (interruptNumber == FAULT_MPU)
        SCB->SHCSR &= ~(SCB_SHCSR_MEMFAULTENA_Msk);                                                 // Disable the MemManage interrupt.

    else if (interruptNumber == FAULT_BUS)
        SCB->SHCSR &= ~(SCB_SHCSR_BUSFAULTENA_Msk);                                                 // Disable the bus fault interrupt.

    else if (interruptNumber == FAULT_USAGE)
        SCB->SHCSR &= ~(SCB_SHCSR_USGFAULTENA_Msk);                                                 // Disable the usage fault interrupt.

    else if (interruptNumber == FAULT_SYSTICK)
        SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);                                                // Disable the System Tick interrupt.

     else if (interruptNumber >= 16)
        HWREG32 (g_pulDisRegs[(interruptNumber - 16) / 32]) = 1 << ((interruptNumber - 16) & 31);   // Disable the general interrupt.

}

void Int_registerInterrupt(uint_32 interruptNumber, void (*intHandler)(void))
{
    uint32_t ulIdx, ulValue;

    // See if the RAM vector table has been initialized.
    if (SCB->VTOR != (uint32_t) g_pfnRAMVectoring)
    {
        // Copy the vector table from the beginning of FLASH to the RAM vector table.
        ulValue = SCB->VTOR;
        for (ulIdx = 0; ulIdx < (NUM_INTERRUPTS + 1); ulIdx++)
            g_pfnRAMVectoring[ulIdx] = (void (*)(void)) HWREG32((ulIdx * 4) + ulValue);

        // Point the NVIC at the RAM vector table.
        SCB->VTOR = (uint32_t) g_pfnRAMVectoring;
    }

    // Save the interrupt handler.
    g_pfnRAMVectoring[interruptNumber] = intHandler;
}

void Int_unregisterInterrupt(uint_32 interruptNumber)
{
    // Reset the interrupt handler.
    g_pfnRAMVectoring[interruptNumber] = IntDefaultHandler;
}
