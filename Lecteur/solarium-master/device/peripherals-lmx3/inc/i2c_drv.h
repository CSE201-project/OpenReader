/*
 * i2c_drv.h
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#ifndef I2C_DRV_H_
#define I2C_DRV_H_

#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include <libdigiapix/i2c.h>

// I2c device control class
class I2cDev
{
public:
	i2c_t *i2c_bus;
	int i2c_address;

	int Init(uint8_t i2c_bus_id, uint8_t _i2c_address);
	int PointRegister(uint8_t reg_address);
	int DirectRead(uint8_t *data, uint16_t count);
	int DirectWrite(uint8_t *data, uint16_t count);
	int ReadRegisters(uint8_t reg_address, uint8_t *data, uint16_t count);
	int WriteRegisters(uint8_t reg_address, uint8_t *data, uint16_t count);
};

#define RETURN_BADRC(rc) if (rc != EXIT_SUCCESS) return rc
#define THROW_BADRC(rc) if (rc != EXIT_SUCCESS) throw std::runtime_error("I2C Error")

#endif /* I2C_DRV_H_ */
