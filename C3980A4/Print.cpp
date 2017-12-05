#include "Header.h"
#include "Print.h"
#include "Main.h"
#include <string.h>
#include <stdio.h>



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
				if (printColumn > currentWindowWidth - 25) {
					printColumn = 0;
					printRow += charHeight;
				}
				TextOut(dc, printColumn, printRow, buff, sizeof(buff));
				printColumn += length.cx;
				nullCount--;
			}
			if (printColumn > currentWindowWidth - 25) {
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
