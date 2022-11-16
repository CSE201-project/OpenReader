/*
 * uvs_drv.cpp
 *
 * UV and Visible Sensor Control
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/uvs_drv.h"


#define LED_MAXIMUM_INTENSITY 		90

int Uvs::ReadChipId(void){
	uint8_t chipId;

	// Read chip ID
	PointRegister(CHIPID);
	DirectRead(&chipId, 1);

	//printf("The target CHIPID is 0x%02X \n", chipId);

	return EXIT_FAILURE;
}

int Uvs::SetResolution(Uvs::RES_UV_val resolution){
	resUv = resolution;
	return EXIT_SUCCESS;
}
int Uvs::SetRange(Uvs::RANGE_UV_val range){
	rangeUv = range;
	return EXIT_SUCCESS;
}

int Uvs::ConfigSensor(void){	// Read chip ID
	uint8_t mode;
    int rc;

	mode = OPER_UVB_OPERATION | PMOD_NORMAL;
	rc = WriteRegisters(MODE, &mode, 1);
    THROW_BADRC(rc);

	WriteRegisters(RES_UV, &resUv, 1);
    THROW_BADRC(rc);

	WriteRegisters(RANGE_UVB, &rangeUv, 1);
    THROW_BADRC(rc);

	return EXIT_SUCCESS;
}

uint16_t Uvs::ReadUVB(void){
	uint16_t uvbValue;
	uint8_t uvbReg[2];
	int rc;

	// Read UVB
	rc= PointRegister(UVB_LSB);
    THROW_BADRC(rc);

	rc= DirectRead(&uvbReg[0], 2);
    THROW_BADRC(rc);
	uvbValue = (uvbReg[1]<<8) | (uvbReg[0]);

	//printf("UVB measurement 0x%04X \n", uvbValue);

	return uvbValue;
}


// ================ Visible sensor configuration ================ //

int Vis::ConfigSensor(void){	// Read chip ID
	uint8_t regValue[2];
	int rc;

	regValue[0] = 	ALS_AND_WHITE << CHAN_EN_CONF_REG_OFFSET;

	regValue[1] = 	(HIGH_SENSITIVITY<<SENS_CONF_REG_OFFSET) |
					(DG_NORMAL<<GAIN_CONF_REG_OFFSET) |
					(NORMAL_SENSITIVITY<<GAIN_CONF_REG_OFFSET) |
					(ALS_INT_100MS<<ALS_INT_TIME_CONF_REG_OFFSET);
	rc = WriteRegisters(ALS_CONF_CMD, &regValue[0], 2);
    THROW_BADRC(rc);

	regValue[0] = 0x00;
	rc = WriteRegisters(PSM_CMD, &regValue[0], 1);
    THROW_BADRC(rc);

	return EXIT_SUCCESS;
}

uint16_t Vis::ReadAls(void){
	uint16_t alsValue;
	uint8_t uvbReg[2];

	// Read chip ID
	int rc = ReadRegisters(ALS_CMD, &uvbReg[0], 2);
    THROW_BADRC(rc);
	alsValue = (uvbReg[1]<<8) | (uvbReg[0]);

	//printf("ALS measurement 0x%04X \n", alsValue);

	return alsValue;
}

uint16_t Vis::ReadWhite(void){
	uint16_t whiteValue;
	uint8_t whiteReg[2];

	// Read chip ID
	int rc = ReadRegisters(WHITE_CMD, &whiteReg[0], 2);
    THROW_BADRC(rc);
	whiteValue = (whiteReg[1]<<8) | (whiteReg[0]);

	//printf("White measurement 0x%04X \n", whiteReg);

	return whiteValue;
}

