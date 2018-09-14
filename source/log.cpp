#include <stdio.h>
#include <stdarg.h>
#include "nx/string.h"
#include "socket.h"
#include "log.h"

Array<PrintHook> g_hooks;

void registerPrintHook(PrintHook func)
{
	g_hooks.push(func);
}


FILE* f = NULL;

CircularBuffer<string, 0x100> g_printLog;
CircularBuffer<string, 0x100>& printLog() { return g_printLog; }

char printBuffer[0x1000];

extern Socket* currentSocket;

FILE* file()
{
	if (!f)
	{
#ifdef _MSC_VER
		f = stdout;
#else
		f = stdout;
		//f = fopen("/nutd.log", "a");
#endif
	}

	return f;
}


void print(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void fatal(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void error(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void warning(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}

void debug(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(printBuffer, format, args);
	printf("%s", printBuffer);
	printLog().push(printBuffer);

	va_end(args);
	fflush(f);

	for (auto& f : g_hooks)
	{
		f();
	}
}