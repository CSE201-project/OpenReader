/*
 * i2c_drv.cpp
 *
 * I2C Device Control
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#include "inc/i2c_drv.h"

int I2cDev::Init(uint8_t i2c_bus_id, uint8_t _i2c_address)
{
	bool found = false;
	int bus_number, i;
	uint8_t *buses = NULL;

	/* Test retrieved I2C device handle */
	if (i2c_bus_id < 0) {
		printf("I2C bus index must be 0 or greater\n");
		return EXIT_FAILURE;
	}

	/* Retrieve the list of available I2C buses */
	bus_number = ldx_i2c_list_available_buses(&buses);
	if (bus_number > 0) {
		found = true;
		for (i = 0; i < bus_number; i++) {
			//printf("I2C buss %i \n", buses[i]);
	        	//if(buses[i] == i2c_bus_id)
	        		//found = true;
		}
	} else {
		printf("The target does not have any I2C bus available\n");
		return EXIT_FAILURE;
	}

	if(!found){
			printf("I2C bus not available on target\n");
			return EXIT_FAILURE;
	}

	/* Request I2C */
	i2c_bus = ldx_i2c_request((unsigned int)i2c_bus_id);
	if (!i2c_bus) {
		printf("Failed to initialize I2C\n");
		return EXIT_FAILURE;
	}

	//printf("I2C bus acquired\n");
	i2c_address = _i2c_address;
	return EXIT_SUCCESS;
}


int I2cDev::PointRegister(uint8_t reg_address){
	if (ldx_i2c_write(i2c_bus, i2c_address, &reg_address, 1) != EXIT_SUCCESS) {
		printf("Error: Data written failed.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int I2cDev::ReadRegisters(uint8_t reg_address, uint8_t *data, uint16_t count)
{
	uint8_t write_data;

	/* Create write buffer */
	write_data = reg_address;

	if(ldx_i2c_transfer(i2c_bus, i2c_address, &write_data, 1, data, count)!= EXIT_SUCCESS) {
		printf("Error: Register read failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int I2cDev::DirectRead(uint8_t *data, uint16_t count)
{
	if(ldx_i2c_read(i2c_bus, i2c_address, data, count)!= EXIT_SUCCESS) {
		printf("Error: Register read failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int I2cDev::DirectWrite(uint8_t *data, uint16_t count)
{
	if(ldx_i2c_write(i2c_bus, i2c_address, data, count)!= EXIT_SUCCESS) {
		printf("Error: Register read failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int I2cDev::WriteRegisters(uint8_t reg_address, uint8_t *data, uint16_t count)
{
	int i=0;
	uint8_t *write_data;

	/* Create write buffer */
	write_data = (uint8_t *)calloc(1 + count, sizeof(uint8_t));
	if (write_data == NULL) {
		printf("Error: allocating i2c transfer buffer\n");
		return EXIT_FAILURE;
	}
	write_data[0] = reg_address;
	for(; i<count; i++)
		write_data[1+i] = data[i];

	if (ldx_i2c_write(i2c_bus, i2c_address, write_data, (uint16_t)(1 + count)) != EXIT_SUCCESS) {
		printf("Error: Data written failed.\n");
		free(write_data);
		return EXIT_FAILURE;
	}

	free(write_data);
	return EXIT_SUCCESS;
}

