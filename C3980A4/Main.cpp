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

HANDLE port;
HANDLE timerThread;
HWND hwnd;
boolean connectMode = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
VOID startTimer();



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	PAINTSTRUCT paintstruct;
	HDC hdc = BeginPaint(hwnd, &paintstruct);

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	Wcl.hIconSm = NULL;
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	Wcl.lpszClassName = programName;

	Wcl.lpszMenuName = "commandMenu";
	Wcl.cbClsExtra = 0;
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;


	hwnd = CreateWindow(programName, programName, WS_OVERLAPPEDWINDOW, 10, 10, windowWidth, windowHeight, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}



VOID startTimer()
{
	SetTimer(hwnd, TIMER_TEST, TEST_TIMEOUT, (TIMERPROC)NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	DWORD threadId;

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case (MENU_CONNECT):
			connectMode = true;

			if ((port = CreateFile("com1", GENERIC_READ | GENERIC_WRITE, 0,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
				== INVALID_HANDLE_VALUE)
			{
				MessageBox(NULL, "Error opening COM port:", "", MB_OK);
				return FALSE;
			}

			break;
		case (MENU_DISCONNECT):
			connectMode = false;
			break;
		case (MENU_QUIT):
			
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &paintstruct);
		break;
	case WM_DESTROY:
		
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
