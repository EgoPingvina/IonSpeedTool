#include "Pulsar.h"

DigitalOut led1 = { PA_5, 0 };
//static TIM_HandleTypeDef htim;
DigitalOut led = {PA_4, 0};
DigitalOut test_out = { PA_0, 0 };

Pulsar::Pulsar()
{
}

TIM_HandleTypeDef * Pulsar::GetTimAccurateHandler() { return &Pulsar::htimAccurate; }
TIM_HandleTypeDef * Pulsar::GetTimPeriodicHandler() { return &Pulsar::htimPeriodic; }

void Pulsar::AccurateTimerInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Параметры используемого таймера
	RCC_ClkInitTypeDef clockConfig;
	// Частота ядра (влияет на частоты шин)
	uint32_t pclkFrequency;

	// Собираем информацию об установленных настройках
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();

	__HAL_RCC_GPIOD_CLK_ENABLE();

	/**
		TIM3 GPIO Configuration
		PA6     ------> TIM3_CH1
		PA7     ------> TIM3_CH2
	*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin			= GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode		= GPIO_MODE_AF_PP;
	//GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed		= GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	__HAL_RCC_TIM3_CLK_ENABLE();
	__TIM3_FORCE_RESET();
	__TIM3_RELEASE_RESET();

	htimAccurate.Instance				= TIM3;

	//1 us per tick
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		htimAccurate.Init.Prescaler = (uint16_t)((pclkFrequency) / 1000000) - 1;
	else
		htimAccurate.Init.Prescaler = (uint16_t)((pclkFrequency * 2) / 1000000 - 1);

	htimAccurate.Init.CounterMode		= TIM_COUNTERMODE_UP;
	htimAccurate.Init.Period			= 60000;
	htimAccurate.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	htimAccurate.Init.RepetitionCounter = 0;

	HAL_TIM_OnePulse_Init(&htimAccurate, TIM_OPMODE_SINGLE);
	//HAL_TIM_Base_Init(&htimAccurate);

	TIM_OC_InitTypeDef sConfigOC;

	/********************* Настройка 1 канала сравнения ****************************/
	sConfigOC.OCMode			= TIM_OCMODE_PWM1;
	sConfigOC.Pulse				= 400;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
		led = 1;
	/********************************************************************************/

	/********************* Настройка 2 канала сравнения ****************************/
	sConfigOC.OCMode			= TIM_OCMODE_PWM1;
	sConfigOC.Pulse				= 23;		//Internal = 24, External = 23
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
		led = 1;
	/********************************************************************************/

	/********************* Настройка 3 канала сравнения ****************************/
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 55000;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
		led = 1;
	/********************************************************************************/

	/********************* Настройка 4 канала сравнения ****************************/
	sConfigOC.OCMode		= TIM_OCMODE_TIMING;
	sConfigOC.Pulse			= 17;		//Internal = 17, External = 17
	sConfigOC.OCPolarity	= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
		led = 1;
	/********************************************************************************/

	/*************** Настройка прерываний **********************/
	NVIC_SetPriority(TIM3_IRQn, 0);
	NVIC_EnableIRQ(TIM3_IRQn);

	//Включить генерацию прерывания по совпадению с каналом 1 
	//if (HAL_TIM_OC_Start_IT(&htimAccurate, TIM_CHANNEL_1 ) != HAL_OK)
	//	led = 1;

	//if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_2) != HAL_OK)
	//	led = 1;

	//Включить генерацию прерывания по достижению значения периода
	//if (HAL_TIM_Base_Start_IT(&htimAccurate) != HAL_OK)
	//	led = 1;
}

void Pulsar::PeriodicTimerInit()
{
	RCC_ClkInitTypeDef	clockConfig;
	uint32_t			pclkFrequency;

	// Собираем информацию об установленных настройках тактирования
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();

	//Сброс шины таймера
	__HAL_RCC_TIM2_CLK_ENABLE();
	__TIM2_FORCE_RESET();
	__TIM2_RELEASE_RESET();

	//1 ms per tick
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		htimPeriodic.Init.Prescaler = (uint16_t)((pclkFrequency) / 2000) - 1;
	else
		htimPeriodic.Init.Prescaler = (uint16_t)((pclkFrequency * 2) / 2000 - 1);

	htimPeriodic.Instance				= TIM2;
	htimPeriodic.Init.CounterMode		= TIM_COUNTERMODE_UP;
	htimPeriodic.Init.Period			= 200;
	htimPeriodic.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	htimPeriodic.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&htimPeriodic) != HAL_OK)
		led = !led;

	NVIC_SetPriority(TIM2_IRQn, 0);
	NVIC_EnableIRQ(TIM2_IRQn);

	//Запуск периодического таймера
	if (HAL_TIM_Base_Start_IT(&htimPeriodic) != HAL_OK)
		led = !led;
}


extern "C" void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(Pulsar::GetTimPeriodicHandler());
}

extern "C" void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(Pulsar::GetTimAccurateHandler());
}

//Обратный вызов завершения периода таймеров
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	TIM_OC_InitTypeDef sConfigOC;

	if (htim->Instance == TIM2)
	{
		test_out = !test_out;
		
		//Запуск смещающего канала 20 us
		HAL_TIM_OC_Start_IT(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_4);

		//Запуск строба
		//HAL_TIM_OnePulse_Start(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);

		//Запуск упреждающего канала для останова таймера
		HAL_TIM_OC_Start_IT(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_3);
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
	if (htim->Instance == TIM3)
	{
		static uint8_t second_ch_state = 0;
		TIM_OC_InitTypeDef sConfigOC;

		//Выяснить по какому из каналов произошло совпадение
		switch (htim->Channel)
		{
		case HAL_TIM_ACTIVE_CHANNEL_1:
		{
			//HAL_TIM_OC_Stop(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);
			//HAL_TIM_Base_Stop_IT(Pulsar::GetTimAccurateHandler());
			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_2:
		{
			
			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_3:
		{
			HAL_TIM_OC_Stop(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_1);
			HAL_TIM_OC_Stop(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_2);

			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_4:
		{

			HAL_TIM_OC_Start(Pulsar::GetTimAccurateHandler(), TIM_CHANNEL_2);

			break;
		}

		}
	}
}

TIM_HandleTypeDef Pulsar::htimPeriodic;
TIM_HandleTypeDef Pulsar::htimAccurate;