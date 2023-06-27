#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "uart.h"
#include "stm32f103_myPeripherals.h"
#include "stm32f10x_util.h"
#include "stm32f10x_ina219.h"
#include <stdlib.h>

/* global variables */
volatile char receivedChar;
volatile uint16_t currentPWM;
extern const uint16_t PWM_period;
volatile int noMs = 0;
float voltage, current;
char *buff;
volatile int flagMeasure = 0;
volatile int br=0;
volatile int rpm=0;
const int freq= 72000000;
const float ts=0.05;

volatile uint16_t pulse_ticks = 0;
volatile unsigned long start_time = 0;
volatile unsigned long end_time = 0;

//initial referent value (motor speed)
volatile int yref=4000;

/* UART receive interrupt handler */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		//echo character
		receivedChar = USART_GetChar();
		if(receivedChar == 'u')
		{
			currentPWM = Get_PWM();
			currentPWM += (uint16_t)500;
			if(currentPWM < PWM_period)
				Set_PWM(currentPWM);
		}
		else if(receivedChar == 'd')
		{

			currentPWM = Get_PWM();
			if(currentPWM > 7000u)
			{
				currentPWM -= (uint16_t)500;
				Set_PWM(currentPWM);
			}
		}
		else if(receivedChar == '+'){
			yref+=600;
			if(yref>7800){
				yref=7800;
			}
		}
		else if(receivedChar == '-'){
			yref-=600;
			if(yref<200){
				yref=200;
			}
		}
		USART_PutChar(receivedChar);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

/* TIM2 input capture interrupt */
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_CC4))
	{
		end_time = TIM2->CCR4;
		pulse_ticks = end_time - start_time;
        start_time = end_time;
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);
		br++;
	}

}

// controller input/output
volatile uint16_t u_1=0,u;
volatile uint16_t u_2=0;
volatile int e_1=0,e_2=0,e;

// PI
const float m1 = 0.516124799280896;
const float m2 = 0.133064516129032;

void PI_controller(){
	e=yref-rpm;
	u = u_1 + m1*e - m1*m2*e_1;
	Set_PWM(u);

	u_1=u;
	e_1=e;
}

// PID
const float r1 = 0.876602282840625;
const float r2 = 0.224122435248103;
const float r3 = 0.218648872181070;
const float r4 = 0.095493703172832;


void PID_controller(){
	e=yref-rpm;
	u = r4*u_1 + u_1 - r4*u_2 + r1*e -
	  r1*r3*e_1 - r1*r2*e_1 +
	  r1*r2*r3*e_2;
	Set_PWM(u);

	u_2=u_1;
	e_2=e_1;
	u_1=u;
	e_1=e;
}

/* 41 holes on encoder */
/* TIMER4 every 0.05 second interrupt --> sending data to PC */

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update))
	{
		USART_PutChar('p');
		USART_PutChar('m');

		rpm=(uint32_t)((float)br/ts* (float)60/41);

		PID_controller();

		USART_SendUInt_32(rpm);
		USART_SendUInt_32(yref);

		//USART_SendUInt_32((uint16_t)Get_PWM());
		//rpm=(uint32_t)(((float)freq*60))/((float)pulse_ticks*41);

		br=0;
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}

int flag=0;
const int lower=250;
const int upper=800;
int duration=250;

/* systick timer for periodic tasks */
/* generating PRBS */
void SysTick_Handler(void)
{
	noMs++;
	if(flag==0){
		//Set_PWM(7000u);
			if(noMs>duration){
				flag=1;
				noMs=0;
				duration=(rand()%(upper-lower))+lower;
			}
	}
	else{
		 //Set_PWM(9000u);
			if(noMs>duration){
				flag=0;
				noMs=0;
				duration=(rand()%(upper-lower))+lower;
		}
	}
}

/* main program - init peripherals and loop */
int main(void)
{
	NVIC_SetPriorityGrouping(0u);
	Systick_init();
	Setup_LED();
	Setup_L298N();
	Setup_button();
	Setup_timers();
	USART1_PC_Init();
	
	while (1)
	{
		// read push button - stop motor
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14))
		{
			Set_PWM(0u);
		}
		// read push button - turn on motor
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))
		{
			Set_PWM(5000u);
		}
	}
}

