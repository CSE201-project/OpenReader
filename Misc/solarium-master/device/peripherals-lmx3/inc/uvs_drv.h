/*
 * uvs_drv.h
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#ifndef UVS_DRV_H_
#define UVS_DRV_H_

#include <memory>

#include "i2c_drv.h"

#define GUVB_C31SM_I2C_ADDRESS			0x39			// UVB sensor address is 0b0111001
#define VEML6035_I2C_ADDRESS			0x29			// UVB sensor address is 0b0111001


// UV sensor control class
class Uvs : public I2cDev
{
public:
	/** Name of the PCA9624 registers (for direct register access) */
	enum Uvs_reg{
		CHIPID 		= 0x00,		/**< CHIPID register      */
		MODE		= 0x01, 	/**< MODE register      */
		RES_UV		= 0x04, 	/**< PWM0 register       */
		RANGE_UVB	= 0x07,  	/**< PWM1 register       */
		MODE_CTL 	= 0x0A, 	/**< Mode control register       */
		SOFT_RESET 	= 0x0B, 	/**< Soft Reset register       */
		UVB_LSB 	= 0x17,   	/**< PWM4 register       */
		UVB_MSB 	= 0x18,		/**< PWM5 register       */
		NVM_READ_CTL = 0x30,	/**< PWM6 register       */
		NVM_MSB 	= 0x31,  	/**< PWM7 register       */
		NVM_LSB 	= 0x32   	/**< GRPPWM register     */
	};

	enum OPER_val{
		OPER_NO_OPERATION 	= 0x00,
		OPER_UVB_OPERATION	= 0x20,
	};

	enum PMOD_val{
		PMOD_NORMAL 		= 0x00,
		PMOD_LOW_POWER		= 0x01,
		PMOD_AUTO_SHTDWN	= 0x02,
		PMOD_SHTDWN			= 0x03,
	};

	enum RES_UV_val{
		RES_UV_800MS 		= 0x00,
		RES_UV_400MS		= 0x01,
		RES_UV_200MS		= 0x02,
		RES_UV_100MS		= 0x03,
	};

	enum RANGE_UV_val{
		RANGE_UV_1x 		= 0x00,
		RANGE_UV_2x 		= 0x01,
		RANGE_UV_4x 		= 0x02,
		RANGE_UV_8x 		= 0x03,
		RANGE_UV_16x		= 0x04,
		RANGE_UV_32x		= 0x05,
		RANGE_UV_64x		= 0x06,
		RANGE_UV_128x		= 0x07,
	};

	uint8_t resUv = RES_UV_100MS;
	uint8_t rangeUv = RANGE_UV_1x;

	int SetResolution(RES_UV_val);
	int SetRange(RANGE_UV_val);

	int ReadChipId();
	int ConfigSensor();
	uint16_t ReadUVB();
};


// Visible sensor control class
class Vis : public I2cDev
{
public:

	/** Name of the PCA9624 registers (for direct register access) */
	enum Vis_reg{
		ALS_CONF_CMD = 0x00,	/**< ALS configuration register      	*/
		WH_CMD, 				/**< Illumination hi threshold register */
		WL_CMD, 				/**< Illumination low threshold register*/
		PSM_CMD, 				/**< Power Safe Mode register       	*/
		ALS_CMD, 				/**< Ambiant Light Sensor output register */
		WHITE_CMD, 				/**< White Sensor output register       */
		IF_CMD 					/**< Interrupt flag register       		*/
	};


	#define SENS_CONF_REG_OFFSET 			12
	enum SENS_val{
		HIGH_SENSITIVITY 	= 0x00,
		LOW_SENSITIVITY 	= 0x01
	};


	#define DG_CONF_REG_OFFSET 				11
	enum DG_val{
		NORMAL_SENSITIVITY 	= 0x00,
		DOUBLE_SENSITIVITY 	= 0x01
	};

	#define GAIN_CONF_REG_OFFSET 			10
	enum GAIN_val{
		DG_NORMAL 			= 0x00,
		DG_DOUBLE 			= 0x01
	};

	#define ALS_INT_TIME_CONF_REG_OFFSET 	6
	enum ALS_INT_TIME_val{
		ALS_INT_100MS 		= 0x00,
		ALS_INT_200MS 		= 0x01,
		ALS_INT_400MS 		= 0x02,
		ALS_INT_800MS 		= 0x03,
		ALS_INT_25MS 		= 0x08,
		ALS_INT_50MS 		= 0x0C,
	};


	#define CHAN_EN_CONF_REG_OFFSET 		2
	enum CHAN_EN_val{
		ALS_ONLY 			= 0x00,
		ALS_AND_WHITE 		= 0x01
	};

	enum OPER_val{
		OPER_NO_OPERATION 	= 0x00,
		OPER_UVB_OPERATION	= 0x02,
	};

	int ConfigSensor();
	uint16_t ReadAls();
	uint16_t ReadWhite(void);
};
#endif /* UVS_DRV_H_ */
