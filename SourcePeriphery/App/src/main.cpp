#include "cmsis_os.h"

#include "GlobalConfig.h"
#include "Rcc.hpp"

#include "PulseGenerator.h"

Serial pc(PA_2, PA_3);

PulseGenerator& timer = PulseGenerator::Instance();

int main()
{
	SystemClock_Config();

	pc.baud(115200);
	timer.Initialize(1000, 3);

	osKernelStart();
}