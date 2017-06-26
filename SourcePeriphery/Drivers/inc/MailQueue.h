#pragma once

#include "type_traits"
#include <string.h>

#include "cmsis_os.h"

template <typename T>
class MailQueue
{
public:
	using Store_t = typename std::remove_const<
		typename std::remove_reference<T >::type>::type;

	MailQueue(size_t qsize);
	~MailQueue() { }

	osStatus MailPut(const Store_t* elem);
	Store_t MailGet(uint32_t millisec = osWaitForever);
	void MailGet(Store_t * destination, uint32_t millisec = osWaitForever);

private:

	osMailQId 		mailQ_id;
	os_mailQ_cb*	mailQ_cb;
	os_mailQ_def	mailQ_def;

	uint16_t		size;
};

template <typename T>
MailQueue<T>::MailQueue(size_t qsize)
	: mailQ_cb(NULL)
	, size(0)
{
		mailQ_def.queue_sz 	= qsize;
		mailQ_def.item_sz 	= sizeof(Store_t);
		mailQ_def.cb 		= &mailQ_cb;
		
		mailQ_id = osMailCreate(&mailQ_def, NULL);
}

template <typename T>
osStatus MailQueue<T>::MailPut(const Store_t* elem)
{
	//Выделить память в очереди под новый объект
	Store_t* dest = static_cast<Store_t*>(osMailAlloc(mailQ_id, osWaitForever));

	if(dest != NULL)
	{
		*dest = *elem; //Скопировать содержимое элемента в пулл памяти очереди

		++this->size;

		return osMailPut(mailQ_id, dest);
	}
	else return osErrorOS;
}

template <typename T>
typename MailQueue<T>::Store_t MailQueue<T>::MailGet(uint32_t millisec)
{
	osEvent recieve = osMailGet(mailQ_id, millisec);

	Store_t retval = *static_cast<Store_t*>(recieve.value.p);
	
	osStatus temp = osMailFree(mailQ_id, recieve.value.p);

	--this->size;

	return retval;
}

template<class T>
void MailQueue<T>::MailGet(Store_t * destination, uint32_t millisec)
{
	osEvent recieve = osMailGet(mailQ_id, millisec);

	*destination = *static_cast<Store_t*>(recieve.value.p);

	--this->size;

	osStatus temp = osMailFree(mailQ_id, recieve.value.p);
}