#ifndef BME280_USER_H
#define BME280_USER_H

/**
 * @file bme280_user.h
 * \brief bme280 abstraction 
 *
 * user funbction for bme280 sensor
 *
 * \author modified by Simon Fourquier
 *
 * Contact: simon.fourquier@hesge.ch
 *
 */
#include "bme280_defs.h"

void bme280InitUser(void);
struct bme280_data bme280GetSensorDataUser();

#endif /*BME280_USER_H*/
