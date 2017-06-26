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
	// Обработчик события тика таймера периода
	void PeriodTickHandler(void);

	// Обработчик события тика таймера длительности
	void DurabilityTickHandler(void);

	// Получение экземпляра
	static PulseGenerator& Instance();

	/// <summary>
	/// Настройка генератора импульсов(внутренние тактирования)
	/// </summary>
	/// <param="period">Длительность импульса в мкс</param>
	/// <param="durability">Длительность импульса в десятых долях мкс</param>
	void Initialize(int32_t period, float durability);

private:
	// Собственный экземпляр
	static PulseGenerator self;

	/* Период второго таймера должен быть больше, чем Pulse. По ТЗ
	 * пульс 1,5 мкс, поэтому делаем 25 мкс(цена деления 0,5 мкс)
	 */
	const int32_t magicDurabilityPeriod = 50;

	/// <summary>
	/// Таймер периода (кратность 100 мкс)
	/// </summary>
	TIM_HandleTypeDef timerPeriod;

	/// <summary>
	/// Таймер длительности (кратность 0,5 мкс)
	/// </summary>
	TIM_HandleTypeDef timerDurability;

	// Признак жизни
	DigitalOut lifeIndicator = { PA_4 };

	// Был ли таймер проинициализирован
	bool isInitialized = false;

	// Конструктор по умолчанию
	PulseGenerator() = default;

	// Конструктор копий (заблокирован)
	PulseGenerator(const PulseGenerator&) = default;

	// Деструктор (заблокирован)
	~PulseGenerator() = default;
};