#pragma once

#include "mbed.h"
#include "MessageReceiver.h"

class SerialReceiver : 
	public MessageReceiver<uint8_t>
{
public:

	SerialReceiver(	PinName		RXpin, 
					uint16_t	QueueSize);

	//Установить скорость порта
	void SetBaud(uint32_t baud);

	//Установить формат даннных порта
	void SetFormat(
				uint8_t bits,
				SerialBase::Parity parity,
				uint8_t StopBits
	);

	//Запуск приема
	void Run();

private:

	//Callback приема каждого символа по UART
	void rx_event_cb();

	Serial serial;

};