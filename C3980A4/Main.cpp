/*
PROGRAM HEADER HERE
*/
#define STRICT
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Header.h"
#include "Main.h"
#include "Receive.h"
#include "Transmit.h"
#include "Print.h"


char programName[] = "C3980 A4";
char filePathBuffer[128];
LPCSTR lpszCommName = "COM1";

HANDLE port;
HANDLE timerThread;

//OpenFile Global Variables
HANDLE fileHandle;
OPENFILENAME ofn;
char inputFileBuffer[2048];
OVERLAPPED ol;
DWORD g_BytesTransferred;

HANDLE idleThread;

HWND hwnd;
boolean connectMode = false;

//Function Headers
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
VOID startTimer();
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
VOID Idle();
VOID Acknowledge();
void bidForLine();
void sendEnq();

extern bool timeout = false;
extern bool linkedReset = false;
//FILE * outputBuffer = NULL;
extern char * inputBuffer = NULL;
HANDLE hComm;


//bool timeout;
//char* inputBuffer;



//Initialize OPENFILENAME



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	PAINTSTRUCT paintstruct;
	HDC hdc = BeginPaint(hwnd, &paintstruct);
	
	FILE * s;
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

	//sets memory of OPENFILEDIALOG
	memset(&ofn, 0, sizeof(ofn)); ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePathBuffer;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filePathBuffer);
	ofn.lpstrFilter = "All\0*.*\Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ol = { 0 };

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
	timeout = false;
	SetTimer(hwnd, TIMER_TEST, TEST_TIMEOUT, TimerProc);
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	timeout = true;
	//MessageBox(hwnd, "test", "", MB_OK);
	//KillTimer(hwnd, TIMER_TEST);
}

VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
{
	printf(TEXT("Error code:\t%x\n"), dwErrorCode);
	printf(TEXT("Number of bytes: \t%x\n"), dwNumberOfBytesTransferred);
	g_BytesTransferred = dwNumberOfBytesTransferred;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	DWORD threadId;

	DCB deviceContext;
	COMMTIMEOUTS ct = { 0 };
	char settings[] = "9600,8,N,1";

	//File Input variables
	DWORD dwBytesRead = 0;

	switch (Message)
	{
	case WM_CREATE:
		if ((idleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Idle, NULL, 0, 0)) == INVALID_HANDLE_VALUE) {
			/* handle error */
			return 0;
		} /* end if (error creating read thread) */
		break;
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

			ct.ReadIntervalTimeout = MAXDWORD;
			SetCommTimeouts(port, &ct);

			//setup device context settings
			if (!SetupComm(port, 8, 8)) {
				MessageBox(hwnd, "setupComm failed", "", NULL);
				break;
			}

			if (!GetCommState(port, &deviceContext)) {
				MessageBox(hwnd, "getCommState failed", "", NULL);
				break;
			}
			/*
			if (!BuildCommDCB(settings, &deviceContext)) {
				MessageBox(hwnd, "buildCommDCB failed", "", NULL);
				break;
			}*/

			if (!SetCommState(port, &deviceContext)) {
				MessageBox(hwnd, "setCommState failed", "", NULL);
				break;
			}
		
			Receive();

			/*
			char a[518];
			memset(a, 'a', 518);
			inputBuffer = a;
			print();
			*/
			break;
		case (MENU_DISCONNECT):
			connectMode = false;
			break;
		case (MENU_QUIT):
			PostQuitMessage(0);
			break;
		case (MENU_FILE):
			if (GetOpenFileName(&ofn) == TRUE)
			{
				fileHandle = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

				if (fileHandle == INVALID_HANDLE_VALUE) {
					MessageBox(hwnd, "I could not open the file master.", "", NULL);
				}

				if (FALSE == ReadFileEx(fileHandle, inputFileBuffer, sizeof(inputFileBuffer) - 1, &ol, FileIOCompletionRoutine))
				{
					//DisplayError(TEXT("ReadFile"));
					MessageBox(hwnd, "File Read Error, master.", "", NULL);
				}
				//if (dwBytesRead > 0 && dwBytesRead <= sizeof(inputFileBuffer) - 1)
				//{
					sprintf_s(inputFileBuffer, "%s", fileHandle);
				//}

			}
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &paintstruct);
		break;
	case WM_CHAR:
		sendEnq();
		break;
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

VOID Idle()
{
	if (linkedReset)
	{
		startTimer();
		while (!timeout)
		{
			MessageBox(hwnd, "idle", "", MB_OK);
			if (inputBuffer != NULL && inputBuffer[1] == 5)
			{
				Acknowledge();
			}
			if (timeout)
			{
				linkedReset = false;
			}
		}
	}
	else
	{
		while (true)
		{
			if (inputBuffer != NULL && strlen(inputBuffer) > 0)
			{
				if (inputBuffer[1] == 5)
				{
					Acknowledge();
				}
			}
			/*
			if (strlen(outputBuffer) > 0)
			{
			control[0] = 22;
			control[1] = 5;
			//send control frame
			bidForLine();
			}
			*/
		}
	}
}

VOID Acknowledge()
{
	control[0] = 22;
	control[1] = 6;
	//put control frame in output buffer
	send(hComm);
	//Receive();
}

void sendEnq()
{
	DWORD dwBytesWritten;
	char enq[1];
	enq[0] = 5;
	bool bwrite = WriteFile(hComm, (LPCVOID)enq, (DWORD)strlen(enq), &dwBytesWritten, NULL);
}

void bidForLine()
{
	startTimer();
	while (timeout != true)
	{
		MessageBox(hwnd, "bid", "", MB_OK);
		if (inputBuffer != NULL)
		{
			if (inputBuffer[1] == 6)
			{
				timeout = true;
				//prepareToSend(outputBuffer, hComm);
			}
		}
	}
	linkedReset = true;
	return;
}
