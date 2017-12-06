/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Transmit.cpp - Handles the sending responsibilities of
--							the protocol
--
-- PROGRAM: c3980A4
--
-- FUNCTIONS:
-- void prepareToSend(char *outputBuffer, HANDLE port)
-- void addData()
-- void send(HANDLE port)
-- VOID addCRC(char* data, unsigned char* crc)
--
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker, Anthony Vu
--
-- NOTES:
-- Transmit handles the send portion of the protocol. It will send 10 frames, each of 518 bytes and wait for
-- an acknowledgment. It uses CRC 32 for error detection.
----------------------------------------------------------------------------------------------------------------------*/#include "Header.h"
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
bool rviSent = false;
bool eof = false;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: prepareToSend
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker
--
-- INTERFACE: void prepareToSend(char *outputBuffer, HANDLE port)
-- char *outputBuffer: the buffer to the file uploaded by the users
-- HANDLE port: the handle to the comm port
--
-- RETURNS: void
--
-- NOTES:
-- This function checks if this is the first time transmitting and sets the pointer. It makes the
-- decision to send the control frame or the data frame based on how many frames have been sent, if
-- the end of the file has been reached, or if the recieving user sent an RVI.
----------------------------------------------------------------------------------------------------------------------*/
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
			if (eof)
			{
				filePtr = NULL;
				memset(inputFileBuffer, 0, MAX_FILE_SIZE);
			}
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
		}
		send(port);
		if (rviSent)
		{
			rviSent = false;
			return;
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addData
--
-- DATE: December 3, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker, Anthony Vu
--
-- INTERFACE: void addData()
--
-- RETURNS: void
--
-- NOTES:
-- This function is used to add data to a data frame. It will not be called if the program is sending a
-- control frame. The function checks if the end of the file has been reached and will pad with nulls and
-- set the end of file variable to true. After data has been added, the CRC will be calculated based on
-- the data.
----------------------------------------------------------------------------------------------------------------------*/
void addData()
{
	char temp[512];
	size_t n = 0;
	int count = 2;
	eof = false;
	unsigned char bytes[4] = {0};

	while (count != 514)
	{
		if (eof == false && *filePtr != 0)
		{
			line[count] = *filePtr;
			filePtr++;
		}
		else
		{
			eof = true;
			line[count] = 0;
		}
		count++;
	}

	memcpy(temp, &line[2], sizeof(char) * 512);
	uint32_t crc = CRC::Calculate(temp, 512, CRC::CRC_32());
	unsigned long shift = crc;
	bytes[0] = (shift >> 24) & 0xFF;
	bytes[1] = (shift >> 16) & 0xFF;
	bytes[2] = (shift >> 8) & 0xFF;
	bytes[3] = shift & 0xFF;
  	addCRC(line, bytes);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send
--
-- DATE: December 3, 2017 
--
-- REVISIONS:
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Haley Booker
--
-- INTERFACE: void send(HANDLE port)
-- HANDLE port: the handle to the comm port
--
-- RETURNS: void
--
-- NOTES:
-- This function handles the recieved frame after data has been written to the comm port.
-- It uses variables set in prepare to send to determine if it sending a control frame or
-- a data frame. It will either timeout or wait for a response from the recieving side. It
-- can recieve an ACK or an RVI(BEL). If it timesout the retransmit count will increase.
-- After 3 retransmissions the program will return to idle.
----------------------------------------------------------------------------------------------------------------------*/
void send(HANDLE port)
{
	DWORD dwBytesWritten;
	int retransmitCount = 0;
	while (retransmitCount < 3)
	{
		//KillTimer(hwnd, TIMER_TEST);
		startTimer(2000);
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
			bool bwrite = writeToPort(line, 518);

			while (timeout == false)
			{
				if (inputBuffer[0] == 22)
				{
					if (inputBuffer[1] == ACK)//6
					{
						sent++;
						memset(inputBuffer, 0, 518);
						if (eof)
						{
							eot = true;
						}
						return;
					}
					if (inputBuffer[1] == BEL)//7
					{
						rviSent = true;
						filePtr = filePtr - 512;
						sent = 0;
						return;
					}
				}
			}
			retransmitCount++;
		}
	}
	filePtr = filePtr - 512;
	eot = true;
	return;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addCRC
--
-- DATE: December 3, 2017
--
-- REVISIONS:
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Anthony Vu
--
-- INTERFACE: VOID addCRC(char* data, unsigned char* crc)
-- char* data: the data frame with the header and data
-- unsigned char* crc: the 4 byte CRC that was generated
--
-- RETURNS: void
--
-- NOTES:
-- This function takes a data frame and CRC and appends the CRC to the data frame.
----------------------------------------------------------------------------------------------------------------------*/
VOID addCRC(char* data, unsigned char* crc)
{
	int count = 514;
	data[count] = crc[0];
	data[count + 1] = crc[1];
	data[count + 2] = crc[2];
	data[count + 3] = crc[3];
}
