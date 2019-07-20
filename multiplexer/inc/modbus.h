#ifndef MODBUS_H_
#define MODBUS_H_

#include "stdint.h"

#define ADDRESS_LENGTH 1
#define MAX_PAQUET_SIZE 1040
#define BAUDDRATE 115200
#define CRC_INIT_VALUE 0xFFFF

typedef enum
{
	F_READ_COIL=1, //1 COIL = on/off
	F_READ_INPUT=2, //1 AN = 16bit
	F_READ_REG=3, //x COIL = on/off
	F_READ_INPUT_REG=4, //x AN = 16bit
	F_WRITE_COIL=5,
	F_WRITE_REG=6,
	F_WRITE_COILS=15,
	F_WRITE_REGS=16,
} t_Function;

typedef struct {
	uint16_t address;
	t_Function function;
	uint32_t length;
	uint8_t data[MAX_PAQUET_SIZE];
} t_paquet;

typedef void (*modbus_write_cb_t)(t_Function function,uint32_t address,uint32_t value);
typedef int (*modbus_read_cb_t)(t_Function function,uint32_t address,uint8_t * data);

void modeBusInit();
void modBusSetCallbackWrite(modbus_write_cb_t l_callBackWrite);
void modBusSetCallbackRead(modbus_read_cb_t l_callBackRead);
void modeBusCrc( uint8_t *input_str, uint32_t num_bytes, uint16_t *crc);
void modeBusCompute();

extern volatile uint32_t modeBusaddress;

#endif
