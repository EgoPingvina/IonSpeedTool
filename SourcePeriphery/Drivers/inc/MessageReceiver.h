#pragma once

#include <type_traits>

#include "mbed.h"

#include "CallbackT.h"
#include "MailQueue.h"
#include "TaskT.h"

enum class MSStatus
{
	Success = 0,
	ReceiveTaskDontRun = 1
};

template<typename T>
class MessageReceiver
{
	using FPointer = Action<T>;

public:
	MessageReceiver(uint16_t Size, uint16_t stackSize)
		: mail(Size)
		, looper(stackSize)
	{ this->looper.Attach(this, &MessageReceiver::Loop); }

	MSStatus Run(){ return this->looper.Run()== osOK ? MSStatus::Success : MSStatus::ReceiveTaskDontRun; }

	void Attach(FPointer functionPointer) { this->handler = functionPointer; }

	void MessageSend(T elem) { mail.MailPut(&elem); }

private:
	MailQueue<T>	mail;
	FPointer		handler;
	TaskT			looper;

	void Loop()
	{
		typename MailQueue<T>::Store_t rx_elem;

		while (1)
		{
			 mail.MailGet(&rx_elem);

		     handler(rx_elem);
		}
	}
};