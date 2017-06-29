#pragma once

#include "mbed.h"
#include "stm32f1xx_hal.h"

extern "C" void TIM3_IRQHandler(void);

extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

class Pulsar
{
public:

	Pulsar();
	static TIM_HandleTypeDef* GetTimAccurateHandler();
	static TIM_HandleTypeDef* GetTimPeriodicHandler();

	static void AccurateTimerInit();
	static void PeriodicTimerInit();

private:

	static TIM_HandleTypeDef htimPeriodic;
	static TIM_HandleTypeDef htimAccurate;

};