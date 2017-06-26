#include "PwmOutT.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

PwmOutT::PwmOutT()
	: min(0)
{

}

void PwmOutT::SetPulse(float value)
{
	TIM_OC_InitTypeDef sConfig;

	if (value < (float)0.0) {
		value = 0.0;
	}
	else if (value > 100) {
		value = 100;
	}

	value = (tim.Init.Period - 200) * value / 100.f + 100;

	    // Configure channels
	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.Pulse        = value; 
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	
	HAL_TIM_PWM_ConfigChannel(&tim, &sConfig, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&tim, TIM_CHANNEL_3);
}

void PwmOutT::Initialize()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	
	TIM_ClockConfigTypeDef  sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef		sConfigOC;

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin   = GPIO_PIN_0;
	GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	uint32_t mHz			= 100000;

	tim.Init.ClockDivision	= 0;
	tim.Init.CounterMode	= TIM_COUNTERMODE_UP;
	tim.Init.Prescaler		= (SystemCoreClock / mHz) - 1;		// 1 mHz 
	tim.Init.Period			= (mHz / 333.0f) - 1;
	tim.Instance			= TIM3;

	__HAL_TIM_DISABLE(&tim);
	HAL_TIM_Base_Init(&tim);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&tim, &sClockSourceConfig);

	HAL_TIM_PWM_Init(&tim);
	__HAL_TIM_ENABLE(&tim);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&tim, &sMasterConfig);

	sConfigOC.OCMode       = TIM_OCMODE_PWM1;
	sConfigOC.Pulse        = 0; 
	sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	HAL_TIM_PWM_ConfigChannel(&tim, &sConfigOC, TIM_CHANNEL_3);
}