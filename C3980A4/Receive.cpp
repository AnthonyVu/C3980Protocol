/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Recieve.cpp - An application that will monitor a specified
-- directory for file creation/modification.
--
-- PROGRAM: protocol
--
-- FUNCTIONS:
-- void Receive()
-- bool Validation(char* input)
--
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew, Anthony Vu
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
#include "Receive.h"
#include "Header.h"
#include "Print.h"
#include "Main.h"
#include "crc.h"
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Receive
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew
--
-- INTERFACE: void Receive()
--
-- RETURNS: void
--
-- NOTES:
-- This functions checks the inputBuffer and if the inputBuffer is 
----------------------------------------------------------------------------------------------------------------------*/
void Receive() {
	DWORD bitsWritten;
	startTimer(2000);
	memset(inputBuffer, 0, 518);
	while (!timeout) {
		if (inputBuffer[0] == SYN)//22 
		{

			//EOT
			if (inputBuffer[1] == EOT)//4 
			{
				memset(inputBuffer, 0, 518);
				KillTimer(hwnd, TIMER_TEST);
				return;
				//RVI
			}
			else if (rvi) {
				char rviFrame[2];
				rviFrame[0] = 22;
				rviFrame[1] = 7;
				writeToPort(rviFrame, sizeof(rviFrame));
				memset(inputBuffer, 0, 518);
				KillTimer(hwnd, TIMER_TEST);
				rvi = false;
				return;
				//STX
			}
			else if (inputBuffer[1] == STX)//2 
			{
				//call validation on inpuBuffer
				if (Validation(inputBuffer)) {

					print();
					KillTimer(hwnd, TIMER_TEST);
					char ack[2];
					ack[0] = 22;
					ack[1] = 6;
					writeToPort(ack, 2);
					startTimer(2000);
					memset(inputBuffer, 0, 518);
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_inotify_watch
--
-- DATE: December 3, 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
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
bool Validation(char* input)
{
	char temp[512];
	unsigned char bytes[4];
	memcpy(temp, &input[2], sizeof(char) * 512);
	uint32_t crc = CRC::Calculate(temp, 512, CRC::CRC_32());
	unsigned long shift = crc;
	bytes[0] = (shift >> 24) & 0xFF;
	bytes[1] = (shift >> 16) & 0xFF;
	bytes[2] = (shift >> 8) & 0xFF;
	bytes[3] = shift & 0xFF;
	for (int i = 0, j = 514; i < 4; i++, j++)
	{
		char a = bytes[i];
		char b = input[j];
		if (a != b)
		{
			updateInfo(&numBitErrors);
			return false;
		}
	}
	return true;
}