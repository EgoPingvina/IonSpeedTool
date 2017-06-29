#include "cmsis_os.h"

#include "GlobalConfig.h"
#include "Rcc.hpp"

#include "PulseGenerator.h"

#include "Pulsar.h"


int main()
{
	SystemClock_Config();

	Pulsar& pulsar = Pulsar::Instance();
	pulsar.Initialize();

	osKernelStart();
}