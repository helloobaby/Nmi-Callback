#include "stdafx.h"
#include "driver_unload.h"
#include "KernelBase.h"
#include<intrin.h>


using  HalSendNmi_t = ULONG_PTR (*)(KAFFINITY* Affinity);
using  HalpApic1WriteIcr_t = ULONG_PTR(*)(int a1,int a2);
PVOID NmiCtx = nullptr;

ULONG_PTR KernelBase = NULL;
ULONG_PTR CurProcessorNumber;

//Hard Signature! in hal.dll
//.reload hal 
//u hal!HalpApic1WriteIcr
HalpApic1WriteIcr_t HalpApic1WriteIcr = (HalpApic1WriteIcr_t)0xfffff8054628ca90;

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
	ULONG_PTR t = 0;
	__vmx_vmread(0x00002800, &t); // bug check in non vm-root mode

	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, ("[+] nmi callback excute in vm-root mode\n"));


	return true;
}

ULONG_PTR
ipiCall(
	_In_ ULONG_PTR Argument
)
{
	if (KeGetCurrentProcessorNumber() == CurProcessorNumber)
		return true;

	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "[+] current processor number %d\n", KeGetCurrentProcessorNumber());
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "[+] ipi call excute cpuid to force vm-exit\n");

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

	_disable();
	CurProcessorNumber = KeGetCurrentProcessorNumber();
	KeIpiGenericCall(ipiCall, 0);
	_enable();
	//
	//how to trigger Nmi Interrupt?
	//
	HalpApic1WriteIcr(apic.high.value, apic.low.value);
	
	
	return STATUS_SUCCESS;
}



















