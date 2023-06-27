#include "stm32f103_myPeripherals.h"

const uint16_t PWM_period = 14400;
extern volatile int noMs;

// make interrupt every milisecond
void Systick_init(void)
{
	SysTick_Config(71999);
}

void Setup_L298N(void){
    // enable clock for GPIO peripheral on APB2 bus
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_structure_L298N;
    // L298N has two enable pins
    GPIO_structure_L298N.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10;
    // output, push pull mode
	GPIO_structure_L298N.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_structure_L298N.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_structure_L298N);
    
    Start_motor();
}

void Start_motor(void){
    GPIO_SetBits(GPIOB,GPIO_Pin_11);
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);
}

void Setup_LED(void){
    // enable clock for GPIO peripheral on APB2 bus
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_structure_LED;
    GPIO_structure_LED.GPIO_Pin = GPIO_Pin_13;
	GPIO_structure_LED.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_structure_LED.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_structure_LED);
}

// button will be used to start and stop motor
void Setup_button(void){

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_structure_Buttons;

	GPIO_structure_Buttons.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    // setting up input, pull-down mode
	GPIO_structure_Buttons.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_structure_Buttons);
}

void Setup_timers(void){
    // Three timers:
    //
    // TIMER 2 -> Motor speed measurement
    // TIMER 3 -> generating PWM signal for controlling motor speed
    // TIMER 4 -> for controller



    /* ############### TIMER 2 ################  */

    // for capturing number of holes on encoder
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructInputCapture;

	GPIO_InitStructInputCapture.GPIO_Pin =  GPIO_Pin_3;
	GPIO_InitStructInputCapture.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructInputCapture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructInputCapture) ;

    TIM_ICInitTypeDef TIM2_ICInitStruct;

    TIM2_ICInitStruct.TIM_Channel = TIM_Channel_4;
    TIM2_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM2_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    // no filter, no prescaler
    TIM2_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM2_ICInitStruct.TIM_ICFilter = 0xFF;

    TIM_ICInit(TIM2, &TIM2_ICInitStruct);
    TIM_Cmd(TIM2, ENABLE) ;

    TIM_ITConfig(TIM2, TIM_IT_CC4, ENABLE);
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0,0));
    NVIC_EnableIRQ(TIM2_IRQn);



    /* ############### TIMER 3 ################  */


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructPWM;

    // Alternative function for PIN 6
	GPIO_InitStructPWM.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructPWM.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructPWM.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructPWM);

	// PWM frequency 5KHz
	TIM_TimeBaseInitTypeDef TIM3_InitStruct;
	TIM3_InitStruct.TIM_Prescaler = 0;
	TIM3_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM3_InitStruct.TIM_Period = PWM_period-1;
	TIM3_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3, &TIM3_InitStruct);
	TIM_Cmd(TIM3, ENABLE);

	// setting characteristics for PWM signal
	TIM_OCInitTypeDef TIM3_OCInitStruct;
	TIM3_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM3_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM3_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM3_OCInitStruct.TIM_Pulse = 0;

	TIM_OC1Init(TIM3, &TIM3_OCInitStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    // duty cycle
	TIM3->CCR1 = (uint16_t)(PWM_period*0.45);


    /* ############### TIMER 4 ################  */


	// period -> 0.05 seconds
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitTypeDef TIM4_InitStructure;
	TIM4_InitStructure.TIM_Prescaler = 5999; 				
	TIM4_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM4_InitStructure.TIM_Period = 599;
	TIM4_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &TIM4_InitStructure);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);

	NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1,0));
	NVIC_EnableIRQ(TIM4_IRQn);
}


uint16_t Get_PWM(void)
{
	return TIM3->CCR1;
}

// can be between 0 - 14400
void Set_PWM(uint16_t PWM_val)
{
	if(PWM_val <= PWM_period && PWM_val>=0){
		TIM3->CCR1 = PWM_val;
	}
}