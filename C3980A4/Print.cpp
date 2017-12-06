/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Print.cpp - Handles the printing of data frames and statistics of the amount of successfull and unsuccessful
-- frames received and sent. 
--
-- PROGRAM: C3980A4
--
-- FUNCTIONS:
-- void print()
-- void printStaticInfo()
-- void updateInfo(size_t* counter)
--
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew, Wilson Hu
--
-- NOTES:
-- This file handles the printing of data frames and statistics of the amount of successfull and unsuccessful
-- frames received and sent. 
----------------------------------------------------------------------------------------------------------------------*/
#include "Header.h"
#include "Print.h"
#include "Main.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <cstring>
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: print
--
-- DATE: December 3, 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew
--
-- INTERFACE: void print()
--
-- RETURNS: void
--
-- NOTES:
-- This function is used to print the current data frame contents to the screen for the user. 
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
				if (printRow >= textRect.bottom - 25) {
					InvalidateRect(hwnd, &textRect, TRUE);
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
			}
			if (printRow >= rect.bottom - 25) {
				
				printRow = 0;
				printColumn = 0;
				InvalidateRect(hwnd, &textRect, TRUE);
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
-- FUNCTION: printStaticInfo
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu
--
-- INTERFACE: void printStaticInfo()
--
-- RETURNS: void
--
-- NOTES:
-- This function is used to print the statistics of successful and unsuccessful transmitted and received frames to and from another user. 
----------------------------------------------------------------------------------------------------------------------*/
void printStaticInfo()
{
	HDC hdc = GetDC(hwnd);

	char connectionState[] = "Disconnected";
	char numPacketSentText[] = "Packets transferred:";
	char numPacketReceivedText[] = "Packets received:";
	char numErrorText[] = "Num. Bit Errors:";
	char numACKSentText[] = "Num. ACKs sent:";
	char numACKReceivedText[] = "Num. ACKs received:";
	char numENQSentText[] = "Num. ENQs sent:";
	char numENQReceivedText[] = "Num. ENQs received:";
	char numTOText[] = "Num. Timeouts occurred:";

	//Prints static portion of info frame
	TextOut(hdc, xState, yState, connectionState, strlen(connectionState));

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
-- FUNCTION: updateInfo
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu
--
-- INTERFACE: void updateInfo(size_t* counter)
--						size_t* counter - 
--
-- RETURNS: void
--
-- NOTES:
-- This function is used to update the statistics on the screen for the user. 
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

	char text[5];
	
	RedrawWindow(hwnd, &rect, region, RDW_INTERNALPAINT);

	//Packet data: sent/rec
	_itoa_s(numPacketsSent, text, 5, 10);
	TextOut(hdc, xPcktData, yPckt, text, strlen(text));
	_itoa_s(numPacketsReceived, text, 5, 10);
	TextOut(hdc, xPcktData, yPckt + yOffset, text, strlen(text));

	//Bit error data
	_itoa_s(numBitErrors, text, 5, 10);
	TextOut(hdc, xBitErrorData, yBitError, text, strlen(text)) ;

	//ACK data: sent/rec
	_itoa_s(numACKSent, text, 5, 10);
	TextOut(hdc, xACKData, yACK, text, strlen(text));
	_itoa_s(numACKReceived, text, 5, 10);
	TextOut(hdc, xACKData, yACK + yOffset, text, strlen(text));

	//ENQ data: sent/rec
	_itoa_s(numENQSent, text, 5, 10);
	TextOut(hdc, xENQData, yENQ, text, strlen(text));
	_itoa_s(numENQReceived, text, 5, 10);
	TextOut(hdc, xENQData, yENQ + yOffset, text, strlen(text));

	//Timeout data
	_itoa_s(numTimeouts, text, 5, 10);
	TextOut(hdc, xTOData, yTO, text, strlen(text));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateConnectionStatus
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Wilson Hu
--
-- INTERFACE: void updateConnection()
--
-- RETURNS: void
--
-- NOTES:
-- This function is used to update the connection status on the screen for the user.
----------------------------------------------------------------------------------------------------------------------*/
void updateConnectionStatus()
{
	if (connectMode)
	{
		HDC hdc = GetDC(hwnd);
		RECT rect;
		rect.top = yState - 5;
		rect.left = xState - 5;
		rect.right = xState + 30;
		rect.bottom = yState + yOffset;
		HRGN region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
		char connectionStatus[] = "---Connected---";

		bool a = RedrawWindow(hwnd, &rect, region, RDW_INTERNALPAINT);

		TextOut(hdc, xState, yState, connectionStatus, strlen(connectionStatus));
	}
}
