################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/HorlogeDemo.c \
../src/charSet.c \
../src/cr_startup_lpc11xx.c \
../src/crp.c \
../src/pwm.c \
../src/screenFeroFluid.c \
../src/ssp.c \
../src/timer16.c \
../src/timer32.c 

OBJS += \
./src/HorlogeDemo.o \
./src/charSet.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o \
./src/pwm.o \
./src/screenFeroFluid.o \
./src/ssp.o \
./src/timer16.o \
./src/timer32.o 

C_DEPS += \
./src/HorlogeDemo.d \
./src/charSet.d \
./src/cr_startup_lpc11xx.d \
./src/crp.d \
./src/pwm.d \
./src/screenFeroFluid.d \
./src/ssp.d \
./src/timer16.d \
./src/timer32.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__REDLIB__ -I"/home/simon/Sync/Domotic/rs485/firmware/HorlogeDemo/inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


