#include "bme280.h"
#include "bme280_defs.h"

/**
 * @file bme280_user.c
 * \brief bme280 abstraction 
 *
 * user funbction for bme280 sensor
 *
 * \author modified by Simon Fourquier
 *
 * Contact: simon.fourquier@hesge.ch
 *
 */

extern uint32_t SystemCoreClock;


#define CS GPIO_OUTPUT_SET(4, 0);
#define nCS GPIO_OUTPUT_SET(4, 1);

struct bme280_dev dev;


#define MASK_DATA1	0xFF
#define MASK_DATA2	0x80
#define MASK_DATA3	0x7F


/************** I2C/SPI buffer length ******/
#define I2C_BUFFER_LEN 64

/*-------------------------------------------------------------------*
*	This is the for read and write the data by using I2C
*	The device address defined in the bme280.h file
*-----------------------------------------------------------------------*/
 /**	\Brief The function is used as I2C bus write
 *	\Return Status of the I2C write
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */
int8_t BME280_I2C_bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t cnt)
{
	int32_t iError = 0;
	uint8_t array[I2C_BUFFER_LEN];
	uint8_t stringpos = 0;
	array[0] = reg_addr;
	for (stringpos = 0; stringpos < cnt; stringpos++) {
		array[stringpos + 1] = *(reg_data + stringpos);
	}
	
	i2cWriteRead(dev_addr,array,cnt+1,0,0);
	/*
	* Please take the below function as your reference for
	* write the data using I2C communication
	* "IERROR = I2C_WRITE_STRING(DEV_ADDR, ARRAY, CNT+1)"
	* add your I2C write function here
	* iError is an return value of I2C read function
	* Please select your valid return value
	* In the driver SUCCESS defined as 0
    * and FAILURE defined as -1
	* Note :
	* This is a full duplex operation,
	* The first read data is discarded, for that extra write operation
	* have to be initiated. For that cnt+1 operation done in the I2C write string function
	* For more information please refer data sheet SPI communication:
	*/
	return (int8_t)iError;
}

 /**	\Brief The function is used as I2C bus read
 *	\Return Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of data byte of to be read
 */
int8_t BME280_I2C_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t cnt)
{
	int32_t iError = 0;
	uint8_t array[I2C_BUFFER_LEN] = {0};
	uint8_t stringpos = 0;
	array[0] = reg_addr;
		
	i2cWriteRead(dev_addr,0,0,array,cnt);
	
	/* Please take the below function as your reference
	 * for read the data using I2C communication
	 * add your I2C rad function here.
	 * "IERROR = I2C_WRITE_READ_STRING(DEV_ADDR, ARRAY, ARRAY, 1, CNT)"
	 * iError is an return value of SPI write function
	 * Please select your valid return value
     * In the driver SUCCESS defined as 0
     * and FAILURE defined as -1
	 */
	for (stringpos = 0; stringpos < cnt; stringpos++) {
		*(reg_data + stringpos) = array[stringpos];
	}
	return (int8_t)iError;
}

/**	\Brief The delay routine
 *	\param delay in ms
*/
void BME280_delay_msek(uint32_t msek)
{
	for(volatile int l;l<SystemCoreClock/2300;l++);//~1ms
}

 /** \Brief The function is used for init BME280
 */
void bme280InitUser(void)
{
    uint8_t settings_sel;
	
	dev.dev_id = BME280_I2C_ADDR_SEC;
	dev.intf = BME280_I2C_INTF;
	dev.read = BME280_I2C_bus_read;
	dev.write = BME280_I2C_bus_write;
	dev.delay_ms = BME280_delay_msek;

	bme280_init(&dev);
	

    /* Recommended mode of operation: Indoor navigation */
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    bme280_set_sensor_settings(settings_sel, &dev);
	
	bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
}

 /**\Brief The function is used for read BME280
 *	\Return Value of bme280
 */
struct bme280_data bme280GetSensorDataUser()
{
	struct bme280_data comp_data;
	bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
	return comp_data;
}
