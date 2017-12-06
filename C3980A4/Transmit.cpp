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
-- DATE: December 5, 2017
--
-- REVISIONS: 
--
-- DESIGNER: Haley Booker, Wilson Hu, Anthony Vu, Matthew Shew
--
-- PROGRAMMER: Haley Booker, Anthony Vu
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
VOID addCRC(char* data, unsigned char* crc)
{
	int count = 514;
	data[count] = crc[0];
	data[count + 1] = crc[1];
	data[count + 2] = crc[2];
	data[count + 3] = crc[3];
}
