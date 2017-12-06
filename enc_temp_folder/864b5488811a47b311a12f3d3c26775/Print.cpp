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
void updateInfo()
{
	HDC hdc = GetDC(hwnd);
	RECT rect;
	rect.top = yPckt;
	rect.left = xPckt;
	rect.right = windowWidth - xOffset;
	rect.bottom = windowHeight - yOffset;
	HRGN region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);

	RedrawWindow(hwnd, &rect, region, RDW_INTERNALPAINT);
	//Packet data: sent/rec
	TextOut(hdc, xPcktData, yPckt, std::to_string(numPacketsSent).std::string::c_str(), strlen(std::to_string(numPacketsSent).std::string::c_str()));
	TextOut(hdc, xPcktData, yPckt + yOffset, std::to_string(numPacketsReceived).std::string::c_str(), strlen(std::to_string(numPacketsReceived).std::string::c_str()));
	//Bit error data
	TextOut(hdc, xBitErrorData, yBitError, std::to_string(numBitErrors).std::string::c_str(), strlen(std::to_string(numBitErrors).std::string::c_str()));
	//ACK data: sent/rec
	TextOut(hdc, xACKData, yACK, std::to_string(numACKSent).std::string::c_str(), strlen(std::to_string(numACKSent).std::string::c_str()));
	TextOut(hdc, xACKData, yACK + yOffset, std::to_string(numACKReceived).std::string::c_str(), strlen(std::to_string(numACKReceived).std::string::c_str()));
	//ENQ data: sent/rec
	TextOut(hdc, xENQData, yENQ, std::to_string(numENQSent).std::string::c_str(), strlen(std::to_string(numENQSent).std::string::c_str()));
	TextOut(hdc, xENQData, yENQ + yOffset, std::to_string(numENQReceived).std::string::c_str(), strlen(std::to_string(numENQReceived).std::string::c_str()));
	//Timeout data
	TextOut(hdc, xTOData, yTO, std::to_string(numTimeouts).std::string::c_str(), strlen(std::to_string(numTimeouts).std::string::c_str()));
}
