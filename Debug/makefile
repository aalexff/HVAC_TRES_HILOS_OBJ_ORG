################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS

GEN_OPTS__FLAG := --cmd_file="configPkg/compiler.opt" 
GEN_CMDS__FLAG := -l"configPkg/linker.cmd" 

ORDERED_OBJS += \
"./HVAC_IO.obj" \
"./HVAC_Threads.obj" \
"./Threads.obj" \
"./adc.obj" \
"./gpio.obj" \
"./timer.obj" \
"./Aux_files/system_msp432p401r.obj" \
"./Drivers_obj/Files.obj" \
"./Drivers_obj/adc_f_MSP432.obj" \
"./Drivers_obj/gpio_f_MSP432.obj" \
"./Drivers_obj/int_MSP432.obj" \
"./Drivers_obj/timer_f_msp432.obj" \
"./Drivers_obj/uart_f_MSP432.obj" \
"../Aux_files/MSP_EXP432P401R_TIRTOS.cmd" \
-l"ti/display/lib/display.aem4f" \
-l"ti/grlib/lib/ccs/m4f/grlib.a" \
-l"third_party/spiffs/lib/ccs/m4f/spiffs.a" \
-l"ti/drivers/lib/drivers_msp432p401x.aem4f" \
-l"third_party/fatfs/lib/ccs/m4f/fatfs.a" \
-l"ti/dpl/lib/dpl_msp432p401x.aem4f" \
$(GEN_CMDS__FLAG) \
-l"ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Aux_files/subdir_vars.mk
-include Drivers_obj/subdir_vars.mk
-include subdir_rules.mk
-include Aux_files/subdir_rules.mk
-include Drivers_obj/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
HVAC_TRES_HILOS_OBJ_ORG.out \

EXE_OUTPUTS__QUOTED += \
"HVAC_TRES_HILOS_OBJ_ORG.out" \

BIN_OUTPUTS += \
HVAC_TRES_HILOS_OBJ_ORG.hex \

BIN_OUTPUTS__QUOTED += \
"HVAC_TRES_HILOS_OBJ_ORG.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "HVAC_TRES_HILOS_OBJ_ORG.out"

# Tool invocations
HVAC_TRES_HILOS_OBJ_ORG.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --advice:power=none -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"HVAC_TRES_HILOS_OBJ_ORG.map" -i"C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source" -i"C:/ti/simplelink_msp432p4_sdk_3_40_01_02/kernel/tirtos/packages" -i"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/lib" --diag_wrap=off --display_error_number --warn_sections --xml_link_info="HVAC_TRES_HILOS_OBJ_ORG_linkInfo.xml" --rom_model -o "HVAC_TRES_HILOS_OBJ_ORG.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

HVAC_TRES_HILOS_OBJ_ORG.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armhex" --diag_wrap=off -o "HVAC_TRES_HILOS_OBJ_ORG.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "HVAC_IO.obj" "HVAC_Threads.obj" "Threads.obj" "adc.obj" "gpio.obj" "timer.obj" "Aux_files\system_msp432p401r.obj" "Drivers_obj\Files.obj" "Drivers_obj\adc_f_MSP432.obj" "Drivers_obj\gpio_f_MSP432.obj" "Drivers_obj\int_MSP432.obj" "Drivers_obj\timer_f_msp432.obj" "Drivers_obj\uart_f_MSP432.obj" 
	-$(RM) "HVAC_IO.d" "HVAC_Threads.d" "Threads.d" "adc.d" "gpio.d" "timer.d" "Aux_files\system_msp432p401r.d" "Drivers_obj\Files.d" "Drivers_obj\adc_f_MSP432.d" "Drivers_obj\gpio_f_MSP432.d" "Drivers_obj\int_MSP432.d" "Drivers_obj\timer_f_msp432.d" "Drivers_obj\uart_f_MSP432.d" 
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

