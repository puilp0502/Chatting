#ifndef _DEFAULT_HEADERS
#define _DEFAULT_HEADERS
#include "stdafx.h"
#endif

#include <WinSock2.h>
#include <conio.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")
unsigned int WINAPI Waitforinput(void*);
int main(){
	char* ip = "127.0.0.1";
	HANDLE wait = (HANDLE)_beginthreadex(NULL, 0, Waitforinput,)
}