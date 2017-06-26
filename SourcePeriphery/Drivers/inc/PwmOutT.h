#pragma once

#include <stdint.h>
#include "stm32f1xx_hal.h"

class PwmOutT
{
public:
	PwmOutT();

	void Initialize();

	void SetPulse(float value);

private:
	TIM_HandleTypeDef tim;

	uint16_t min, max;
};