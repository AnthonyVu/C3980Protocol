/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: InotifyDaemon.c - An application that will monitor a specified
-- directory for file creation/modification.
--
-- PROGRAM: inotd
--
-- FUNCTIONS:
-- void daemonize (void)
-- int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int ProcessFiles (char pathname[MAXPATHLEN])
-- unsigned int GetProcessID (char *process)
--
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- NOTES:
-- The program will monitor a directory that is specified in a configuration file for any type of file
-- modification activity (creation, read/write, deletion). The design uses the “inotify” kernel-level
-- utility to obtain the file system event notification. The “select” system call is used to monitor
-- the watch descriptor (returned from inotify).
--
-- Once select is triggered, the directory under watch is processed to determine the exact type of
-- file activity. Once the created/modified files have been identified, they are moved to a separate
-- archive directory. Before the archival process takes place, the system process table (/proc) is
-- searched to verify that the modifying process is currently active and running.
--
-- Note that the application once invoked, will continue to execute as a daemon.
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

HANDLE port;
HANDLE timerThread;

//OpenFile Global Variables
HANDLE fileHandle;
OPENFILENAME ofn;
extern char inputFileBuffer[MAX_FILE_SIZE] = {0};
OVERLAPPED ol;
DWORD g_BytesTransferred;

//idle & readThread handles
HANDLE idleThread;
HANDLE readInputBufferThread;


boolean connectMode = false;

int printRow = 0;
int printColumn = 0;

//Main.cpp function headers
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
VOID startTimer(unsigned int time);
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
VOID Idle();
VOID Acknowledge();
BOOL Validation(uint32_t receivedCRC, char* input);
void bidForLine();
void sendEnq();
DWORD readThread(LPDWORD);
BOOL writeToPort(char*, DWORD);
VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped);

//bool values used to keep track of timeouts
extern bool timeout = false;
extern bool linkedReset = false;

char inputBuffer[518] = { 0 };
extern bool rvi = false;

//global counters for protocol statistics
size_t numPacketsSent = 0;
size_t numPacketsReceived = 0;
size_t numBitErrors = 0;
size_t numACKSent = 0;
size_t numACKReceived = 0;
size_t numENQSent = 0;
size_t numENQReceived = 0;
size_t numTimeouts = 0;

//char inputBuffer[518] = {0};

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	COMMTIMEOUTS ct = { 0 };
	char settings[] = "9600,N,8,1"; //"9600,N,8,1"

	MSG Msg;
	WNDCLASSEX Wcl;
	PAINTSTRUCT paintstruct;
	HDC hdc = BeginPaint(hwnd, &paintstruct);

	//FILE * s;
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
		//return FALSE;
	}

	ct.ReadIntervalTimeout = 500;
	ct.ReadTotalTimeoutMultiplier = 500;
	ct.ReadTotalTimeoutConstant = 500;
	ct.WriteTotalTimeoutMultiplier = 500;
	ct.WriteTotalTimeoutConstant = 500;
	SetCommTimeouts(port, &ct);

	//setup device context settings
	if (!SetupComm(port, 8, 8)) { //is it supposed to be 8?
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
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	DWORD threadId; //unused?

					//File Input variables
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
					/* handle error */
					//return 0;
					MessageBox(hwnd, "Could not create idleThread", "", NULL);
				} /* end if (error creating read thread) */
				if ((readInputBufferThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readThread, NULL, 0, 0)) == INVALID_HANDLE_VALUE) {
				}
				//if (ResumeThread(readInputBufferThread) == -1) {
				//	MessageBox(hwnd, "could not resume readInputBufferThread, my lord", "", NULL);
				//}
			}
			break;
			//Disconnect menu button pressed
		case (MENU_DISCONNECT):
			connectMode = false;
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
					//DisplayError(TEXT("ReadFile"));
					MessageBox(hwnd, inputFileBuffer, "", NULL);
				}
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
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/
VOID startTimer(unsigned int time)
{
	timeout = false;
	SetTimer(hwnd, TIMER_TEST, time, TimerProc);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	//Timeout has occurred
	timeout = true;
	KillTimer(hwnd, TIMER_TEST);
	//MessageBox(hwnd, "Timed out.", "", NULL);
	//numTimeouts++;
	updateInfo(&numTimeouts);

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/
VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)
{
	printf(TEXT("Error code:\t%x\n"), dwErrorCode);
	printf(TEXT("Number of bytes: \t%x\n"), dwNumberOfBytesTransferred);
	g_BytesTransferred = dwNumberOfBytesTransferred;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
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
				//MessageBox(hwnd, "idle", "", MB_OK);

				if (inputBuffer[0] == SYN && inputBuffer[1] == ENQ)
				{
					Acknowledge();
					linkedReset = false;
					break;
					//memset(inputBuffer, 0, 518);
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
			//memset(inputBuffer, 0, 518);
		}
		else if (strlen(inputFileBuffer) > 0)
		{
			/*
			uint32_t test = CRC::Calculate(inputFileBuffer, st, CRC::CRC_32());
			bool passed = Validation(test, inputFileBuffer);
			unsigned char bytes[4];
			unsigned long n = test;

			bytes[0] = (n >> 24) & 0xFF;
			bytes[1] = (n >> 16) & 0xFF;
			bytes[2] = (n >> 8) & 0xFF;
			bytes[3] = n & 0xFF;
			*/
			sendEnq();
			bidForLine();
			fprintf(stderr, "asdf");
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/
VOID Acknowledge()
{
	DWORD bytesWritten;
	control[0] = 22;
	control[1] = 6;
	bool write = writeToPort(control, 2);
	//WriteFile(port, control, sizeof(control), &bytesWritten, NULL);
	Receive();
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/

VOID sendEnq()
{
	DWORD dwBytesWritten;
	char enq[2];
	enq[0] = 22;
	enq[1] = 5;
	bool write = writeToPort(enq, 2);
	//bool bwrite = WriteFile(port, enq, 2, &dwBytesWritten, NULL);
	if (!write) {
		MessageBox(hwnd, "i suck  at writefile", "", MB_OK);
	}
	//write success, Increment numENQSent counter
	else {
		//numENQSent++;
		updateInfo(&numENQSent);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
----------------------------------------------------------------------------------------------------------------------*/

VOID bidForLine()
{
	//MessageBox(hwnd, "Calling bidForLine()", "", NULL);
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
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
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
			//MessageBox(hwnd, "I have received an event, m'lord!", "", NULL);
			ClearCommError(port, &dwError, &cs);

			osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (osReader.hEvent == NULL)
			{
				MessageBox(hwnd, "I COULD NOT CREATE HEVENT MASTER", "", NULL);
				return 0;
			}

			if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
			{
				if (!ReadFile(port, inputBuffer, sizeof(inputBuffer), &nBytesRead, &osReader)) //need receive buffer to be extern to access
				{
					//error case, handle error here
					int i = 0;
				}
				else
				{
					//handle success
					int u = 0;
				}
				updateInfo(&numPacketsReceived);
			}
		}
		PurgeComm(port, PURGE_RXCLEAR);
	}
	return nBytesRead;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: March 16, 2008
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int initialize_inotify_watch (int fd, char pathname[MAXPATHLEN])
-- int fd: the descriptor returned by inotify_init()
-- char pathname[MAXPATHLEN]: fully qualified pathname of
-- directory to be watched.
--
-- RETURNS: Returns the watch descriptor (wd), which is bound to fd and the
-- directory pathname.
--
-- NOTES:
-- This function is used to generate a watch descriptor using a initialized descriptor from
-- inotify_init and a specified pathname. This watch descriptor can then be used by the select call
-- to monitor for events, i.e., file activity inside the watched directory.
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


