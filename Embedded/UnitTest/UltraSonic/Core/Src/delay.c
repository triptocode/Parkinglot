/*
 * delay.c
 *
 *  Created on: Sep 12, 2023
 *      Author: user
 */

#include "delay.h"

void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim3, 0);    // 타이머 카운터 10번을 0번부터 시작하겠다
  while((__HAL_TIM_GET_COUNTER(&htim3)) < us);
}

