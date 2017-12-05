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



	/*
	
	char remember[150];
	int characters = 0;
	int charHeight, rows = 0;
	DWORD btransferred;
	TEXTMETRIC tm;
	SIZE length;
	RECT rect;
	int width;

	GetTextMetrics(dc, &tm);
	charHeight = tm.tmExternalLeading + tm.tmHeight;

	GetWindowRect((HWND)n, &rect);
	width = rect.right - rect.left;

	while (connectMode) {
		GetTextExtentPoint32(dc, remember, characters, &length);
		if (length.cx >= width - 25) {
			rows++;
			memset(remember, ' ', characters);
			characters = 0;

			GetTextExtentPoint32(dc, remember, characters, &length);
		}
		if (ReadFile(port, buffer, sizeof(buffer), &btransferred, NULL)) {
			if (btransferred) {
				remember[characters] = buffer[0];
				characters++;
				sprintf_s(toScreen, "%c", buffer[0]);
				TextOut(dc, length.cx, charHeight * rows, toScreen, sizeof(toScreen));
			}
		}
	}*/


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
				if (printColumn > windowWidth) {
					printColumn = 0;
					printRow += charHeight;
				}
				TextOut(dc, printColumn, printRow, buff, sizeof(buff));
				printColumn += length.cx;
				nullCount--;
			}
			if (printColumn > windowWidth - 25) {
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
