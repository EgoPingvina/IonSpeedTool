#pragma once

#include <cmsis_os.h>
#include <CallbackT.h>

class TaskT
{
public:
	TaskT(Action<> callback, uint32_t stakSize = configMINIMAL_STACK_SIZE, osPriority priority = osPriorityNormal, const char *name = "namee", bool run = false);
	TaskT(uint32_t stakSize = configMINIMAL_STACK_SIZE, osPriority priority = osPriorityNormal, const char *name = "namee");

	void Attach(Action<> && handler);

    void Attach(void(*fptr)(void));

    template<class TObject>
    void Attach(TObject* object, void (TObject::*memberPointer)(void))
    {
        this->handler.Attach(object, memberPointer);
    }

	osStatus SetPriority(osPriority priority);
	osPriority GetPriority() const;
	
	const char* GetName() const;
	
	osThreadId* GetHandle();
	
	osStatus Run();
	osStatus Suspend();
	osStatus Resume();
	osStatus Terminate();

	bool IsRunned();
private:
	osThreadDef_t params;
	osThreadId 	  handle;
    Action<>	  handler;
    char name[5];
	bool isRunned;

	static void Start(const void * param);
};