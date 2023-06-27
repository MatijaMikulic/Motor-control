#ifndef STM32F103_MY_PERIPHERALS
#define STM32F103_MY_PERIPHERALS

#include "stm32f10x.h"
#include "stdint.h"

void Systick_init(void);
void Setup_L298N(void);
void Start_motor(void);
void Setup_LED(void);
void Setup_timers(void);
void Setup_button(void);
uint16_t Get_PWM(void);
void Set_PWM(uint16_t PWM_val);

#endif