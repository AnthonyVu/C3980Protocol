#include "Header.h"
#include "Main.h"
#include "Receive.h"
#include "Transmit.h"
#include "Print.h"



extern bool sentCtrl = false;
extern int sent = 0;
extern bool eot = false;
extern char control[2] = { 0 };
extern char line[518] = { 0 };
extern char * filePtr = NULL;

void prepareToSend(char *outputBuffer, HANDLE port)
{
	sentCtrl = false;
	while (sentCtrl == false)
	{
		if (sent == 10)
		{
			eot = true;
		}
		if (eot == true)
		{
			control[0] = 22;
			control[1] = 4;
			sentCtrl = true;
		}
		else
		{
			if (filePtr == NULL)
			{
				filePtr = outputBuffer;
			}
			line[0] = 22;
			line[1] = 2;
			addData();
			addCRC();
		}
		send(port);
	}
}

void addData()
{
	size_t n = 0;
	int c;
	int count = 2;
	bool eof = false;
	while (count != 514)
	{
		if (eof == false && *filePtr != EOF)
		{
			line[count] = *filePtr;
			filePtr++;
		}
		else
		{
			eof = true;
			eot = true;
			line[count] = '\0';
		}
		count++;
	}
}

void send(HANDLE port)
{
	DWORD dwBytesWritten;
	int retransmitCount = 0;
	while (retransmitCount < 3)
	{
		//KillTimer(hwnd, TIMER_TEST);
		startTimer();
		timeout = false;
		if (eot == true)
		{
			sent = 0;
			//bool bwrite = WriteFile(port, (LPCVOID)control, (DWORD)strlen(control), &dwBytesWritten, NULL);
			bool bwrite = writeToPort(control, 2);
			return;
			//send control frame
		}
		else
		{
			//send line frame
			//bool bwrite = WriteFile(port, (LPCVOID)line, (DWORD)strlen(line), &dwBytesWritten, NULL);
			bool bwrite = writeToPort(line, strlen(line));
			
			while (timeout == false)
			{
				if (inputBuffer[0] == 22)
				{
					if (inputBuffer[1] == 6)
					{
						sent++;
						printT();
						return;
					}
					if (inputBuffer[1] == 21)
					{
						return;
					}
				}
			}
			retransmitCount++;
		}
	}
	eot = true;
	return;
}

void addCRC()
{

}


void printT() {
	HDC dc = GetDC(hwnd);

	TEXTMETRIC tm;
	RECT rect;
	//int windowWidth;
	int charHeight;


	GetTextMetrics(dc, &tm);
	charHeight = tm.tmExternalLeading + tm.tmHeight;

	GetWindowRect(hwnd, &rect);
	//windowWidth = rect.right - rect.left;



	int nullCount = 0;
	char buff[2];
	//strncpy_s(temp, inputBuffer, sizeof(temp));


	for (int i = 2; i < 514; i++) {
		if (line[i] == NULL) {
			nullCount++;
		}
		else
		{
			sprintf_s(buff, "%c", '\0');
			while (nullCount > 0) {
				if (printColumn > windowWidth) {
					printColumn = 0;
					printRow += 16;
				}
				TextOut(dc, printColumn, printRow, buff, sizeof(buff));
				printColumn += 10;
				nullCount--;
			}
			if (printColumn > windowWidth - 25) {
				printColumn = 0;
				printRow += 16;
			}
			sprintf_s(buff, "%c", line[i]);
			TextOut(dc, printColumn, printRow, buff, sizeof(buff));
			printColumn += 10;
		}
	}
	ReleaseDC(hwnd, dc);
}
