/*
 * I2C_LCD.h
 *
 *  Created on: Sep 20, 2023
 *      Author: user
 */

#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_



#include "stm32f4xx_hal.h"
#include "i2c.h"


#define LCD_ADDRESS (0X27 << 1)


void LCD_init();

void LCD_clear();
void LCD_put_cursor(uint8_t row, uint8_t col);

void LCD_send_cmd(uint8_t);
void LCD_send_data(char);
void LCD_send_str(char *);



#endif /* INC_I2C_LCD_H_ */
