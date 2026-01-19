################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
libraries/display/Crystalfontz128x128_ST7735.obj: C:/Users/ryanm/Documents/WPI/libraries/display/Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/ryanm/Documents/WPI/ECE_Software/CodeComposer3849/lab_0_Workspace/ece3849_Lab4_rmartinkat_raguilar" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295/utils" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/Users/ryanm/Documents/WPI/ECE_Software/CodeComposer3849/lab_0_Workspace/FreeRTOS/FreeRTOS/FreeRTOS/include" --include_path="C:/Users/ryanm/Documents/WPI/ECE_Software/CodeComposer3849/lab_0_Workspace/FreeRTOS/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F" --include_path="C:/Users/ryanm/Documents/WPI/libraries/HAL_TM4C1294" --include_path="C:/Users/ryanm/Documents/WPI/libraries/OPT3001" --include_path="C:/Users/ryanm/Documents/WPI/libraries/buttonsDriver" --include_path="C:/Users/ryanm/Documents/WPI/libraries/display" --include_path="C:/Users/ryanm/Documents/WPI/libraries/elapsedTime" --include_path="C:/Users/ryanm/Documents/WPI/libraries/joystickDriver" --include_path="C:/Users/ryanm/Documents/WPI/libraries" --include_path="C:/Users/ryanm/Documents/WPI/libraries/timerLib" --include_path="C:/Users/ryanm/Downloads/snake_base_lab3" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="libraries/display/$(basename $(<F)).d_raw" --obj_directory="libraries/display" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

libraries/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.obj: C:/Users/ryanm/Documents/WPI/libraries/display/HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/ryanm/Documents/WPI/ECE_Software/CodeComposer3849/lab_0_Workspace/ece3849_Lab4_rmartinkat_raguilar" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295/utils" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/Users/ryanm/Documents/WPI/ECE_Software/CodeComposer3849/lab_0_Workspace/FreeRTOS/FreeRTOS/FreeRTOS/include" --include_path="C:/Users/ryanm/Documents/WPI/ECE_Software/CodeComposer3849/lab_0_Workspace/FreeRTOS/FreeRTOS/FreeRTOS/portable/CCS/ARM_CM4F" --include_path="C:/Users/ryanm/Documents/WPI/libraries/HAL_TM4C1294" --include_path="C:/Users/ryanm/Documents/WPI/libraries/OPT3001" --include_path="C:/Users/ryanm/Documents/WPI/libraries/buttonsDriver" --include_path="C:/Users/ryanm/Documents/WPI/libraries/display" --include_path="C:/Users/ryanm/Documents/WPI/libraries/elapsedTime" --include_path="C:/Users/ryanm/Documents/WPI/libraries/joystickDriver" --include_path="C:/Users/ryanm/Documents/WPI/libraries" --include_path="C:/Users/ryanm/Documents/WPI/libraries/timerLib" --include_path="C:/Users/ryanm/Downloads/snake_base_lab3" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="libraries/display/$(basename $(<F)).d_raw" --obj_directory="libraries/display" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


