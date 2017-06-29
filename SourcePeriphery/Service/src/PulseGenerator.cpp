#include "PulseGenerator.h"

#pragma region private

/** Configure pins as
* Analog
* Input
* Output
* EVENT_OUT
* EXTI
*/
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin		= GPIO_PIN_6;
	GPIO_InitStruct.Mode	= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed	= GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

#pragma endregion

//extern "C" void TIM_Period_IRQ_Handler()
//{
//	PulseGenerator::Instance().PeriodTickHandler();
//}

//extern "C" void TIM_Durability_IRQ_Handler()
//{
//	PulseGenerator::Instance().DurabilityTickHandler();
//}

// ������������� ����������-��������
PulseGenerator PulseGenerator::self;

void PulseGenerator::PeriodTickHandler()
{
	if (__HAL_TIM_GET_FLAG(&this->timerPeriod, TIM_FLAG_UPDATE) != RESET)
		if (__HAL_TIM_GET_IT_SOURCE(&this->timerPeriod, TIM_FLAG_UPDATE) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&this->timerPeriod, TIM_FLAG_UPDATE);

			// ��������� ������ ������������ ��������
			HAL_TIM_OC_Start_IT(&this->timerDurability, TIM_CHANNEL_1);

			// ����� �������� �����
			this->lifeIndicator = !this->lifeIndicator;
		}
}

void PulseGenerator::DurabilityTickHandler()
{
	if (__HAL_TIM_GET_FLAG(&this->timerPeriod, TIM_FLAG_UPDATE) != RESET)
		if (__HAL_TIM_GET_IT_SOURCE(&this->timerPeriod, TIM_FLAG_UPDATE) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&this->timerPeriod, TIM_FLAG_UPDATE);

			// ������������� ������ �� ����������� ���������� ����
			HAL_TIM_OC_Stop_IT(&this->timerDurability, TIM_CHANNEL_1);
			HAL_TIM_Base_Stop_IT(&this->timerPeriod);
		}
}

PulseGenerator& PulseGenerator::Instance()
{
	return self;
}

void PulseGenerator::Initialize(int32_t period, float durability)
{
	MX_GPIO_Init();

	// ��������� ������������� �������
	RCC_ClkInitTypeDef clockConfig;
	// ������� ���� (������ �� ������� ���)
	uint32_t pclkFrequency;

	// �������� ���������� �� ������������� ����������
	HAL_RCC_GetClockConfig(&clockConfig, &pclkFrequency);
	pclkFrequency = HAL_RCC_GetPCLK1Freq();

#pragma region ������ �������

	// �������� ������
	TIM_Period_Enable();

	// ����� �������
	TIM_Period_FORCE_RESET();
	TIM_Period_RELEASE_RESET();

	this->timerPeriod.Instance = TIM_Period;

	/* ��������� ������������:
	* TIMxCLK = PCLKx, ����� ������������ APB = 1, ����� TIMxCLK = 2 * PCLKx
	*/
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		this->timerPeriod.Init.Prescaler		= (uint16_t)((pclkFrequency) / 10000) - 1;
	else
		this->timerPeriod.Init.Prescaler		= (uint16_t)((pclkFrequency * 2) / 10000) - 1;

	this->timerPeriod.Init.CounterMode			= TIM_COUNTERMODE_UP;		// ��� ���� �������������� �������
	this->timerPeriod.Init.Period				= period;					// �������� ������ �������
	this->timerPeriod.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	this->timerPeriod.Init.RepetitionCounter	= 0;						// ����� ������������ ���������� ������� ���������� � 0

	HAL_TIM_Base_Init(&this->timerPeriod);

	HAL_TIM_Base_Start_IT(&this->timerPeriod);

	// ������������ ��������� ��� ���������� �������
	NVIC_SetPriority(TIM_Period_IRQ, 0);

	// ��������� ���� ������������ ����������
	__HAL_TIM_CLEAR_IT(&this->timerPeriod, TIM_FLAG_UPDATE);

	// ��������� ���������� �������
	NVIC_EnableIRQ(TIM_Period_IRQ);

#pragma endregion

#pragma region ������ ������������ ��������

	// �������� ������
	TIM_Durability_Enable();

	// ����� �������
	TIM_Durability_FORCE_RESET();
	TIM_Durability_RELEASE_RESET();

	this->timerDurability.Instance = TIM_Durability;

	/* ��������� ������������:
	* TIMxCLK = PCLKx, ����� ������������ APB = 1, ����� TIMxCLK = 2 * PCLKx
	*/
	if (clockConfig.APB1CLKDivider == RCC_HCLK_DIV1)
		this->timerDurability.Init.Prescaler		= (uint16_t)((pclkFrequency) / 2000000) - 1;
	else
		this->timerDurability.Init.Prescaler		= (uint16_t)((pclkFrequency * 2) / 2000000) - 1;

	this->timerDurability.Init.CounterMode			= TIM_COUNTERMODE_UP;			// ��� ���� �������������� �������
	this->timerDurability.Init.Period				= this->magicDurabilityPeriod;	// �������� ������ �������
	this->timerDurability.Init.ClockDivision		= TIM_CLOCKDIVISION_DIV1;
	this->timerDurability.Init.RepetitionCounter	= 0;							// ����� ������������ ���������� ������� ���������� � 0

	HAL_TIM_OC_Init(&this->timerDurability);

#pragma region Output Capture Settings

	TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode		= TIM_OCMODE_PWM1;
	sConfigOC.Pulse			= durability;
	sConfigOC.OCPolarity	= TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode	= TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&this->timerDurability, &sConfigOC, TIM_CHANNEL_1);

#pragma endregion

	// ������������ ��������� ��� ���������� �������
	NVIC_SetPriority(TIM_Durability_IRQ, 0);

	// ��������� ���� ������������ ����������
	__HAL_TIM_CLEAR_IT(&this->timerDurability, TIM_FLAG_UPDATE);

	// ��������� ���������� �������
	NVIC_EnableIRQ(TIM_Durability_IRQ);

#pragma endregion

	// ������ �������, ��� ������ ������������������ � ������� �������
	this->isInitialized = true;
}