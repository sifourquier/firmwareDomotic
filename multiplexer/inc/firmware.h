#ifndef FIRMWARE_H_
#define FIRMWARE_H_

#define MOD_BUS_ADDRESS_FIRMWARE 65535

void firmwareFlash(volatile uint16_t crc);
void firmwareDecode(uint8_t * data,uint16_t nbByte);

#endif
