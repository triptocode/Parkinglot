/*
 * I2C_LCD.c
 *
 *  Created on: Sep 14, 2023
 *      Author: user
 */

#include "I2C_LCD.h"

#define SLAVE_ADDRESS_LCD   0x27<<1

void lcd_send_cmd(char cmd)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (cmd & 0xf0);    // cmd랑 0xf0이랑 &연산하면 상위만 남음
  data_l = ((cmd<<4) & 0xf0);
  //data_l = (cmd & 0x0f); // both ok
  data_t[0] = data_u | 0x0c;    // en = 1, rs = 0
  data_t[1] = data_u | 0x08;
  data_t[2] = data_l | 0x0c;
  data_t[3] = data_l | 0x08;
  HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);    //
}

void lcd_send_data(char data)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (data & 0xf0);    // cmd랑 0xf0이랑 &연산하면 상위만 남음
  data_l = ((data<<4) & 0xf0);
  //data_l = (cmd & 0x0f); // both ok
  data_t[0] = data_u | 0x0d;    // en = 1, rs = 0
  data_t[1] = data_u | 0x09;
  data_t[2] = data_l | 0x0d;
  data_t[3] = data_l | 0x09;
  HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);    //
}

void lcd_clear(void)
{
  lcd_send_data(0x08);
  for(int i = 0; i<70; i++)
  {
    lcd_send_data(' ');
  }
}

void lcd_put_cur(int row, int col)
{
  switch(row)
  {
    case 0:
      col |= 0x80;
      break;

    case 1:
      col |= 0xc0;
      break;
  }
  lcd_send_cmd(col);
}

void lcd_init(void) // 4bit
{
  HAL_Delay(30);
  lcd_send_cmd(0x30);
  HAL_Delay(5);
  lcd_send_cmd(0x30);
  HAL_Delay(1);
  lcd_send_cmd(0x30);
  HAL_Delay(10);
  lcd_send_cmd(0x20);
  HAL_Delay(10);        // 여기까지가 초기화 // 4bit initial

  lcd_send_cmd(0x28);   // 2row, 5*8 font
  HAL_Delay(1);
  lcd_send_cmd(0x08);   // display all off
  HAL_Delay(1);
  lcd_send_cmd(0x01);   // clear display
  HAL_Delay(1);
  lcd_send_cmd(0x0c);   // display on cursor off flash off
  HAL_Delay(1);
  lcd_send_cmd(0x06);   // display right move  // 여기까지가 initial
}

void lcd_send_string(char *str)
{
  while(*str)
  {
    lcd_send_data(*str++);
  }
}














