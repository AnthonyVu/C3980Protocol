#include "Header.h"
#include "Print.h"
#include <string.h>
#include <stdio.h>



void print() {
	HDC dc = GetDC(hwnd);
	   
	TEXTMETRIC tm;
	RECT rect;
	int windowWidth;
	int charHeight;
	int row = 0;
	int column = 0;

	GetTextMetrics(dc, &tm);
	charHeight = tm.tmExternalLeading + tm.tmHeight;

	GetWindowRect(hwnd, &rect);
	windowWidth = rect.right - rect.left;



	int nullCount = 0;
	char temp[515];
	char buff[2];
	strncpy_s(temp, inputBuffer, sizeof(temp) - 1);




	for (int i = 2; i < 514; i++) {
		if (temp[i] == NULL) {
			nullCount++;
		}
		else 
		{
			sprintf_s(buff, "%c", '\0');
			while (nullCount > 0) {
				if (column > windowWidth) {
					column = 0;
					row += 16;
				}
				TextOut(dc, column, row, buff, sizeof(buff));
				column += 10;
				nullCount--;
			}
			if (column > windowWidth - 25) {
				column = 0;
				row += 16;
			}
			sprintf_s(buff, "%c", temp[i]);
			TextOut(dc, column, row, buff, sizeof(buff));
			column += 10;
		}
	}
	ReleaseDC(hwnd, dc);
}
