################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DS18B20.c \
../src/I2C.c \
../src/TLI4970.c \
../src/bme280.c \
../src/bme280_user.c \
../src/cr_startup_lpc11xx.c \
../src/crp.c \
../src/firmware.c \
../src/gpio.c \
../src/iap_driver.c \
../src/lcd_thermostat.c \
../src/main.c \
../src/modbus.c \
../src/oneWire.c \
../src/pwm.c \
../src/sht30.c \
../src/ssp.c \
../src/timer16.c \
../src/timer32.c \
../src/triac.c \
../src/uart.c 

OBJS += \
./src/DS18B20.o \
./src/I2C.o \
./src/TLI4970.o \
./src/bme280.o \
./src/bme280_user.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o \
./src/firmware.o \
./src/gpio.o \
./src/iap_driver.o \
./src/lcd_thermostat.o \
./src/main.o \
./src/modbus.o \
./src/oneWire.o \
./src/pwm.o \
./src/sht30.o \
./src/ssp.o \
./src/timer16.o \
./src/timer32.o \
./src/triac.o \
./src/uart.o 

C_DEPS += \
./src/DS18B20.d \
./src/I2C.d \
./src/TLI4970.d \
./src/bme280.d \
./src/bme280_user.d \
./src/cr_startup_lpc11xx.d \
./src/crp.d \
./src/firmware.d \
./src/gpio.d \
./src/iap_driver.d \
./src/lcd_thermostat.d \
./src/main.d \
./src/modbus.d \
./src/oneWire.d \
./src/pwm.d \
./src/sht30.d \
./src/ssp.d \
./src/timer16.d \
./src/timer32.d \
./src/triac.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -DSHT30 -Dn_DHT22 -DLED_12V -Dn_TRIACK -Dn_DS18B20 -Dn_TLI4970 -DTHERMOSTAT_LCD -Dn_LED_SWITCH -DCR_INTEGER_PRINTF -D__CODE_RED -DCORE_M0 -D__LPC11XX__ -D__REDLIB__ -I"/home/simon/Sync/Domotic/rs485/firmware/rs485/inc" -Og -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


