#include "Header.h"
#include "Main.h"
#include "Receive.h"
#include "Transmit.h"
#include "Print.h"
#include "crc.h"
#include <stdint.h>

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
			uint32_t crc = CRC::Calculate(line, strlen(line), CRC::CRC_32());
			unsigned char bytes[4];
			unsigned long n = crc;

			bytes[0] = (n >> 24) & 0xFF;
			bytes[1] = (n >> 16) & 0xFF;
			bytes[2] = (n >> 8) & 0xFF;
			bytes[3] = n & 0xFF;
			addCRC(line, bytes);
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
		startTimer(5000);
		timeout = false;
		if (eot == true)
		{
			sent = 0;
			bool bwrite = writeToPort(control, (DWORD)strlen(control));
			return;
			//send control frame
		}
		else
		{
			//send line frame
			bool bwrite = writeToPort(line, strlen(line));

			while (timeout == false)
			{
				if (inputBuffer[0] == 22)
				{
					if (inputBuffer[1] == 6)
					{
						sent++;
						//printT();
						memset(inputBuffer, 0, 518);
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

VOID addCRC(char* data, unsigned char* crc)
{
	data[sizeof(data) - 3] = crc[0];
	data[sizeof(data) - 2] = crc[1];
	data[sizeof(data) - 1] = crc[2];
	data[sizeof(data)] = crc[3];
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
