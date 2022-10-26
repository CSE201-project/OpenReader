/*
 * tmp_drv.h
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#ifndef THM_DRV_H_
#define THM_DRV_H_

#include <memory>

#include "i2c_drv.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/


/** 7-bit I2C addresses of Temperature Sensors */
#define HDC1080_I2C_ADDRESS			0x40			// HDC1080 I2C address

#define HDC1080_RST_SOFT_OFFSET		15
#define HDC1080_RST_SOFT_RESET 		0x01

#define HDC1080_HEATER_OFFSET		13
#define HDC1080_HEATER_DISABLED 	0x00
#define HDC1080_HEATER_ENABLED 		0x01

#define HDC1080_MODE_OFFSET			12
#define HDC1080_TEMP_OR_HUMIDITY 	0x00
#define HDC1080_TEMP_AND_HUMIDITY 	0x01

#define HDC1080_BTST_OFFSET			10		// Read only
#define HDC1080_BTST_SUP_2V8 		0x00
#define HDC1080_BTST_INF_2V8 		0x01

#define HDC1080_TRES_OFFSET			10
#define HDC1080_TRES_14_BIT 		0x00
#define HDC1080_TRES_11_BIT 		0x01

#define HDC1080_HRES_OFFSET			8
#define HDC1080_HRES_14_BIT 		0x00
#define HDC1080_HRES_11_BIT 		0x01
#define HDC1080_HRES_08_BIT 		0x10

// Temperature and humidity sensor control class
class ThmSensor : public I2cDev
{
public:
	/** HDC1080 sensor registers */
	enum Hdc1080_reg{
		TEMPERATURE	= 0x00,		/**< Temperature reading register      */
		HUMIDITY 	= 0x01,		/**< Humidity reading register      */
		CONFIG 		= 0x02,		/**< Configuration register      */
		SERIAL_ID1	= 0xFB, 	/**< Serial ID first 2 bytes register 	*/
		SERIAL_ID2	= 0xFC, 	/**< Serial ID mid 2 bytes register 	*/
		SERIAL_ID3	= 0xFD,  	/**< Serial ID last byte register 		*/
		MFG_ID 		= 0xFE, 	/**< Manufacturer ID register   */
		DEV_ID 		= 0xFF, 	/**< Device ID register       	*/
	};

	int 		ReadIDs(void);
	int 		ConfigAcquisition(void);
	int 		StartAcquisition(void);
	int 		ReadResults(void);

	double 		GetTemperature(void);
	double 		GetHumidity(void);
};

#endif /* THM_DRV_H_ */
