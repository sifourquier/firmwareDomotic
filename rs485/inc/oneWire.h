#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#define ONE_WIRE_SKIP_ROM 0xCC

#define MAX_LENGTH_1_WIRE 10

typedef struct
{
	int16_t temperature;
	uint16_t humidity;
} t_DHT22Data;

void initOneWire();
int SendReadOneWire(uint8_t* dataWr,uint8_t lengthWr,uint8_t lengthRd);
int getDataOneWire(uint8_t ** buff);
int startDHT22();
int getDataDHT22(t_DHT22Data* data);

#endif
