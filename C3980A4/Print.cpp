#include "Header.h"
#include "Print.h"
#include <string.h>
#include <stdio.h>



void print() {

	HDC dc = GetDC(hwnd);
	int nullCount = 0;
	char temp[515];
	char buff[2];
	strncpy(temp, inputBuffer, sizeof(temp) - 1);
	for (int i = 2; i < 514; i++) {
		if (temp[i] == NULL) {
			nullCount++;
		}
		else 
		{
			sprintf_s(buff, "%c", '\0');
			while (nullCount > 0) {
				TextOut(dc, 0, 0, buff, sizeof(buff));
				nullCount--;
			}
			sprintf_s(buff, "%c", temp[i]);
			TextOut(dc, 0, 0, buff, sizeof(buff));
		}
	}
}
