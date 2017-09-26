#include "main.h"
#include "Utils.h"

extern void DeviceInit(void);
extern size_t GetCommand(uint8_t * const buff);
extern void CommHandler(const TCommReply * const comm);
extern void ButtonsHandler(void);
extern uint8_t GetHwVersion(void);
extern void SetSysClockHsiTo24(void);

/* Версии:
	 1 - версия данной структуры
	 0 - версия интерфейса
	 0 - версия аппаратуры
	 0 - версия ВПО */
TBlockInfo BlockVersions =
{
	1,
	0,
	0,
	1
};

int main(void)
{
	SetSysClockHsiTo24();
	SaveLastRstReason();
	DeviceInit();
	InitQueues();
	BlockVersions.HwVersion = GetHwVersion();
	TCommReply Command = {0};
	
	while(1)
	{
		IWDG_ReloadCounter();		// Reset watchdog counter
		GetCommand((uint8_t*)&Command);
		CommHandler(&Command);
		ButtonsHandler();
	}
}
