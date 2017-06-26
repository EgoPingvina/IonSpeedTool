#pragma once

#include "cmsis_os.h"
#include "mbed.h"

namespace RtosServices
{

class OsTimer
{
public:

	OsTimer(os_timer_type timer_type);
	~OsTimer();

	void Attach(void(*fPointer)(void));
	
	template<typename TObject>
	void Attach(TObject* obj, void (TObject::*memberFunction)(void), uint32_t delay_ms)
	{
		_timer_handler.attach(obj, memberFunction);
			_delay_ms = delay_ms;
	}

	void Start();
	void Stop();

private:

	static void StaticTimerCB(void const * param);

	osTimerDef_t		_timer_def;
	osTimerId			_timer_id;
	os_timer_type		_timer_type;
	FunctionPointer		_timer_handler;
	uint32_t			_delay_ms;
};

} //namespace RtosServices
