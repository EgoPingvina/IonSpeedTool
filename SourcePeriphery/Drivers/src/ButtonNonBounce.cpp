#include "ButtonNonBounce.h"

ButtonNonBounce::ButtonNonBounce(PinName pin)
	: button(pin)
	, timer(osTimerOnce)
	, button_state(ButtonStates::Released)
{
	timer.Attach(this, &ButtonNonBounce::TimerCB, 30);

	button.fall(this, &ButtonNonBounce::ButtonFallEventCB);
	button.mode(PinMode::PullUp);
}

void ButtonNonBounce::ButtonRiseEventCB(void)
{
	timer.Start();
}

void ButtonNonBounce::ButtonFallEventCB(void)
{
	timer.Start();
}

void ButtonNonBounce::ButtonDummyCB(void)
{
}



void ButtonNonBounce::TimerCB(void)
{

	switch (button_state)
	{
		case Released:
		{
			if (!button.read())
			{
				if (static_cast<bool>(PressedFp))
					PressedFp.call();

				button_state = Pressed;
				button.fall(this, &ButtonNonBounce::ButtonDummyCB);
				button.rise(this, &ButtonNonBounce::ButtonRiseEventCB);
				button.mode(PinMode::PullUp);
			}
			break;
		}

		case Pressed:
		{
			if (button.read())
			{
				if (static_cast<bool>(ReleasedFp))
					ReleasedFp.call();

				button_state = Released;
				button.rise(this, &ButtonNonBounce::ButtonDummyCB);
				button.fall(this, &ButtonNonBounce::ButtonFallEventCB);
				button.mode(PinMode::PullUp);
			}

			break;
		}
	}

}
