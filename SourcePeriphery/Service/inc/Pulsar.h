#pragma once

#include "mbed.h"
#include "GlobalConfig.h"
#include "stm32f1xx_hal.h"


class Pulsar
{
public:

	static Pulsar& Instance();

	TIM_HandleTypeDef* GetTimAccurateHandler();
	TIM_HandleTypeDef* GetTimPeriodicHandler();

private:

	//��������� ������� ������� (1 ���)
	struct AccurateTimerSettings 
	{
		static constexpr uint16_t period_us				= 60000;	//������ ������� �������

		static constexpr uint16_t cutoff_us				= 55000;	//����� �������� ������� (��� ���������� �� ����������)

		static constexpr uint16_t strob_us				= 400;		//������������ ������������� ��������

		static constexpr uint16_t pulse_hse72_us		= 24;		//����� ������ ��������� �������� ������������ ������ ������
		static constexpr uint16_t phase_hse72_us		= 17;		//��� ��������� ��������� ������������

		static constexpr uint16_t pulse_hsi64_us		= 24;		//�������� ������ ��� ��������� ��������� ��������
		static constexpr uint16_t phase_hsi64_us		= 16;		//��� ��������� ��������� ������������


		static constexpr float tick_factor				= 1.0f;
		static constexpr uint16_t truePeriod_us			= tick_factor * period_us;
		static constexpr uint16_t trueStrob_us			= tick_factor * strob_us;
		static constexpr uint16_t trueCutoff_us			= tick_factor * cutoff_us;

		static constexpr uint16_t truePulse_hse72_us	= tick_factor * pulse_hse72_us;
		static constexpr uint16_t truePulse_hsi64_us	= tick_factor * pulse_hsi64_us;
		static constexpr uint16_t truePhase_hse72_us	= tick_factor * phase_hse72_us;
		static constexpr uint16_t truePhase_hsi64_us	= tick_factor * phase_hsi64_us;
	};

	//��������� �������������� ������� (1 ��)
	struct PeriodicTimerSettings
	{
		static constexpr uint16_t period_ms			= 100;

		static constexpr float tick_factor			= 2.0f;
		static constexpr uint16_t truePeriod_ms		= tick_factor * period_ms;
	};

	//��������� ��������� ��������
	static void AccurateTimerInit();
	static void PeriodicTimerInit();

	//�������� ��������
	static TIM_HandleTypeDef htimPeriodic;
	static TIM_HandleTypeDef htimAccurate;

	Pulsar();
};

extern "C" void TIM3_IRQHandler(void);
extern "C" void TIM3_IRQHandler(void);
extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);