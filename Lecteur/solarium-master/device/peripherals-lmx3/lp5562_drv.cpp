/*
 * pca9624_drv.cpp
 *
 *  Created on: Feb 15, 2021
 *      Author: john
 */

#include "inc/lp5562_drv.h"

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define DEFAULT_I2C_ALIAS			"LEDS_I2C_BUS"
#define LED_MAXIMUM_INTENSITY 		90

/*
 * Pulse sequence provides a pulsation pattern for various led activity:
 * Battery low indication (yellow = green + red) -> pulsation only
 * Firmware update (violet = red + blue) -> for firmware update progression
 * Error reporting (red) -> with pulsation only
 *
 * Pulsation sequence only relies on a single engine as all active leds share
 * the same activation profile. -> we only configure Engine 1 in this mode
 *
 */
uint16_t fade_sequence[] = {
        0x0A00,     // Engine 1
        0xE00C,
        0x057E,
        0x05FE,
        0x5A00,
        0xE300,

        0xE080,     // Engine 2
        0x4400,
        0x047E,
        0x05FE,
        0x5A00,
        0xE002,

        0xE080,     // Engine 3
        0x4900,
        0x037E,
        0x05FE,
        0x5A00,
        0xE002
};


uint16_t blink_sequence[] = {
        0x0A00,     // Engine 1
        0xE00C,
        0x3313,
        0x1431,
		0x0831,
        0x0431,
        0x0831,
        0x0C31,
        0x0CB1,
        0x08B1,
        0x04B1,
        0x08B1,
        0x229D,
        0x229D,
        0x7300,
        0xE300,
        0xE080,
        0x5A00,
        0x3313,
        0x1431,
        0x0831,
        0x0431,
        0x0831,
        0x0C31,
        0x0CB1,
        0x08B1,
        0x04B1,
        0x08B1,
        0x229D,
        0x229D,
        0xE002,
        0x0000,
        0xE080,
        0x7300,
        0x3313,
        0x1431,
        0x0831,
        0x0431,
        0x0831,
        0x0C31,
        0x0CB1,
        0x08B1,
        0x04B1,
        0x08B1,
        0x229D,
        0x229D,
        0xE002,
        0x0000
};

/*
 * Blink sequence enables a progression of the light pulsation from RGB to white leds
 * Program shall select the leading led for the sequence (blue or green)
 * Blue sequence -> used for Bluetooth activity
 * Green sequence -> used for status activity at startup
 *
 */

uint16_t pulse_sequence_1[] = {
        // Engine 1
        0x0A00,
        0xE00C,
        0x047E,
        0x4600,
        0xA483,
        0x10FE,
        0x4600,
        0xE300,
        0x0000,
        0x0000,

        // Engine 2
        0xE080,
        0x4600,
        0xA191,
        0x047E,
        0x4600,
        0x4600,
        0xA315,
        0x04FE,
        0x5000,
        0xE002,

        // Engine 3
        0xE080,
        0x4600,
        0xA321,
        0x047E,
        0x4300,
        0x04FE,
        0x4600,
        0xA1A6,
        0x5000,
        0xE002
};


uint16_t pulse_sequence_2[] = {
        // Engine 1 - Blue and green
        0x0A00,
        0xE00C,
        0x027E,
        0x02FE,
        0x4600,
        0x057E,
        0x05FE,
        0x4C00,
        0xE300,

        // Engine 2 - White
        0xE080,
        0x027E,
        0x02FE,
        0x4B00,
        0x047E,
        0x04FE,
        0x4D00,
        0xE002,
        0x0000,

        // Engine 3 - Red
        0xE080,
        0x027E,
        0x02FE,
        0x4F00,
        0x037E,
        0x03FE,
        0x4D00,
        0xE002,
        0x0000
};


/*
 * Data Rx sequence provides a pulsation pattern for data sequence like:
 * firmware update (yellow = red + blue) -> pulsation only
 *
 * Data Rx sequence relies on leading activity of Engine 3
 *
 */
uint16_t dataRx_sequence[] = {
        0xE200,     // Engine 1
        0x4D00,
        0x027E,
        0x4D00,
        0x04FE,
        0x4600,
        0xE008,

        0xE200,     // Engine 2
        0x4800,
        0x027E,
        0x4D00,
        0x04FE,
        0x4600,
        0xE008,

        0x0A00,     // Engine 3
        0xE006,
        0x027E,
        0x4D00,
        0x04FE,
        0x4600,
        0xE180
};

/*
 * Data Rx sequence provides a pulsation pattern for data sequence like:
 * firmware update (yellow = red + blue) -> pulsation only
 *
 * Data Rx sequence relies on leading activity of Engine 3
 *
 */
uint16_t error_sequence[] = {
        0xA000,     // Engine 1
        0xE00C,
        0x4D00,
        0x027E,
        0x04FE,
        0xA182,
        0xE300,

        0xE080,     // Engine 2
        0x4D00,
        0x027E,
        0x04FE,
        0xA191,
        0x5000,
        0xE002,

        0xE080,     // Engine 3
        0x4D00,
        0x027E,
        0x04FE,
        0xA1A1,
        0x5000,
        0xE002
};


void Lp5562::SetLed(LedChannel channel, uint8_t intensity)
{
	ledIntensity[channel] = intensity;
	return;
}

void Lp5562::LoadIntensities()
{
	uint8_t regVal = LP5562_RESET;
	uint8_t cRed, cBlue, cGreen, cWhite;

	regVal = LP5562_CMD_DIRECT; // LP5562_PWM_HF
	WriteRegisters(OP_MODE, &regVal, 1);
	usleep(300);

	regVal = LP5562_ENG_SEL_PWM; // LP5562_PWM_HF
	WriteRegisters(ENG_SEL, &regVal, 1);

    WriteRegisters(B_PWM, &ledIntensity[B_LED],		1);
    WriteRegisters(G_PWM, &ledIntensity[G_LED], 	1);
    WriteRegisters(R_PWM, &ledIntensity[R_LED],  	1);
    WriteRegisters(W_PWM, &ledIntensity[W_LED], 	1);


    regVal = LP5562_DEFAULT_CFG; // LP5562_PWM_HF
    WriteRegisters(CONFIG, &regVal, 1);

    regVal = LP5562_ENABLE_DEFAULT;
	WriteRegisters(OP_MODE,    &regVal,   1);
	usleep(300);

	return;
}

void Lp5562::LoadEngines(Led_mode mode)
{
	uint16_t *sequencePtr = NULL;
    uint8_t regBuffer[2], seqLength = 0, dataBatch[4], readBuf[16];
    uint8_t cRed, cBlue, cGreen, cWhite;
    int i = 0, j = 0;

    cRed = 0x60;
    cBlue = 0x70;
    cGreen = 0x60;
    cWhite = 0x70;

    // Now load the sequence into engines buffer
    if((mode==BLINK_RED)||(mode==BLINK_YELLOW)||(mode==BLINK_VIOLET))
    {
        sequencePtr = blink_sequence;
        seqLength = sizeof(blink_sequence)/3;
    }else if((mode==PULSE_GREEN)||(mode==PULSE_BLUE)||(mode==PULSE_WHITE))
    {
        sequencePtr = pulse_sequence_1;
        seqLength = sizeof(pulse_sequence_1)/3;
    }else if((mode==PULSE_1)||(mode==BT_SEARCH)){
        sequencePtr = blink_sequence; //blink_sequence
        seqLength = sizeof(blink_sequence)/3;
    }else if(mode==FIRMWARE_SEQ){
        sequencePtr = dataRx_sequence;
        seqLength = sizeof(dataRx_sequence)/3;
    }else if(mode==ERROR_SEQ){
        sequencePtr = error_sequence;
        seqLength = sizeof(error_sequence)/3;
    }

    // Once the sequence has been loaded, update the dominant color
    if(mode==BLINK_RED)
    {
        cBlue = 0x80;
        cGreen = 0x80;

        // Red and white being attached to engine 1 for pulsation
        dataBatch[0] = 0;
                //(LP5562_ENG1_SEL << LP5562_SEL_WT_OFFSET);// |
                //(LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET) |
                //(LP5562_ENG1_SEL << LP5562_SEL_GR_OFFSET) |
                //(LP5562_ENG1_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1;
        dataBatch[2] = LP5562_MODE_ENG1_M | LP5562_MODE_ENG2_M | LP5562_MODE_ENG3_M;
        dataBatch[3] = LP5562_MODE_ENG1_M | LP5562_MODE_ENG2_M | LP5562_MODE_ENG3_M;
    }else if(mode==BLINK_YELLOW)
    {
        // Then allocate the led to the right engine
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_BL_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_WT_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1;
        dataBatch[2] = LP5562_RUN_ENG1;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1;

    }else if(mode==BLINK_VIOLET)
    {
        // Then allocate the led to the right engine
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_BL_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_WT_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1;
        dataBatch[2] = LP5562_RUN_ENG1;
        dataBatch[3] = LP5562_MASTER_ENABLE | LP5562_RUN_ENG1;
    }else if(mode==BLINK_WHITE)
    {
        // Then allocate the led to the right engine
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_BL_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_WT_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1;
        dataBatch[2] = LP5562_RUN_ENG1;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1;

    }else if(mode==PULSE_BLUE)
    {
        // Blue on first engine, green & white on second and red on third
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG2_SEL << LP5562_SEL_WT_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1 | LP5562_LOAD_ENG2 | LP5562_LOAD_ENG3;
        dataBatch[2] = LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
    }else if(mode==PULSE_GREEN)
    {
        cBlue = 0x02;
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG2_SEL << LP5562_SEL_WT_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1 | LP5562_LOAD_ENG2 | LP5562_LOAD_ENG3;
        dataBatch[2] = LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
   }else if(mode==BT_SEARCH)
    {
        cRed = 0x40;
        cBlue = 0x40;
        cGreen = 0x40;
        cWhite = 0x40;

        // Blue on first engine, green & white on second and red on third
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG2_SEL << LP5562_SEL_WT_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1 | LP5562_LOAD_ENG2 | LP5562_LOAD_ENG3;
        dataBatch[2] = LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3; // LP5562_ENABLE_RUN_PROGRAM

		printf("Setting up Bluetooth IHM");
		printf("\r\n");
    }else if(mode==PULSE_1)
    {
        cRed = 0x40;
        cBlue = 0x40;
        cGreen = 0x40;
        cWhite = 0x40;

        // Blue on first engine, green & white on second and red on third
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG1_SEL << LP5562_SEL_GR_OFFSET)|
                (LP5562_ENG2_SEL << LP5562_SEL_WT_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1 | LP5562_LOAD_ENG2 | LP5562_LOAD_ENG3;
        dataBatch[2] = LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
    }else if(mode==FIRMWARE_SEQ)
    {
        cRed = 0x09;
        cBlue = 0x02;
        cGreen = 0x00;
        cWhite = 0x02;

        // Blue on first engine, green & white on second and red on third
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG2_SEL << LP5562_SEL_WT_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1 | LP5562_LOAD_ENG2 | LP5562_LOAD_ENG3;
        dataBatch[2] = LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
    }else if(mode==ERROR_SEQ)
    {
        cRed = 0x09;
        cBlue = 0x02;
        cGreen = 0x02;
        cWhite = 0x02;

        // Blue on first engine, green & white on second and red on third
        dataBatch[0] =
                (LP5562_ENG1_SEL << LP5562_SEL_RD_OFFSET)|
                (LP5562_ENG2_SEL << LP5562_SEL_WT_OFFSET)|
                (LP5562_ENG3_SEL << LP5562_SEL_BL_OFFSET);

        dataBatch[1] = LP5562_LOAD_ENG1 | LP5562_LOAD_ENG2 | LP5562_LOAD_ENG3;
        dataBatch[2] = LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
        dataBatch[3] = LP5562_ENABLE_DEFAULT | LP5562_RUN_ENG1 | LP5562_RUN_ENG2 | LP5562_RUN_ENG3;
    }
    /*for(int i=0; i<13; i++)
    	readBuf[i] = 0;
    ReadRegisters(ENABLE, &readBuf[0], 13);
    for(int i=0; i<13; i++)
    	printf("[Register addr: 0x%2X], value = 0x%2X \r\n", i, readBuf[i]);*/

    regBuffer[0] = LP5562_DEFAULT_CFG; // LP5562_PWM_HF
    WriteRegisters(CONFIG, &regBuffer[0], 1);

    WriteRegisters(B_PWM, &cBlue,		1);
    WriteRegisters(G_PWM, &cGreen, 	1);
    WriteRegisters(R_PWM, &cRed,  	1);
    WriteRegisters(W_PWM, &cWhite, 	1);

    // First configure engines for loading mode
    regBuffer[0] = LP5562_CMD_DISABLE;
    WriteRegisters(OP_MODE,    regBuffer,     1);
	  usleep(300);

    // Finally program the different register with the write config
    WriteRegisters(ENG_SEL,    &dataBatch[0],  1);

    WriteRegisters(OP_MODE,    &dataBatch[1],   1);
    usleep(300);

    // Now load the sequence into engines buffer
    for(j=0; j<3; j++){
        for(i = 0; i<seqLength; i++){
            regBuffer[0] = (uint8_t)(sequencePtr[seqLength * j + i]>>8) & 0xFF;
            regBuffer[1] = (uint8_t)sequencePtr[seqLength * j + i] & 0xFF;
            WriteRegisters(PROG_MEM_ENG1 + 0x20 * j + 2 * i,  regBuffer, 2);
        }

        // Write last position with null pointer to make sure previous sequence would be discarded
        regBuffer[0] = 0x00;
        regBuffer[1] = 0x00;
        WriteRegisters(PROG_MEM_ENG1 + 0x20 * j + 2 * seqLength,  regBuffer, 2);
    }

    WriteRegisters(OP_MODE,    &dataBatch[2],   1);
	  usleep(300);

    WriteRegisters(ENABLE,     &dataBatch[3],   1);
	  usleep(600);

    return;
}


/**
 * \brief Function to stop engines of the LP5562 Led processor.
 *
 * ### Class of software element ###
 * Class B
 *
 * ### Software Element/Unit ID ###
 * SWU_HBD_YYYY

 * ### Software element description ###
 * This function deactivate engines of the Led processor.
 *
 * @param[in] void
 *
 * @return void
 *
 */
void Lp5562::StopEngines()
{
    uint8_t regValue = LP5562_CMD_DISABLE;

    WriteRegisters(OP_MODE, &regValue, 1);
    // lp5562_wait_opmode_done();

    return;
}


int IhmController::SetMode(IhmController::Ihm_mode _mode){
	lpDriver.LoadEngines((Lp5562::Led_mode)_mode);

	return EXIT_SUCCESS;
}
