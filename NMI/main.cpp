#include "stdafx.h"
#include "driver_unload.h"
#include "KernelBase.h"
#include<intrin.h>


using  HalSendNmi_t = ULONG_PTR (*)(KAFFINITY* Affinity);
using  HalpApic1WriteIcr_t = ULONG_PTR(*)(int a1,int a2);
PVOID NmiCtx = nullptr;

ULONG_PTR KernelBase = NULL;

//Hard Signature! in hal.dll
HalSendNmi_t HalSendNmi = (HalSendNmi_t)0xfffff8071a8aaf60;
HalpApic1WriteIcr_t HalpApic1WriteIcr = (HalpApic1WriteIcr_t)0xfffff8071a87fa90;

//intel volume3 p395
struct x1Apic
{
		union
		{
			struct {
				ULONG Vector : 8;
				ULONG DeliveryMode : 3;
				ULONG DestinationMode : 1;
				ULONG DeliveryStatus : 1;
				ULONG Reserved3 : 1;
				ULONG Level : 1;
				ULONG TriggerMode : 1;
				ULONG Reserved1 : 2;
				ULONG DestinationShorthand : 2;
				ULONG Reserved2 : 12;
			};
			int value;
		}low;
		union {
			struct {
				ULONG Reserved4 : 24;
				ULONG DestinationField : 8;
			};
			int value;
		}high;
};


BOOLEAN
NmiCb(
	_In_opt_ PVOID Context,
	_In_ BOOLEAN Handled
)
{
	Log("NMI test\n");

	ULONG_PTR t = 0;
	__vmx_vmread(0x00002800, &t);

	return true;
}

ULONG_PTR
ipiCall(
	_In_ ULONG_PTR Argument
)
{
	Log("ipicall\n");

	//force vm-exit


		int a[4] = {};
		__cpuid(a, 0);
	
	return true;
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath)
{
	DriverObject->DriverUnload = DriverUnload;

	KernelBase = GetKernelBase();

	NmiCtx = KeRegisterNmiCallback(NmiCb, nullptr);

	x1Apic apic{};
	apic.low.DeliveryMode = 4;//100b NMI
	apic.low.DestinationMode = 1;
	apic.low.DestinationShorthand = 3;//11b excluding self     10 all processors

	KeIpiGenericCall(ipiCall, 0);

	//
	//how to trigger Nmi Interrupt?
	//
	HalpApic1WriteIcr(apic.high.value, apic.low.value);

	//ed fffff807`19f7e7a4 90909090
	//eb fffff807`19f7e7a4+4 90
	
	
	return STATUS_SUCCESS;
}



















