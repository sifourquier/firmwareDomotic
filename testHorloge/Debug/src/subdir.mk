################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/charSet.c \
../src/cr_startup_lpc175x_6x.c \
../src/crp.c \
../src/screen.c \
../src/ssp.c \
../src/testHorloge.c 

OBJS += \
./src/charSet.o \
./src/cr_startup_lpc175x_6x.o \
./src/crp.o \
./src/screen.o \
./src/ssp.o \
./src/testHorloge.o 

C_DEPS += \
./src/charSet.d \
./src/cr_startup_lpc175x_6x.d \
./src/crp.d \
./src/screen.d \
./src/ssp.d \
./src/testHorloge.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_CMSIS=CMSIS_CORE_LPC17xx -D__LPC17XX__ -D__REDLIB__ -I"/home/simon/Sync/Domotic/rs485/firmware/CMSIS_CORE_LPC17xx/inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


