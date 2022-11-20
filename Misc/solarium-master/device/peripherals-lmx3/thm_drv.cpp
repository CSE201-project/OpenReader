/*
 * thm_drv.cpp
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "inc/thm_drv.h"

uint8_t acquisitionReg[4];


int ThmSensor::ReadIDs(void){
	uint8_t idReg[4];

	ReadRegisters(ThmSensor::MFG_ID, idReg, 2);
	ReadRegisters(ThmSensor::DEV_ID, &idReg[2], 2);

	printf("Registers value 0x%02X%02X 0x%02X%02X\n", idReg[0], idReg[1], idReg[2], idReg[3]);

	return EXIT_SUCCESS;
}

int ThmSensor::ConfigAcquisition(void){
	uint16_t config = 0x00;
	uint8_t configReg[2];

	config |= (HDC1080_HEATER_DISABLED 		<< HDC1080_HEATER_OFFSET);
	config |= (HDC1080_TEMP_AND_HUMIDITY 	<< HDC1080_MODE_OFFSET);
	config |= (HDC1080_TRES_14_BIT 			<< HDC1080_TRES_OFFSET);
	config |= (HDC1080_HRES_14_BIT 			<< HDC1080_HRES_OFFSET);

	configReg[0] = ((config >> 8) 	& 0xFF);
	configReg[1] = (config 			& 0xFF);

	int rc = WriteRegisters(ThmSensor::CONFIG, configReg, 2);
    THROW_BADRC(rc);

	return EXIT_SUCCESS;
}

int ThmSensor::StartAcquisition(void){
	// Point to register to trigger acquisition
	int rc = PointRegister(ThmSensor::TEMPERATURE);
    THROW_BADRC(rc);
	usleep(1000);
	printf("Acquisition started \n");

	return EXIT_SUCCESS;
}

int ThmSensor::ReadResults(void){

	// Read both temperature and humidity registers
	int rc = DirectRead(&acquisitionReg[0], 4);
    THROW_BADRC(rc);

	return EXIT_SUCCESS;
}

double ThmSensor::GetTemperature(void){
	uint16_t tempValue;
	double temperature;

	// Format measurement register
	tempValue = (acquisitionReg[0]<<8) | (acquisitionReg[1]);

    //Temp(C) = reading/(2^16)*165(C) - 40(C)
	temperature = (double)(tempValue)/(65536)*165-40;
	printf("Temperature measurement is %.2lf Deg C\n", temperature);

	return temperature;
}


double ThmSensor::GetHumidity(void){
	uint16_t humidityValue;
	double humidity;

	// Format measurement register
	humidityValue = (acquisitionReg[2]<<8) | (acquisitionReg[3]);

    //Humidity(%) = reading/(2^16)*100%
	humidity = (double)(humidityValue)/(65536)*100;
	printf("Humidity measurement is %.2lf %% \n", humidity);

	return humidity;
}

