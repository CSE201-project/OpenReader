/*
 * lp5562_drv.h
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#ifndef IHM_DRV_H_
#define IHM_DRV_H_

#include <memory>

#include <libdigiapix/i2c.h>

#include "i2c_drv.h"

#define DEFAULT_I2C_ADDRESS_1		0x30
#define DEFAULT_I2C_ADDRESS_2		0x32

#define LP5562_PROGRAM_LENGTH     32
#define LP5562_MAX_LEDS            4


/* ENABLE Register 00h */
#define LP5562_EXEC_ENG1_M      0x30
#define LP5562_EXEC_ENG2_M      0x0C
#define LP5562_EXEC_ENG3_M      0x03
#define LP5562_EXEC_M           0x3F
#define LP5562_MASTER_ENABLE    0x40    /* Chip master enable */
#define LP5562_LOGARITHMIC_PWM  0x80    /* Logarithmic PWM adjustment */
#define LP5562_EXEC_RUN         0x2A
#define LP5562_ENABLE_DEFAULT   \
    (LP5562_MASTER_ENABLE | LP5562_LOGARITHMIC_PWM)
#define LP5562_ENABLE_RUN_PROGRAM   \
    (LP5562_ENABLE_DEFAULT | LP5562_EXEC_RUN)

/* OPMODE Register 01h */
#define LP5562_MODE_ENG1_M      0x30
#define LP5562_MODE_ENG2_M      0x0C
#define LP5562_MODE_ENG3_M      0x03
#define LP5562_LOAD_ENG1        0x10
#define LP5562_LOAD_ENG2        0x04
#define LP5562_LOAD_ENG3        0x01
#define LP5562_RUN_ENG1         0x20
#define LP5562_RUN_ENG2         0x08
#define LP5562_RUN_ENG3         0x02
#define LP5562_ENG1_IS_LOADING(mode)    \
    ((mode & LP5562_MODE_ENG1_M) == LP5562_LOAD_ENG1)
#define LP5562_ENG2_IS_LOADING(mode)    \
    ((mode & LP5562_MODE_ENG2_M) == LP5562_LOAD_ENG2)
#define LP5562_ENG3_IS_LOADING(mode)    \
    ((mode & LP5562_MODE_ENG3_M) == LP5562_LOAD_ENG3)


/* CONFIG Register 08h */
#define LP5562_PWM_HF           0x40
#define LP5562_PWRSAVE_EN       0x20
#define LP5562_CLK_INT          0x01    /* Internal clock */
#define LP5562_DEFAULT_CFG      (LP5562_CLK_INT | LP5562_PWRSAVE_EN)

/* RESET Register 0Dh */
#define LP5562_RESET            0xFF


/* LEDMAP Register 70h */
#define LP5562_ENG_SEL_PWM      0
#define LP5562_ENG_FOR_RGB_M    0x3F
#define LP5562_ENG_SEL_RGB      0x1B    /* R:ENG1, G:ENG2, B:ENG3 */
#define LP5562_ENG_FOR_W_M      0xC0
#define LP5562_ENG1_FOR_W       0x40    /* W:ENG1 */
#define LP5562_ENG2_FOR_W       0x80    /* W:ENG2 */
#define LP5562_ENG3_FOR_W       0xC0    /* W:ENG3 */


#define LP5562_SEL_WT_OFFSET    6       /* White offset */
#define LP5562_SEL_RD_OFFSET    4       /* Red offset   */
#define LP5562_SEL_GR_OFFSET    2       /* Green offset */
#define LP5562_SEL_BL_OFFSET    0       /* Blue offset  */

#define LP5562_ENG1_SEL         0x01    /* ENG1 */
#define LP5562_ENG2_SEL         0x02    /* ENG2 */
#define LP5562_ENG3_SEL         0x03    /* ENG3 */


/* Program Commands */
#define LP5562_CMD_DISABLE      0x00
#define LP5562_CMD_LOAD         0x15
#define LP5562_CMD_RUN          0x2A
#define LP5562_CMD_DIRECT       0x3F
#define LP5562_PATTERN_OFF      0


class Lp5562 : public I2cDev
{
public:

	/** Name of the PCA9624 registers (for direct register access) */
	enum Lp5562_reg{
		ENABLE = 0x00,	/**< ENABLE Register 00h      */
		OP_MODE,      		/**< MODE2 register      */

		B_PWM,       		/**< B PWM register       */
		G_PWM,       		/**< G PWM register       */
		R_PWM,       		/**< R PWM register       */

		B_CURRENT,       	/**< B current register       */
		G_CURRENT,       	/**< G current register       */
		R_CURRENT,       	/**< R current register       */

		CONFIG,       		/**< Config register       */

		RESET = 0x0D,     	/**< Reset register       */
		W_PWM = 0x0E,     	/**< White current register */
		W_CURRENT = 0x0F,	/**< White current register */

		PROG_MEM_ENG1 = 0x10,
		PROG_MEM_ENG2 = 0x30,
		PROG_MEM_ENG3 = 0x50,

		ENG_SEL = 0x70,
	};

	/*
	 * Led channel offset .
	 */
	enum LedChannel{
	    B_LED = 0,
		G_LED,
		R_LED,
		W_LED
	};

	/*
	 * Led modes define the type of led activity to reflect the state of the instrument.
	 */
	enum Led_mode{
	    BLINK_RED = 0,
	    BLINK_YELLOW,
	    BLINK_VIOLET,
	    BLINK_WHITE,
	    PULSE_GREEN,
	    PULSE_BLUE,
	    PULSE_WHITE,
	    BT_SEARCH,          // Bluetooth search ()
	    PULSE_1,          	// Pulse sequence feedback
	    FIRMWARE_SEQ,       // Firmware update sequence
	    ERROR_SEQ           // Error message
	};

	uint8_t ledIntensity[4];


	void SetLed(Lp5562::LedChannel channel, uint8_t intensity);
	void LoadIntensities();
	void LoadEngines(Led_mode mode);
	void StopEngines(void);

};


class IhmController
{
public:
	/*
	 * Led modes define the type of led activity to reflect the state of the instrument.
	 */
	enum Ihm_mode{
	    BLINK_RED = 0,
	    BLINK_YELLOW,
	    BLINK_VIOLET,
	    BLINK_WHITE,
	    PULSE_GREEN,
	    PULSE_BLUE,
	    PULSE_WHITE,
	    BT_SEARCH,          // Bluetooth search ()
	    PULSE_1,          	// Pulse sequence feedback
	    FIRMWARE_SEQ,       // Firmware update sequence
	    ERROR_SEQ           // Error message
	};

	Lp5562 		lpDriver; 	// Underlying LP5562 driver

	int SetMode(IhmController::Ihm_mode);
};


#endif /* IHM_DRV_H_ */
