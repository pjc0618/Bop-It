/**
 * @file 3140_accel.h
 * @author Samuel DiPietro
 * @copyright All rights reserved, 2020
 *
 * This file holds function prototypes for concurrency functions
 * @warning This file is offered AS IS and WITHOUT ANY WARRANTY, without
 * even the implied warranties of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "3140_i2c.h"

#ifndef __3140_accel__
#define __3140_accel__

#define I2C_BAUD_RATE	100000U		// Speed of I2C Transmission 

#define ACCEL_I2C_ADDR	0x1D		// I2C Device Address
#define ACCEL_WHO_AM_I	0x0D		// Who-Am-I Reg Address
#define ACCEL_I2C_BASE	I2C0		// I2C Channel Used

#define ACCEL_READ_LEN 7			// Length of Data Read


/* Struct to store read values from the accelerometer */
typedef struct{
	int16_t x;			// Signed Shorts
	int16_t y;
	int16_t z;
	uint8_t s;			// Status Register 
} SRAWDATA;

/* NON-Blocking Who-Am-I Register Read
 *
 * Outputs: Returns 1 if the Who-Am-I Register was able to be read and was
 *			 the correct value. Returns -1 if there was an error
 */
int ACCEL_ReadWhoAmI(void);

/* NON-Blocking: Writes the base setup to the accelerometer. Configures it 
 * 				 for polling accelerometer data. Magnometer disabled.
 *
 * Outputs: Returns 1 if the configuration suceeded, -1 if it failed
 */
int ACCEL_getDefaultConfig(void);

/* Reads data from the accelerometer registers. Also records status register value
 * Input Args[]		accelDat: A pointer to a SRAWDATA struct that will recieved the 
 *							  the polled data
 * 
 * Output: Returns 1 on sucessful read, with the read bytes in the SRAWDATA struct. 
 * 		   Returns -1 on failure to read bytes
 */
int ACCEL_getAccelDat( SRAWDATA * accelDat);

#endif /* __3140_accel__ */