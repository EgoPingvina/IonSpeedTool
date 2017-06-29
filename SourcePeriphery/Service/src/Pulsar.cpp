#include "Pulsar.h"

//Индикация
DigitalOut process_led	= { PA_5, 0 };
DigitalOut alarm_led	= { PA_4, 0 };
DigitalOut test_out		= { PA_0, 0 };

Pulsar::Pulsar()
{
	AccurateTimerInit();
	PeriodicTimerInit();
}

Pulsar& Pulsar::Instance()
{
	static Pulsar singlePulsar = Pulsar();

	return singlePulsar;
}

TIM_HandleTypeDef * Pulsar::GetTimAccurateHandler() { return &Pulsar::htimAccurate; }
TIM_HandleTypeDef * Pulsar::GetTimPeriodicHandler() { return &Pulsar::htimPeriodic; }

void Pulsar::AccurateTimerInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_ClkInitTypeDef clockConfig;
	uint32_t pclkFrequency;

	// Собираем информацию об установленных настройках
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();


	/************************** TIM3 GPIO Configuration *******************************/
	/**
		PA6     ------> TIM3_CH1
		PA7     ------> TIM3_CH2
	*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin			= GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed		= GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/************************* Базовая настройка таймера ****************************/

	__HAL_RCC_TIM3_CLK_ENABLE();
	__TIM3_FORCE_RESET();
	__TIM3_RELEASE_RESET();

	//1 us per tick
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		htimAccurate.Init.Prescaler = static_cast<uint16_t>((pclkFrequency) / 1000000 - 1);
	else
		htimAccurate.Init.Prescaler = static_cast<uint16_t>((pclkFrequency * 2) / 1000000 - 1);

	htimAccurate.Instance				= TIM3;
	htimAccurate.Init.CounterMode		= TIM_COUNTERMODE_UP;
	htimAccurate.Init.Period			= AccurateTimerSettings::truePeriod_us;
	htimAccurate.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	htimAccurate.Init.RepetitionCounter = 0;

	HAL_TIM_OnePulse_Init(&htimAccurate, TIM_OPMODE_SINGLE);

	/********************* Настройка 1 канала сравнения ****************************/
	TIM_OC_InitTypeDef			sConfigOC;

	sConfigOC.OCMode			= TIM_OCMODE_PWM1;
	sConfigOC.Pulse				= AccurateTimerSettings::trueStrob_us;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
		alarm_led = 1;

	/********************* Настройка 2 канала сравнения ****************************/

#if (HSE72mHz == 1)
	sConfigOC.Pulse				= AccurateTimerSettings::truePulse_hse72_us;		//ExternalClock = 24
#else
	sConfigOC.Pulse				= AccurateTimerSettings::truePulse_hsi64_us;		//InternalClock = ?
#endif

	sConfigOC.OCMode			= TIM_OCMODE_PWM1;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
		alarm_led = 1;

	/********************* Настройка 3 канала сравнения ****************************/

	sConfigOC.Pulse				= AccurateTimerSettings::trueCutoff_us;
	sConfigOC.OCMode			= TIM_OCMODE_TIMING;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
		alarm_led = 1;

	/********************* Настройка 4 канала сравнения ****************************/

#if (HSE72mHz == 1)
	sConfigOC.Pulse = AccurateTimerSettings::truePhase_hse72_us;		//ExternalClock = 24
#else
	sConfigOC.Pulse = AccurateTimerSettings::truePhase_hsi64_us;		//InternalClock = ?
#endif

	sConfigOC.OCMode		= TIM_OCMODE_TIMING;
	sConfigOC.OCPolarity	= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
		alarm_led = 1;

	/************************* Настройка прерываний *********************************/
	__HAL_TIM_CLEAR_IT(&htimAccurate, TIM_IT_UPDATE);

	NVIC_SetPriority(TIM3_IRQn, 0);
	NVIC_EnableIRQ(TIM3_IRQn);

	//Настройка прерываний по каналам таймера
#if 0
	//Включить генерацию прерывания по совпадению с каналом 1 
	if (HAL_TIM_OC_Start_IT(&htimAccurate, TIM_CHANNEL_1 ) != HAL_OK)
		led = 1;

	//Включить генерацию прерывания по совпадению с каналом 2
	if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_2) != HAL_OK)
		led = 1;

	//Включить генерацию прерывания по совпадению с каналом 3
	if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_3) != HAL_OK)
		led = 1;

	//Включить генерацию прерывания по совпадению с каналом 4
	if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_4) != HAL_OK)
		led = 1;

	//Включить генерацию прерывания по достижению значения периода
	if (HAL_TIM_Base_Start_IT(&htimAccurate) != HAL_OK)
		led = 1;
#endif

}

void Pulsar::PeriodicTimerInit()
{
	RCC_ClkInitTypeDef	clockConfig;
	uint32_t			pclkFrequency;

	// Собираем информацию об установленных настройках тактирования
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();

	/************************* Базовая настройка таймера ****************************/
	__HAL_RCC_TIM2_CLK_ENABLE();
	__TIM2_FORCE_RESET();
	__TIM2_RELEASE_RESET();

	//0.5 ms per tick
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		htimPeriodic.Init.Prescaler = static_cast<uint16_t>((pclkFrequency) / 2000 - 1);
	else
		htimPeriodic.Init.Prescaler = static_cast<uint16_t>((pclkFrequency * 2) / 2000 - 1);

	htimPeriodic.Instance				= TIM2;
	htimPeriodic.Init.CounterMode		= TIM_COUNTERMODE_UP;
	htimPeriodic.Init.Period			= PeriodicTimerSettings::truePeriod_ms;
	htimPeriodic.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	htimPeriodic.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&htimPeriodic) != HAL_OK)
		alarm_led = 1;

	/************************* Настройка прерываний *********************************/

	__HAL_TIM_CLEAR_IT(&htimPeriodic, TIM_IT_UPDATE);

	NVIC_SetPriority(TIM2_IRQn, 0);
	NVIC_EnableIRQ(TIM2_IRQn);


	//Запуск периодического таймера
	if (HAL_TIM_Base_Start_IT(&htimPeriodic) != HAL_OK)
		alarm_led = 1;
}


extern "C" void TIM2_IRQHandler(void)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	HAL_TIM_IRQHandler(pulsarInstance.GetTimPeriodicHandler());
}

extern "C" void TIM3_IRQHandler(void)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	HAL_TIM_IRQHandler(pulsarInstance.GetTimAccurateHandler());
}

//Обратный вызов завершения периода таймеров
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	TIM_OC_InitTypeDef sConfigOC;

	if (htim->Instance == TIM2)
	{
		test_out = !test_out;
		
		//Запуск смещающего канала 20 us
		HAL_TIM_OC_Start_IT(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_4);

		//Запуск строба
		//HAL_TIM_OnePulse_Start(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_1);

		//Запуск упреждающего канала для останова таймера
		HAL_TIM_OC_Start_IT(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_3);
	}

	if (htim->Instance == TIM3)
	{
		if (htim->Channel == TIM_CHANNEL_1)
		{
			//HAL_TIM_Base_Stop_IT(Pulsar::GetTimAccurateHandler());
			//HAL_TIM_OnePulse_Stop(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);
			//HAL_TIM_OC_Stop(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);
		}
		
	}
}

//Callback совпадения с каналом сравнения
extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	if (htim->Instance == TIM3)
	{
		static uint8_t second_ch_state = 0;
		TIM_OC_InitTypeDef sConfigOC;

		//Выяснить по какому из каналов произошло совпадение
		switch (htim->Channel)
		{
		case HAL_TIM_ACTIVE_CHANNEL_1:
		{
			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_2:
		{
			
			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_3:
		{
			HAL_TIM_OC_Stop(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_1);
			HAL_TIM_OC_Stop(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_2);

			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_4:
		{

			HAL_TIM_OC_Start(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_2);

			break;
		}

		}
	}
}

TIM_HandleTypeDef Pulsar::htimPeriodic;
TIM_HandleTypeDef Pulsar::htimAccurate;