 //FileName:        int_MSP432.h
 //Dependencies:    None.
 //Processor:       MSP432
 //Board:			MSP432P401R
 //Program version: CCS V8.3 TI
 //Company:         Texas Instruments
 //Description:     Tratamiento y sincronización de los drivers propuestos. Comentarios en ingles. Header File.
 //Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta, basado en trabajos de Texas Instruments.
 //Updated:         12/2018

#ifndef INT_MSP432_H_
#define INT_MSP432_H_

#define FAULT_NMI                                       ( 2) // NMI fault
#define FAULT_HARD                                      ( 3) // Hard fault
#define FAULT_MPU                                       ( 4) // MPU fault
#define FAULT_BUS                                       ( 5) // Bus fault
#define FAULT_USAGE                                     ( 6) // Usage fault
#define FAULT_SVCALL                                    (11) // SVCall
#define FAULT_DEBUG                                     (12) // Debug monitor
#define FAULT_PENDSV                                    (14) // PendSV
#define FAULT_SYSTICK                                   (15) // System Tick

#define INT_PSS                                         (16) // PSS IRQ.
#define INT_CS                                          (17) // CS IRQ.
#define INT_PCM                                         (18) // PCM IRQ.
#define INT_WDT_A                                       (19) // WDT_A IRQ.
#define INT_FPU                                         (20) // FPU IRQ.
#define INT_FLCTL                                       (21) // FLCTL IRQ.
#define INT_COMP_E0                                     (22) // COMP_E0 IRQ.
#define INT_COMP_E1                                     (23) // COMP_E1 IRQ.
#define INT_TA0_0                                       (24) // TA0_0 IRQ.
#define INT_TA0_N                                       (25) // TA0_N IRQ.
#define INT_TA1_0                                       (26) // TA1_0 IRQ.
#define INT_TA1_N                                       (27) // TA1_N IRQ.
#define INT_TA2_0                                       (28) // TA2_0 IRQ.
#define INT_TA2_N                                       (29) // TA2_N IRQ.
#define INT_TA3_0                                       (30) // TA3_0 IRQ.
#define INT_TA3_N                                       (31) // TA3_N IRQ.
#define INT_EUSCIA0                                     (32) // EUSCIA0 IRQ.
#define INT_EUSCIA1                                     (33) // EUSCIA1 IRQ.
#define INT_EUSCIA2                                     (34) // EUSCIA2 IRQ.
#define INT_EUSCIA3                                     (35) // EUSCIA3 IRQ.
#define INT_EUSCIB0                                     (36) // EUSCIB0 IRQ.
#define INT_EUSCIB1                                     (37) // EUSCIB1 IRQ.
#define INT_EUSCIB2                                     (38) // EUSCIB2 IRQ.
#define INT_EUSCIB3                                     (39) // EUSCIB3 IRQ.
#define INT_ADC14                                       (40) // ADC14 IRQ.
#define INT_T32_INT1                                    (41) // T32_INT1 IRQ.
#define INT_T32_INT2                                    (42) // T32_INT2 IRQ.
#define INT_T32_INTC                                    (43) // T32_INTC IRQ.
#define INT_AES256                                      (44) // AES256 IRQ.
#define INT_RTC_C                                       (45) // RTC_C IRQ.
#define INT_DMA_ERR                                     (46) // DMA_ERR IRQ.
#define INT_DMA_INT3                                    (47) // DMA_INT3 IRQ.
#define INT_DMA_INT2                                    (48) // DMA_INT2 IRQ.
#define INT_DMA_INT1                                    (49) // DMA_INT1 IRQ.
#define INT_DMA_INT0                                    (50) // DMA_INT0 IRQ.
#define INT_PORT1                                       (51) // PORT1 IRQ.
#define INT_PORT2                                       (52) // PORT2 IRQ.
#define INT_PORT3                                       (53) // PORT3 IRQ.
#define INT_PORT4                                       (54) // PORT4 IRQ.
#define INT_PORT5                                       (55) // PORT5 IRQ.
#define INT_PORT6                                       (56) // PORT6 IRQ.
#define INT_LCD_F                                       (57) // PORT6 IRQ.

#define NUM_INTERRUPTS                                  (57)

#define NVIC_EN0_R              0xE000E100                   // Interrupt 0-31 Set Enable
#define NVIC_EN1_R              0xE000E104                   // Interrupt 32-54 Set Enable
#define NVIC_DIS0_R             0xE000E180                   // Interrupt 0-31 Clear Enable
#define NVIC_DIS1_R             0xE000E184                   // Interrupt 32-54 Clear Enable

// Definición de macros.

extern uint_32 ADC_global_irq_map;
extern boolean timer_activated[2];

// Funciones definidas.

// Función que habilita una interrupción en base a un número definido en este header file.
extern void Int_enableInterrupt         (uint32_t interruptNumber);
// Función que inhabilita una interrupción en base a un número definido en este header file.
extern void Int_disableInterrupt        (uint32_t interruptNumber);
// Función que registra una función para una interrupción dada.
extern void Int_registerInterrupt       (uint_32 interruptNumber, void (*intHandler)(void));
// Función que elimina una función de una interrupción dada.
extern void Int_unregisterInterrupt     (uint_32 interruptNumber);
// Función que limpia banderas exclusivamente de GPIO.
extern void Int_clear_gpio_flags        (FILE _PTR_ file_ptr);
// Función para desactivar las interrupciones de todos los drivers de objetos momentáneamente.
extern void Int_disable                 (void);
// Función para activar las interrupciones de todos los drivers de objetos de acuerdo a si estaban inicialmente activados.
extern void Int_enable                  (void);

#endif
