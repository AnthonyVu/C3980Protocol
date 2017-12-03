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
char * filePtr = NULL;

void prepareToSend(char *outputBuffer, HANDLE hComm)
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
		send(hComm);
	}
}

void addData()
{
	size_t n = 0;
	int c;
	int count = 2;
	bool eof = false;
	while (count != 512)
	{
		if (eof == false && (c = fgetc(filePtr)) != EOF)
		{
			line[count] = (char)c;
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

void send(HANDLE hComm)
{
	DWORD dwBytesWritten;
	int retransmitCount = 0;
	while (retransmitCount < 3)
	{
		startTimer();
		if (eot == true)
		{
			sent = 0;
			bool bwrite = WriteFile(hComm, (LPCVOID)control, (DWORD)strlen(control), &dwBytesWritten, NULL);
			return;
			//send control frame
		}
		else
		{
			//send line frame
			bool bwrite = WriteFile(hComm, (LPCVOID)line, (DWORD)strlen(line), &dwBytesWritten, NULL);
			while (timeout == false)
			{
				if (inputBuffer != NULL)
				{
					if (inputBuffer[1] == 6)
					{
						sent++;
						return;
					}
					if (inputBuffer[1] == 0)
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