/*
PROGRAM HEADER HERE
*/
#define STRICT
#include <Windows.h>
#include "Header.h"
#include "Main.h"
#include "Receive.h"
#include "Transmit.h"
#include "Print.h"

char programName[] = "C3980 A4";
LPCSTR lpszCommName = "COM1";

HANDLE timerThread;
HWND hwnd;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

VOID startTimer()
{
	SetTimer(hwnd, TIMER_TEST, TEST_TIMEOUT, (TIMERPROC)NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;

	switch (Message)
	{
	case WM_COMMAND:
		break;
	case WM_CHAR:
		break;
	case WM_TIMER:
		break;
	case WM_PAINT: 
		break;
	case WM_DESTROY:
		break;
	default:
		break;
	}
}