#include "SerialReceiver.h"


SerialReceiver::SerialReceiver(PinName RXpin, uint16_t QueueSize)
	: MessageReceiver(QueueSize)
	, serial(NC, RXpin)
{
	ThreadCreate("", osPriorityNormal, 500);
}

void SerialReceiver::Run()
{
	serial.attach(this, &SerialReceiver::rx_event_cb);
}

void SerialReceiver::SetBaud(uint32_t baud)
{
	serial.baud(baud);
}

void SerialReceiver::SetFormat(
	uint8_t bits, 			
	SerialBase::Parity parity, 
	uint8_t StopBits)
{
	serial.format(bits, parity, StopBits);
}

void SerialReceiver::rx_event_cb()
{
	MessageSend(serial.getc());
}