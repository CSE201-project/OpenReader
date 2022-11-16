/*
 * pca9624_drv.h
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#ifndef PCA9624_DRV_H_
#define PCA9624_DRV_H_


#include <stdbool.h>
#include <memory>

#include <libdigiapix/gpio.h>
#include <libdigiapix/i2c.h>

#include "i2c_drv.h"

class Pca9624 : public I2cDev
{
public:

	/** Name of the PCA9624 registers (for direct register access) */
	enum Pca9624_reg{
		MODE1,      /**< MODE1 register      */
		MODE2,      /**< MODE2 register      */
		PWM0,       /**< PWM0 register       */
		PWM1,       /**< PWM1 register       */
		PWM2,       /**< PWM2 register       */
		PWM3,       /**< PWM3 register       */
		PWM4,       /**< PWM4 register       */
		PWM5,       /**< PWM5 register       */
		PWM6,       /**< PWM6 register       */
		PWM7,       /**< PWM7 register       */
		GRPPWM,     /**< GRPPWM register     */
		GRPFREQ,    /**< GRPFREQ register    */
		LEDOUT0,    /**< LEDOUT0 register    */
		LEDOUT1,    /**< LEDOUT1 register    */
		SUBADR1,    /**< SUBADR1 register    */
		SUBADR2,    /**< SUBADR2 register    */
		SUBADR3,    /**< SUBADR3 register    */
		ALLCALLADR, /**< ALLCALLADR register */

		REGISTER_START          = MODE1,
		LEDOUT_REGISTER_START   = LEDOUT0,
		PWM_REGISTER_START      = PWM0,
	};

	gpio_t *en_gpio; 	// GPIO controlling PCA enable

	int GpioInit(const char *);
	int Enable(void);
	int Disable(void);
};


class Led_driver
{
public:
	enum Led_channel{
		LED_CHAN0,   	/**< Channel 0      			*/
		LED_CHAN1,   	/**< CHannel 1					*/
		LED_CHAN2,   	/**< LED mode individual PWM   	*/
		LED_CHAN3,   	/**< LED mode individual PWM   	*/
		LED_CHAN4,   	/**< LED mode individual PWM   	*/
		LED_CHAN5,   	/**< LED mode individual PWM   	*/
		LED_CHAN6,   	/**< LED mode individual PWM   	*/
		LED_CHAN7,   	/**< LED mode Group PWM      	*/
		LED_CHAN_COUNT
	};

	enum Led_brightness{
		LED_OFF,   		/**< LED mode OFF      			*/
		LED_ON,   		/**< LED mode Full Brightness	*/
		LED_IND_PWM,   	/**< LED mode individual PWM   	*/
		LED_GRP_PWM   	/**< LED mode Group PWM      	*/
	};

#define LED_MAXIMUM_BRIGHTNESS 		0xD0

	uint8_t  ledMaximumBrightness = LED_MAXIMUM_BRIGHTNESS;

	Pca9624 	pcaDriver; 		// Underlying PCA9624

	uint8_t		led_state[8];
	uint8_t 	led_pwm[8];

	int Init(const char *, uint8_t i2c_bus_id, uint8_t _i2c_address);
	int Enable(void);
	int Disable(void);
	int SetState(Led_driver::Led_channel, Led_driver::Led_brightness);
	int SetIntensity(Led_driver::Led_channel, uint8_t);
	int Refresh(void);
	int Blink(uint32_t);
	int FullBrightness(void);
	int Off(void);
};


#endif /* PCA9624_DRV_H_ */
