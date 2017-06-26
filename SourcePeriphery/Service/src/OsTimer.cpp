#include "OsTimer.h"

namespace RtosServices
{

	OsTimer::OsTimer(os_timer_type timer_type)
		: _timer_type(timer_type)
	{
		_timer_def.ptimer = OsTimer::StaticTimerCB;

		_timer_id = osTimerCreate(&_timer_def, _timer_type, this);

		if (_timer_id == nullptr)
			mbed_assert_internal("Timer handler has nil value", __FILE__, __LINE__);
	}

	OsTimer::~OsTimer()
	{
		osTimerDelete(_timer_id);
	}

	void OsTimer::StaticTimerCB(void const * param)
	{
		OsTimer* _this = static_cast<OsTimer*>(pvTimerGetTimerID(const_cast<osTimerId >(param)));

		_this->_timer_handler.call();
	}

	void OsTimer::Start()
	{
		if (_timer_id != nullptr)
			if (static_cast<bool>(_timer_handler))
				osTimerStart(_timer_id, _delay_ms);
	}

	void OsTimer::Stop()
	{
		if (_timer_id != nullptr)
			osTimerStop(_timer_id);
	}

	void OsTimer::Attach(void(*fPointer)(void))
	{
		_timer_handler.attach(fPointer);
	}

}