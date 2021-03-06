#include "Pulsar.h"

//���������
DigitalOut process_led	= { PA_5, 0 };
DigitalOut alarm_led	= { PA_4, 0 };

Pulsar::Pulsar()
{
}

Pulsar& Pulsar::Instance()
{
	static Pulsar singlePulsar = Pulsar();

	return singlePulsar;
}

bool Pulsar::Initialize()
{
	AccurateTimerInit();
	InputCaptureTimerInit();
	PeriodicTimerInit();

	return true;
}

TIM_HandleTypeDef * Pulsar::GetTimAccurateHandler()		{ return &Pulsar::htimAccurate; }
TIM_HandleTypeDef * Pulsar::GetTimPeriodicHandler()		{ return &Pulsar::htimPeriodic; }
TIM_HandleTypeDef * Pulsar::GetTimInputCaptureHandler() { return &Pulsar::htimInputCapture; }

void Pulsar::AccurateTimerInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_ClkInitTypeDef clockConfig;
	uint32_t pclkFrequency;

	// �������� ���������� �� ������������� ����������
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

	/************************* ������� ��������� ������� ****************************/

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

	/********************* ��������� 1 ������ ��������� ****************************/
	TIM_OC_InitTypeDef			sConfigOC;

	sConfigOC.OCMode			= TIM_OCMODE_PWM1;
	sConfigOC.Pulse				= AccurateTimerSettings::trueStrob_us;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
		alarm_led = 1;

	/********************* ��������� 2 ������ ��������� ****************************/

#if (HSE72mHz == 1)
	sConfigOC.Pulse				= AccurateTimerSettings::truePulse_hse72_us;		//ExternalClock = 24
#else
	sConfigOC.Pulse				= AccurateTimerSettings::truePulse_hsi64_us;		//InternalClock = ?
#endif

	sConfigOC.OCMode			= TIM_OCMODE_PWM1;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
		alarm_led = 1;

	/********************* ��������� 3 ������ ��������� ****************************/

	sConfigOC.Pulse				= AccurateTimerSettings::trueCutoff_us;
	sConfigOC.OCMode			= TIM_OCMODE_TIMING;
	sConfigOC.OCPolarity		= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
		alarm_led = 1;

	/********************* ��������� 4 ������ ��������� ****************************/

#if (HSE72mHz == 1)
	sConfigOC.Pulse = AccurateTimerSettings::truePhase_hse72_us;		//ExternalClock = 24
#else
	sConfigOC.Pulse = AccurateTimerSettings::truePhase_hsi64_us;		//InternalClock = ?
#endif

	sConfigOC.OCMode		= TIM_OCMODE_TIMING;
	sConfigOC.OCPolarity	= TIM_OCPOLARITY_HIGH;

	if (HAL_TIM_OC_ConfigChannel(&htimAccurate, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
		alarm_led = 1;

	/************************* ��������� ���������� *********************************/
	__HAL_TIM_CLEAR_IT(&htimAccurate, TIM_IT_UPDATE);

	NVIC_SetPriority(TIM3_IRQn, 0);
	NVIC_EnableIRQ(TIM3_IRQn);

	//��������� ���������� �� ������� �������
#if 0
	//�������� ��������� ���������� �� ���������� � ������� 1 
	if (HAL_TIM_OC_Start_IT(&htimAccurate, TIM_CHANNEL_1 ) != HAL_OK)
		led = 1;

	//�������� ��������� ���������� �� ���������� � ������� 2
	if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_2) != HAL_OK)
		led = 1;

	//�������� ��������� ���������� �� ���������� � ������� 3
	if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_3) != HAL_OK)
		led = 1;

	//�������� ��������� ���������� �� ���������� � ������� 4
	if (HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_4) != HAL_OK)
		led = 1;

	//�������� ��������� ���������� �� ���������� �������� �������
	if (HAL_TIM_Base_Start_IT(&htimAccurate) != HAL_OK)
		led = 1;
#endif

}

void Pulsar::PeriodicTimerInit()
{
	RCC_ClkInitTypeDef	clockConfig;
	uint32_t			pclkFrequency;

	// �������� ���������� �� ������������� ���������� ������������
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();

	/************************* ������� ��������� ������� ****************************/
	__HAL_RCC_TIM1_CLK_ENABLE();
	__TIM1_FORCE_RESET();
	__TIM1_RELEASE_RESET();

	//0.5 ms per tick
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		htimPeriodic.Init.Prescaler = static_cast<uint16_t>((pclkFrequency) / 2000 - 1);
	else
		htimPeriodic.Init.Prescaler = static_cast<uint16_t>((pclkFrequency * 2) / 2000 - 1);

	htimPeriodic.Instance				= TIM1;
	htimPeriodic.Init.CounterMode		= TIM_COUNTERMODE_UP;
	htimPeriodic.Init.Period			= PeriodicTimerSettings::truePeriod_ms;
	htimPeriodic.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	htimPeriodic.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&htimPeriodic) != HAL_OK)
		alarm_led = 1;

	/************************* ��������� ���������� *********************************/

	__HAL_TIM_CLEAR_IT(&htimPeriodic, TIM_IT_UPDATE);

	NVIC_SetPriority(TIM1_UP_IRQn, 0);
	NVIC_EnableIRQ(TIM1_UP_IRQn);

	if (HAL_TIM_Base_Start_IT(&htimPeriodic) != HAL_OK)
		alarm_led = 1;

}

void Pulsar::InputCaptureTimerInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_ClkInitTypeDef clockConfig;
	uint32_t pclkFrequency;

	// �������� ���������� �� ������������� ����������
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();

	/************************** TIM2 GPIO Configuration *******************************/
	/**
		PA0     ------> TIM3_CH1
	*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/************************* ������� ��������� ������� ****************************/
	__HAL_RCC_TIM2_CLK_ENABLE();
	__TIM2_FORCE_RESET();
	__TIM2_RELEASE_RESET();

	//1 us per tick
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		htimInputCapture.Init.Prescaler = static_cast<uint16_t>((pclkFrequency) / 1000000 - 1);
	else
		htimInputCapture.Init.Prescaler = static_cast<uint16_t>((pclkFrequency * 2) / 1000000 - 1);

	htimInputCapture.Instance = TIM2;
	htimInputCapture.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimInputCapture.Init.Period = 60000;
	htimInputCapture.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htimInputCapture.Init.RepetitionCounter = 0;

	HAL_TIM_IC_Init(&htimInputCapture);

	/********************** ��������� �������� ������ ������� *************************/

	TIM_IC_InitTypeDef sConfigIC;

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 1;

	if (HAL_TIM_IC_ConfigChannel(&htimInputCapture, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
		alarm_led = 1;

	/************************* ��������� ���������� *********************************/

	__HAL_TIM_CLEAR_IT(&htimInputCapture, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htimInputCapture, TIM_IT_CC1);

	NVIC_SetPriority(TIM2_IRQn, 0);
	NVIC_EnableIRQ(TIM2_IRQn);

	//if (HAL_TIM_Base_Start_IT(&htimInputCapture) != HAL_OK)
	//	alarm_led = 1;

	//if (HAL_TIM_IC_Start_IT(&htimInputCapture, TIM_CHANNEL_1) != HAL_OK)
	//	alarm_led = 1;
}

extern "C" void TIM1_UP_IRQHandler(void)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	HAL_TIM_IRQHandler(pulsarInstance.GetTimPeriodicHandler());
}

extern "C" void TIM2_IRQHandler(void)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	HAL_TIM_IRQHandler(pulsarInstance.GetTimInputCaptureHandler());
}

extern "C" void TIM3_IRQHandler(void)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	HAL_TIM_IRQHandler(pulsarInstance.GetTimAccurateHandler());
}

//�������� ����� ���������� ������� ��������
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	TIM_OC_InitTypeDef sConfigOC;

	if (htim->Instance == TIM1)
	{	
		//������ ���������� ������ 20 us
		HAL_TIM_OC_Start_IT(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_4);

		//������ ������
		HAL_TIM_OC_Start_IT(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_1);

		//������ ������������ ������ ��� �������� �������
		HAL_TIM_OC_Start_IT(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_3);

		return;
	}

	if (htim->Instance == TIM2)
	{
		process_led = !process_led;

		return;
	}

	if (htim->Instance == TIM3)
	{
		if (htim->Channel == TIM_CHANNEL_1) {}

		return;
	}
}

//Callback ���������� � ������� ���������
extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	static Pulsar& pulsarInstance = Pulsar::Instance();

	if (htim->Instance == TIM3)
	{
		static uint8_t second_ch_state = 0;
		TIM_OC_InitTypeDef sConfigOC;

		//�������� �� ������ �� ������� ��������� ����������
		switch (htim->Channel)
		{
		case HAL_TIM_ACTIVE_CHANNEL_1: //����� ��������-������
		{
			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_2: //����� ��������-�����
		{
			
			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_3: //������� ������� � ������� ��������� CH1, CH2
		{
			HAL_TIM_OC_Stop(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_1);
			HAL_TIM_OC_Stop(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_2);

			break;
		}

		case HAL_TIM_ACTIVE_CHANNEL_4:	//������ �������� �����
		{

			HAL_TIM_OC_Start(pulsarInstance.GetTimAccurateHandler(), TIM_CHANNEL_2);

			//��������� ���������� �� ������� �������� �������
			HAL_TIM_Base_Start_IT(pulsarInstance.GetTimInputCaptureHandler());


			break;
		}

		}
	}
}

TIM_HandleTypeDef Pulsar::htimPeriodic;
TIM_HandleTypeDef Pulsar::htimAccurate;
TIM_HandleTypeDef Pulsar::htimInputCapture;