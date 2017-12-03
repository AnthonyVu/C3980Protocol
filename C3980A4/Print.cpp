#include "Header.h"
#include "Print.h"
#include "Main.h"
#include <string.h>
#include <stdio.h>



void print() {
	HDC dc = GetDC(hwnd);
	   
	TEXTMETRIC tm;
	RECT rect;
	//int windowWidth;
	int charHeight;
	

	GetTextMetrics(dc, &tm);
	charHeight = tm.tmExternalLeading + tm.tmHeight;

	GetWindowRect(hwnd, &rect);
	//windowWidth = rect.right - rect.left;



	int nullCount = 0;
	char temp[514];
	char buff[2];
	//strncpy_s(temp, inputBuffer, sizeof(temp));


	for (int i = 2; i < 514; i++) {
		if (inputBuffer[i] == NULL) {
			nullCount++;
		}
		else 
		{
			sprintf_s(buff, "%c", '\0');
			while (nullCount > 0) {
				if (printColumn > windowWidth) {
					printColumn = 0;
					printRow += 16;
				}
				TextOut(dc, printColumn, printRow, buff, sizeof(buff));
				printColumn += 10;
				nullCount--;
			}
			if (printColumn > windowWidth - 25) {
				printColumn = 0;
				printRow += 16;
			}
			sprintf_s(buff, "%c", temp[i]);
			TextOut(dc, printColumn, printRow, buff, sizeof(buff));
			printColumn += 10;
		}
	}
	ReleaseDC(hwnd, dc);
}
