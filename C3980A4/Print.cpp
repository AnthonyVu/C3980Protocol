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
----------------------------------------------------------------------------------------------------------------------*/#include "Header.h"
#include "Print.h"
#include "Main.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <cstring>


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
void print() {
	HDC dc = GetDC(hwnd);
	   
	TEXTMETRIC tm;
	RECT rect;
	int currentWindowWidth;
	int charHeight;
	SIZE length;
	RECT textRect;
	textRect.top = yDataStart;
	textRect.left = xDataStart;
	textRect.right = xDataEnd;
	textRect.bottom = yDataEnd;
	

	GetTextMetrics(dc, &tm);
	charHeight = tm.tmExternalLeading + tm.tmHeight;

	GetWindowRect(hwnd, &rect);
	currentWindowWidth = rect.right - rect.left;
	 

	int nullCount = 0;
	char buff[2];
	

	for (int i = 2; i < 514; i++) {
		if (inputBuffer[i] == NULL) {
			nullCount++;
		}
		else 
		{
			sprintf_s(buff, "%c", '\0');
			GetTextExtentPoint32(dc, buff, 1, &length);
			while (nullCount > 0) {
				if (printColumn > yDataEnd - 25) {
					printColumn = 0;
					printRow += charHeight;
				}
				if (printRow >= rect.bottom - 25) {
					InvalidateRect(hwnd, &rect, TRUE);
					printRow = 0;
					printColumn = 0;
				}
				TextOut(dc, printColumn, printRow, buff, sizeof(buff));
				printColumn += length.cx;
				nullCount--;
			}
			if (printColumn > yDataEnd - 25) {
				printColumn = 0;
				printRow += charHeight;
				InvalidateRect(hwnd, &textRect, TRUE);
			}
			if (printRow >= rect.bottom - 25) {
				InvalidateRect(hwnd, &rect, TRUE);
				printRow = 0;
				printColumn = 0;
			}
			sprintf_s(buff, "%c", inputBuffer[i]);
			GetTextExtentPoint32(dc, buff, 1, &length);
			TextOut(dc, printColumn, printRow, buff, sizeof(buff));
			printColumn += length.cx;
		}
	}
	ReleaseDC(hwnd, dc);
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
void printStaticInfo()
{
	HDC hdc = GetDC(hwnd);

	char numPacketSentText[] = "Packets transferred:";
	char numPacketReceivedText[] = "Packets received:";
	char numErrorText[] = "Num. Bit Errors:";
	char numACKSentText[] = "Num. ACKs sent:";
	char numACKReceivedText[] = "Num. ACKs received:";
	char numENQSentText[] = "Num. ENQs sent:";
	char numENQReceivedText[] = "Num. ENQs received:";
	char numTOText[] = "Num. Timeouts occurred:";

	//Prints static portion of info frame
	TextOut(hdc, xPckt, yPckt, numPacketSentText, strlen(numPacketSentText));
	TextOut(hdc, xPckt, yPckt + yOffset, numPacketReceivedText, strlen(numPacketReceivedText));

	TextOut(hdc, xBitError, yBitError, numErrorText, strlen(numErrorText));

	TextOut(hdc, xACK, yACK, numACKSentText, strlen(numACKSentText));
	TextOut(hdc, xACK, yACK + yOffset, numACKReceivedText, strlen(numACKReceivedText));

	TextOut(hdc, xENQ, yENQ, numENQSentText, strlen(numENQSentText));
	TextOut(hdc, xENQ, yENQ + yOffset, numENQReceivedText, strlen(numENQReceivedText));

	TextOut(hdc, xTO, yTO, numTOText, strlen(numTOText));
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
void updateInfo(size_t* counter)
{
	HDC hdc = GetDC(hwnd);
	RECT rect;
	rect.top = yPckt;
	rect.left = xPckt;
	rect.right = windowWidth - xOffset;
	rect.bottom = windowHeight - yOffset;
	HRGN region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);

	(*counter)++;

	char text[4];
	
	RedrawWindow(hwnd, &rect, region, RDW_INTERNALPAINT);

	//Packet data: sent/rec
	_itoa_s(numPacketsSent, text, 4, 10);
	TextOut(hdc, xPcktData, yPckt, text, strlen(text));
	_itoa_s(numPacketsReceived, text, 4, 10);
	TextOut(hdc, xPcktData, yPckt + yOffset, text, strlen(text));

	//Bit error data
	_itoa_s(numBitErrors, text, 4, 10);
	TextOut(hdc, xBitErrorData, yBitError, text, strlen(text)) ;

	//ACK data: sent/rec
	_itoa_s(numACKSent, text, 4, 10);
	TextOut(hdc, xACKData, yACK, text, strlen(text));
	_itoa_s(numACKReceived, text, 4, 10);
	TextOut(hdc, xACKData, yACK + yOffset, text, strlen(text));

	//ENQ data: sent/rec
	_itoa_s(numENQSent, text, 4, 10);
	TextOut(hdc, xENQData, yENQ, text, strlen(text));
	_itoa_s(numENQReceived, text, 4, 10);
	TextOut(hdc, xENQData, yENQ + yOffset, text, strlen(text));

	//Timeout data
	_itoa_s(numTimeouts, text, 4, 10);
	TextOut(hdc, xTOData, yTO, text, strlen(text));
}
