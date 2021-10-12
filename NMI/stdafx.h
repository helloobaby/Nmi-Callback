#pragma once 
#define _NO_CRT_STDIO_INLINE 
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(push, 0)
#include <ntifs.h>
#include <ntdef.h>
#include <ntddk.h>
#include <ntstatus.h>
#include <cstdio>
#pragma warning(pop)

inline ULONG Log(const char* format, ...)
{
	char buffer[256];

	va_list ap;
	__va_start(&ap, format);
	vsprintf(buffer, format, ap);

	return DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, buffer);
}

//
//²»ÒªÓÃ__FUNCTION__
//
#define TRACE(x) Log("[TRACE]%s %d\n",__func__,__LINE__)
