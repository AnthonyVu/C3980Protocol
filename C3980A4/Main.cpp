/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Main.cpp - The main functions and the base state for both
--						sending and recieving
--
-- PROGRAM: C3980A4
--
-- FUNCTIONS:
-- int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- VOID startTimer(unsigned int time)
-- VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
-- VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
-- VOID Idle()
-- VOID Acknowledge()
-- VOID sendEnq()
-- VOID bidForLine()
-- DWORD readThread(LPDWORD lpdwParam1)
-- BOOL writeToPort(char* writeBuffer, DWORD dwNumToWrite)
--
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker, Wilson Hu, Anthony Vu, Matthew Shew
--
-- NOTES:
-- The program starts the GUI and will allow the user to connect to the comm port. Connecting will put the
-- program in idle. While in Idle the program waits for the user to add a file or for an Enq to be
-- recieved. Depending on that the program will either start to send or recieve data and control frames.
-- The main.cpp file also handles the threads and common functions. It has a timer to set timeouts and
-- generic read and write functions that are event drivenn using an overlapped IO.
----------------------------------------------------------------------------------------------------------------------*/
#define STRICT
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Header.h"
#include "Main.h"
#include "Receive.h"
#include "Transmit.h"
#include "Print.h"
#include "crc.h"

char programName[] = "C3980 A4";
char filePathBuffer[MAX_FILENAME_SIZE];
LPCSTR lpszCommName = "COM1";
HWND hwnd;
DCB deviceContext;
int printRow = 0;
int printColumn = 0;


//OpenFile Global Variables
HANDLE fileHandle;
OPENFILENAME ofn;
extern char inputFileBuffer[MAX_FILE_SIZE] = {0};
OVERLAPPED ol;
DWORD g_BytesTransferred;

//idle & readThread handles
HANDLE idleThread;
HANDLE readInputBufferThread;
HANDLE port;
HANDLE timerThread;

boolean connectMode = false;

//Main.cpp function headers
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
VOID startTimer(unsigned int time);
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
VOID Idle();
VOID Acknowledge();
void bidForLine();
void sendEnq();
DWORD readThread(LPDWORD);
BOOL writeToPort(char*, DWORD);
VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped);

//bool values used to keep track of timeouts
bool timeout = false;
bool linkedReset = false;

char inputBuffer[518] = { 0 };
bool rvi = false;

//global counters for protocol statistics
size_t numPacketsSent = 0;
size_t numPacketsReceived = 0;
size_t numBitErrors = 0;
size_t numACKSent = 0;
size_t numACKReceived = 0;
size_t numENQSent = 0;
size_t numENQReceived = 0;
size_t numTimeouts = 0;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu, Matthew Shew
--
-- INTERFACE: int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- HINSTANCE hInst: current instance of the program
-- HINSTANCE hprevInstance: handle if any previous instance is running
-- LPSTR lspszCmdParam: command line parameters
--int nCmdShow: a flag that says whether the main application window will be minimized, maximized, or shown normally.
--
-- RETURNS: An int that can be used a status code
--
-- NOTES:
-- Starts the program and allocats memory needed along with setting timeout and comm settings.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	COMMTIMEOUTS ct = { 0 };
	char settings[] = "9600,N,8,1";

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

	if ((port = CreateFile("com1", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
		== INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Error opening COM port:", "", MB_OK);
	}

	ct.ReadIntervalTimeout = 500;
	ct.ReadTotalTimeoutMultiplier = 500;
	ct.ReadTotalTimeoutConstant = 500;
	ct.WriteTotalTimeoutMultiplier = 500;
	ct.WriteTotalTimeoutConstant = 500;
	SetCommTimeouts(port, &ct);

	//setup device context settings
	if (!SetupComm(port, 8, 8)) {
		MessageBox(hwnd, "setupComm failed", "", NULL);
	}

	if (!GetCommState(port, &deviceContext)) {
		MessageBox(hwnd, "getCommState failed", "", NULL);
	}

	if (BuildCommDCB(settings, &deviceContext)) {
		deviceContext.fOutxCtsFlow = FALSE;
		deviceContext.fOutxDsrFlow = FALSE;
		deviceContext.fDtrControl = DTR_CONTROL_DISABLE;
		deviceContext.fOutX = FALSE;
		deviceContext.fInX = FALSE;
		deviceContext.fRtsControl = RTS_CONTROL_DISABLE;
	}
	else {
		MessageBox(hwnd, "buildCommDCB failed", "", NULL);
	}

	if (!SetCommState(port, &deviceContext)) {
		MessageBox(hwnd, "setCommState failed", "", NULL);
	}

	printStaticInfo();
	//updateInfo();

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu, Haley Booker, Anthony Vu, Matthew Shew
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- HWND hwnd: the handle of the window
-- UINT Message: event messages generated by windows
-- WPARAM wParam: the value in the message
-- LPARAM lParam: the value int the message
--
-- RETURNS: The return value is the result of the message processing and depends on the message sent.
--
-- NOTES:
-- Handles windows events. A user can send an RVI, open a file, connect to the comm port or quit the program
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	DWORD threadId;
	DWORD dwBytesRead = 0;

	switch (Message)
	{
	//Switch case to handle menu buttons
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		//Connect menu button pressed, should probably connect before setting connectMode=true
		case (MENU_CONNECT):
			if (!connectMode) {
				connectMode = true;

				if ((idleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Idle, NULL, 0, 0)) == INVALID_HANDLE_VALUE) {
					MessageBox(hwnd, "Could not create idleThread", "", NULL);
				} 
				if ((readInputBufferThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readThread, NULL, 0, 0)) == INVALID_HANDLE_VALUE) {
					MessageBox(hwnd, "Could not create readThread", "", NULL);
				}
				
			}
			break;
		//Quit menu button pressed
		case (MENU_QUIT):
			PostQuitMessage(0);
			break;
		//File menu button pressed
		case (MENU_FILE):
			if (GetOpenFileName(&ofn) == TRUE)
			{
				fileHandle = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

				if (fileHandle == INVALID_HANDLE_VALUE) {
					MessageBox(hwnd, "I could not open the file master.", "", NULL);
				}

				if (!ReadFileEx(fileHandle, inputFileBuffer, sizeof(inputFileBuffer) - 1, &ol, FileIOCompletionRoutine))
				{
					MessageBox(hwnd, inputFileBuffer, "", NULL);
				}
				CloseHandle(fileHandle);
			}
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &paintstruct);
		break;
	case WM_CHAR:
		switch (wParam) {
		case 114:
			rvi = true;
			break;
		}
		break;
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startTimer
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Anthony Vu
--
-- INTERFACE: VOID startTimer(unsigned int time)
-- unsigned int time: the amount of time before timing out in milliseconds
--
-- RETURNS: void
--
-- NOTES:
-- This function resets the timer and calls it using the time specified.
----------------------------------------------------------------------------------------------------------------------*/
VOID startTimer(unsigned int time)
{
	timeout = false;
	SetTimer(hwnd, TIMER_TEST, time, TimerProc);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TimerProc
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Anthony Vu
--
-- VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
-- HWND hwnd: a handle to the window
-- UINT uMsg: the message
-- UINT_PTR idEvent: a pointer to the event
-- DWORD dwTime: the amount of time
--
-- RETURNS: void
--
-- NOTES:
-- The timer function for when a timeout has occured
----------------------------------------------------------------------------------------------------------------------*/
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	//Timeout has occurred
	timeout = true;
	KillTimer(hwnd, TIMER_TEST);
	//numTimeouts++;
	updateInfo(&numTimeouts);

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: FileIOCompletionRoutine
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu
--
-- INTERFACE: VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
-- DWORD dwErrorCode: the varaible for errors
-- DWORD dwNumberOfBytesTransferred: the number of bytes transfered
-- LPOVERLAPPED lpOverlapped: the overlapped structure
--
-- RETURNS: void
--
-- NOTES:
-- This function prints information after an IO event including errors that occur and the number of bytes transfered
----------------------------------------------------------------------------------------------------------------------*/
VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
{
	printf(TEXT("Error code:\t%x\n"), dwErrorCode);
	printf(TEXT("Number of bytes: \t%x\n"), dwNumberOfBytesTransferred);
	g_BytesTransferred = dwNumberOfBytesTransferred;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Idle
--
-- DATE: December 3, 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- INTERFACE: VOID Idle()
--
-- RETURNS: VOID
--
-- NOTES:
-- This function is used to wait for the user to be ready to send data frames to the other user, or for another user to be ask for control
-- of the line to send frames to the user(ourselves). If the user is ready to send data frames, the function will send an enq frame to ask for control
-- of the line. If the user has sent an enq and does not receive and ack, or the user has completed sending 10 frames, the function will wait 2 seconds before
-- they can bid for the line again where another user can begin sending frames to the user(ourselves).
----------------------------------------------------------------------------------------------------------------------*/
VOID Idle()
{
	while (true)
	{
		if (linkedReset != false)
		{
			startTimer(2001);
			while (!timeout)
			{
				if (inputBuffer[0] == SYN && inputBuffer[1] == ENQ)
				{
					Acknowledge();
					linkedReset = false;
					break;
				}
				if (timeout)
				{
					linkedReset = false;
				}
			}
		}
		if (inputBuffer[0] == SYN && inputBuffer[1] == ENQ)
		{
			Acknowledge();
			linkedReset = false;
		}
		else if (strlen(inputFileBuffer) > 0)
		{
			sendEnq();
			bidForLine();
			fprintf(stderr, "asdf");
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Acknowledge
--
-- DATE: December 3, 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Anthony Vu
--
-- INTERFACE: VOID Acknowledge()
--
-- RETURNS: VOID
--
-- NOTES:
-- This function is used to send an ack frame to communicate that the user is ready to receive data frames and then goes to 
-- receive state to listen for the data frames.
----------------------------------------------------------------------------------------------------------------------*/
VOID Acknowledge()
{
	DWORD bytesWritten;
	control[0] = 22;
	control[1] = 6;
	bool write = writeToPort(control, 2);
	Receive();
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendEnq
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker
--
-- INTERFACE: VOID sendEnq()
--
-- RETURNS: VOID
--
-- NOTES:
-- This function is used to send and enq frame to the other user to ask for control of the line to send data frames. 
----------------------------------------------------------------------------------------------------------------------*/
VOID sendEnq()
{
	DWORD dwBytesWritten;
	char enq[2];
	enq[0] = 22;
	enq[1] = 5;
	bool write = writeToPort(enq, 2);
	if (!write) {
		MessageBox(hwnd, "sendEnq failed", "", MB_OK);
	}
	//write success, Increment numENQSent counter
	else {
		//numENQSent++;
		updateInfo(&numENQSent);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: bidForLine
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker
--
-- INTERFACE: VOID bidForLine()
--
-- RETURNS: VOID
--
-- NOTES:
-- This function is used to wait for an acknowledgment from the receiver to communicate that they are ready to receive data. Upon receiving an 
-- ack, data frames can now be sent to the receiver. If the timer ends before the ack has been received the function returns and goes to the idle state
-- with linkReset set to true so the sender cannot bid for the line again instantly.
----------------------------------------------------------------------------------------------------------------------*/
VOID bidForLine()
{
	startTimer(2000);
	while (timeout != true)
	{
		//Received an ACK in inputBuffer
		if (inputBuffer[0] == SYN && inputBuffer[1] == ACK)
		{
			memset(inputBuffer, 0, 518);
			prepareToSend(inputFileBuffer, port);
			linkedReset = true;
			eot = false;
			memset(inputBuffer, 0, 518);
			timeout = true;

			//Receive success, increment numACKReceived
			//numACKReceived++;
			updateInfo(&numACKReceived);

			KillTimer(hwnd, TIMER_TEST);

		}
	}
	return;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readThread
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu
--
-- INTERFACE: DWORD readThread(LPDWORD lpdwParam1)
--						LPDWORD lpdwParam1 - unused thread param
--
-- RETURNS: DWORD - the number of Bytes written to the port
--
-- NOTES:
-- This function is used to listen to the port for received data with event driven flow.
----------------------------------------------------------------------------------------------------------------------*/
DWORD readThread(LPDWORD lpdwParam1)
{
	DWORD nBytesRead = 0;
	DWORD dwEvent, dwError;
	COMSTAT cs;
	OVERLAPPED osReader = { 0 };
	char a[10];

	SetCommMask(port, EV_RXCHAR);

	//temp bool used for read loop
	while (connectMode) {
		if (WaitCommEvent(port, &dwEvent, NULL))
		{
			ClearCommError(port, &dwError, &cs);
			osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (osReader.hEvent == NULL)
			{
				MessageBox(hwnd, "I COULD NOT CREATE HEVENT MASTER", "", NULL);
				return 0;
			}

			if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
			{
				ReadFile(port, inputBuffer, sizeof(inputBuffer), &nBytesRead, &osReader);
				updateInfo(&numPacketsReceived);
			}
		}
		PurgeComm(port, PURGE_RXCLEAR);
	}
	return nBytesRead;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeToPort
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu
--
-- INTERFACE: BOOL writeToPort(char* writeBuffer, DWORD dwNumToWrite)
--					char* writeBuffer - pointer to the char array to be sent through the port
--					DWORD dwNumToWrite - number of Bytes to write to the port
--
-- RETURNS: BOOL - return true if the write to the port was successful, false otherwise
--
-- NOTES:
-- This function is used to send a char array through the port to the receiver.
----------------------------------------------------------------------------------------------------------------------*/
BOOL writeToPort(char* writeBuffer, DWORD dwNumToWrite)
{
	OVERLAPPED osWrite = { 0 };
	DWORD dwWritten;
	DWORD dwRes;
	BOOL result;

	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
	{
		return FALSE;
	}
	//Issue write
	if (!WriteFile(port, writeBuffer, dwNumToWrite, &dwRes, &osWrite))
	{

		if (GetLastError() != ERROR_IO_PENDING)
		{
			result = FALSE;
		}
		else {
			//Write is pending
			dwRes = WaitForSingleObject(osWrite.hEvent, 2000);
			switch (dwRes)
			{
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(port, &osWrite, &dwWritten, FALSE))
					result = FALSE;
				else
					result = TRUE;
				break;
			default:
				result = FALSE;
				break;
			}
		}
	}
	else  //WriteFile succeeded, increment numPacketsSent
	{
		result = TRUE;
		//numPacketsSent++;
		updateInfo(&numPacketsSent);
	}
	return result;
}


