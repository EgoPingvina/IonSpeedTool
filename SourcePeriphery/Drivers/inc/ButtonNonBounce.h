#pragma once

#include "mbed.h"
#include "OsTimer.h"

using namespace RtosServices;

class ButtonNonBounce
{
public:

	ButtonNonBounce(PinName button_pin);

	template<typename TObject>
	void PressedAttach(TObject* obj, void(TObject::*method)(void))
	{
		PressedFp.attach(obj, method);
	}

	template<typename TObject>
	void ReleasedAttach(TObject* obj, void(TObject::*method)(void))
	{
		ReleasedFp.attach(obj, method);
	}

private:

	void ButtonRiseEventCB(void);
	void ButtonFallEventCB(void);
	void ButtonDummyCB(void);
	void TimerCB(void);

	enum ButtonStates
	{
		Released,
		FallJitter,
		Pressed,
		RiseJitter
	};

	InterruptIn		button;
	OsTimer			timer;

	FunctionPointer	PressedFp;
	FunctionPointer	ReleasedFp;

	ButtonStates	button_state;

};