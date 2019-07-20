################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DS18B20.c \
../src/I2C.c \
../src/TLI4970.c \
../src/cr_startup_lpc11xx.c \
../src/crp.c \
../src/firmware.c \
../src/gpio.c \
../src/iap_driver.c \
../src/main.c \
../src/modbus.c \
../src/oneWire.c \
../src/pwm.c \
../src/ssp.c \
../src/timer16.c \
../src/timer32.c \
../src/triac.c \
../src/uart.c 

OBJS += \
./src/DS18B20.o \
./src/I2C.o \
./src/TLI4970.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o \
./src/firmware.o \
./src/gpio.o \
./src/iap_driver.o \
./src/main.o \
./src/modbus.o \
./src/oneWire.o \
./src/pwm.o \
./src/ssp.o \
./src/timer16.o \
./src/timer32.o \
./src/triac.o \
./src/uart.o 

C_DEPS += \
./src/DS18B20.d \
./src/I2C.d \
./src/TLI4970.d \
./src/cr_startup_lpc11xx.d \
./src/crp.d \
./src/firmware.d \
./src/gpio.d \
./src/iap_driver.d \
./src/main.d \
./src/modbus.d \
./src/oneWire.d \
./src/pwm.d \
./src/ssp.d \
./src/timer16.d \
./src/timer32.d \
./src/triac.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -DCR_INTEGER_PRINTF -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__REDLIB__ -I"/home/simon/Sync/Domotic/rs485/firmware/multiplexer/inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


