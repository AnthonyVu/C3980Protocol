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
FILE * filePtr = NULL;

void prepareToSend(FILE *outputBuffer)
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
		send();
	}
}

void addData()
{
	size_t n = 0;
	int c;
	int count = 0;
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

void send()
{
	int retransmitCount = 0;
	while (retransmitCount < 3)
	{
		//startTimer();
		if (eot == true)
		{
			sent = 0;
			//send control frame
		}
		else
		{
			//send line frame
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