/**************************************************************************/
/*!
 * @file Adafruit_EEPROM_I2C.cpp
 */
/**************************************************************************/
#include <math.h>
#include <stdlib.h>

#include "Adafruit_EEPROM_I2C.h"

#include <stm32yyxx_hal_i2c.h>

// TODO: For CM3K not likely to be on I2C1.
// Only used if hi2c handle isn't provided
extern "C" I2C_HandleTypeDef hi2c2;

/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
 Constructor
 */
/**************************************************************************/
Adafruit_EEPROM_I2C::Adafruit_EEPROM_I2C(void) {

	hi2c = &hi2c2;	// Assign the default I2C, if needed
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  addr
 *            The I2C address to be used.
 *    @param  theWire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_EEPROM_I2C::begin(uint8_t addr, I2C_HandleTypeDef *handle) {

	// Is the Adafruit I2C Device already provided ?
	if (handle == NULL) {
		// Check to see if the extern I2C is ready
		// TODO: NULL check on hi2c1 to see if it needs to be configured
		if (hi2c->State == (HAL_I2C_StateTypeDef::HAL_I2C_STATE_READY)) {
			i2c_dev = new Adafruit_I2CDevice(addr, hi2c);
		} else {
			// TODO: Configure the default I2C device?
			// Throw error, or wait?
		}
	} else {
		hi2c = handle;
		i2c_dev = new Adafruit_I2CDevice(addr, hi2c);
	}

	_addr = addr;

	return i2c_dev->begin();
}

/**************************************************************************/
/*!
 @brief  Writes a byte at the specific EEPROM address

 @param[in] addr
 The 16-bit address to write to in EEPROM memory
 @param[in] value
 The 8-bit value to write at addr
 @returns True on I2C command success, false on timeout or I2C failure
 */
/**************************************************************************/
bool Adafruit_EEPROM_I2C::write(uint16_t addr, uint8_t value) {
	uint8_t buff[3] = { (uint8_t) (addr >> 8), (uint8_t) addr, value };

	if (!i2c_dev->write(buff, 3))
		return false;

	// Wait until it acks!
	uint8_t timeout = 100;
	while (timeout--) {
		if (i2c_dev->detected())
			return true;
		delay(1);
	}

	// timed out :(
	return false;
}

/**************************************************************************/
/*!
 @brief  Reads an 8 bit value from the specified EEPROM address
 @param addr
 The 16-bit address to read from in EEPROM memory
 @returns    The 8-bit value retrieved at addr
 */
/**************************************************************************/
uint8_t Adafruit_EEPROM_I2C::read(uint16_t addr) {
	uint8_t buff[2] = { (uint8_t) (addr >> 8), (uint8_t) addr };

	if (!i2c_dev->write_then_read(buff, 2, buff, 1))
		return 0x0;

	return buff[0];
}

/**************************************************************************/
/*!
 @brief  Writes multiple bytes at the specific EEPROM address

 @param[in] addr
 The 16-bit address to write to in EEPROM memory
 @param[in] buffer Pointer to buffer of bytes to write
 @param num How many bytes to write!
 @returns True on I2C command success, false on timeout or I2C failure
 */
/**************************************************************************/
bool Adafruit_EEPROM_I2C::write(uint16_t addr, uint8_t *buffer, uint16_t num) {
	while (num--) {
		if (!write(addr++, buffer[0])) {
			return false;
		}
		buffer++;
	}
	return true;
}

/**************************************************************************/
/*!
 @brief  Reads multiple bytes from the specified EEPROM address
 @param addr
 The 16-bit address to read from in EEPROM memory
 @param buffer Pointer to buffer of bytes that will be filled!
 @param num How many bytes to write!
 @returns    The 8-bit value retrieved at addr
 */
/**************************************************************************/
bool Adafruit_EEPROM_I2C::read(uint16_t addr, uint8_t *buffer, uint16_t num) {

	for (uint16_t i = 0; i < num; i++) {
		uint8_t buff[2] = { (uint8_t) (addr >> 8), (uint8_t) addr };

		if (!i2c_dev->write_then_read(buff, 2, buff, 1))
			return false;
		buffer[i] = buff[0];

		addr++;
	}

	return true;
}
