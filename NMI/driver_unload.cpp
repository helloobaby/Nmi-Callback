#include"driver_unload.h"

extern PVOID NmiCtx;

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	if (NmiCtx)
		KeDeregisterNmiCallback(NmiCtx);
}