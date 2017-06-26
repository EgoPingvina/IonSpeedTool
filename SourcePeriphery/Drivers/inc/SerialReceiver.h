#pragma once

#include "mbed.h"
#include "MessageReceiver.h"

class SerialReceiver : 
	public MessageReceiver<uint8_t>
{
public:

	SerialReceiver(	PinName		RXpin, 
					uint16_t	QueueSize);

	//���������� �������� �����
	void SetBaud(uint32_t baud);

	//���������� ������ ������� �����
	void SetFormat(
				uint8_t bits,
				SerialBase::Parity parity,
				uint8_t StopBits
	);

	//������ ������
	void Run();

private:

	//Callback ������ ������� ������� �� UART
	void rx_event_cb();

	Serial serial;

};