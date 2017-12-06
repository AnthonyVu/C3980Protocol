#include "Header.h"
#include "Print.h"
#include "Main.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <cstring>


/*
print() header here
*/
void print() {
	HDC dc = GetDC(hwnd);
	   
	TEXTMETRIC tm;
	RECT rect;
	int currentWindowWidth;
	int charHeight;
	SIZE length;
	

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
				TextOut(dc, printColumn, printRow, buff, sizeof(buff));
				printColumn += length.cx;
				nullCount--;
			}
			if (printColumn > yDataEnd - 25) {
				printColumn = 0;
				printRow += charHeight;
			}
			sprintf_s(buff, "%c", inputBuffer[i]);
			GetTextExtentPoint32(dc, buff, 1, &length);
			TextOut(dc, printColumn, printRow, buff, sizeof(buff));
			printColumn += length.cx;
		}
	}
	ReleaseDC(hwnd, dc);
}

/*
printStaticInfo() header here
*/
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

/*
updateInfo() header here
*/
void updateInfo(size_t counter)
{
	HDC hdc = GetDC(hwnd);
	RECT rect;
	rect.top = yPckt;
	rect.left = xPckt;
	rect.right = windowWidth - xOffset;
	rect.bottom = windowHeight - yOffset;
	HRGN region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);

	counter++;

	char text[2];
	
	RedrawWindow(hwnd, &rect, region, RDW_INTERNALPAINT);

	//Packet data: sent/rec
	_itoa_s(numPacketsSent, text, 2, 10);
	TextOut(hdc, xPcktData, yPckt, text, strlen(text));
	_itoa_s(numPacketsReceived, text, 2, 10);
	TextOut(hdc, xPcktData, yPckt + yOffset, text, strlen(text));

	//Bit error data
	_itoa_s(numBitErrors, text, 2, 10);
	TextOut(hdc, xBitErrorData, yBitError, text, strlen(text)) ;

	//ACK data: sent/rec
	_itoa_s(numACKSent, text, 2, 10);
	TextOut(hdc, xACKData, yACK, text, strlen(text));
	_itoa_s(numACKReceived, text, 2, 10);
	TextOut(hdc, xACKData, yACK + yOffset, text, strlen(text));

	//ENQ data: sent/rec
	_itoa_s(numENQSent, text, 2, 10);
	TextOut(hdc, xENQData, yENQ, text, strlen(text));
	_itoa_s(numENQReceived, text, 2, 10);
	TextOut(hdc, xENQData, yENQ + yOffset, text, strlen(text));

	//Timeout data
	_itoa_s(numTimeouts, text, 2, 10);
	TextOut(hdc, xTOData, yTO, text, strlen(text));
}
