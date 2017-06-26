#pragma once

#include <stm32f1xx.h>
#include <mbed.h>

#pragma region consts

#define TIM_Period						TIM2
#define TIM_Period_IRQ					TIM2_IRQn
#define TIM_Period_Enable				__TIM2_CLK_ENABLE
#define TIM_Period_IRQ_Handler			TIM2_IRQHandler
#define TIM_Period_FORCE_RESET			__TIM2_FORCE_RESET
#define TIM_Period_RELEASE_RESET		__TIM2_RELEASE_RESET

#define TIM_Durability					TIM3
#define TIM_Durability_IRQ				TIM3_IRQn
#define TIM_Durability_Enable			__TIM3_CLK_ENABLE
#define TIM_Durability_IRQ_Handler		TIM3_IRQHandler
#define TIM_Durability_FORCE_RESET		__TIM3_FORCE_RESET
#define TIM_Durability_RELEASE_RESET	__TIM3_RELEASE_RESET

#pragma endregion

class PulseGenerator
{
public:
	// ���������� ������� ���� ������� �������
	void PeriodTickHandler(void);

	// ���������� ������� ���� ������� ������������
	void DurabilityTickHandler(void);

	// ��������� ����������
	static PulseGenerator& Instance();

	/// <summary>
	/// ��������� ���������� ���������(���������� ������������)
	/// </summary>
	/// <param="period">������������ �������� � ���</param>
	/// <param="durability">������������ �������� � ������� ����� ���</param>
	void Initialize(int32_t period, float durability);

private:
	// ����������� ���������
	static PulseGenerator self;

	/* ������ ������� ������� ������ ���� ������, ��� Pulse. �� ��
	 * ����� 1,5 ���, ������� ������ 25 ���(���� ������� 0,5 ���)
	 */
	const int32_t magicDurabilityPeriod = 50;

	/// <summary>
	/// ������ ������� (��������� 100 ���)
	/// </summary>
	TIM_HandleTypeDef timerPeriod;

	/// <summary>
	/// ������ ������������ (��������� 0,5 ���)
	/// </summary>
	TIM_HandleTypeDef timerDurability;

	// ������� �����
	DigitalOut lifeIndicator = { PA_4 };

	// ��� �� ������ ������������������
	bool isInitialized = false;

	// ����������� �� ���������
	PulseGenerator() = default;

	// ����������� ����� (������������)
	PulseGenerator(const PulseGenerator&) = default;

	// ���������� (������������)
	~PulseGenerator() = default;
};