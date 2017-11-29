#pragma once
#define IDT_TIMER1			101
#define IDT_TIMER2			102
#define TIMER_TEST			999
#define TRANSMIT_TIMEOUT	2000
#define RECEIVE_TIMEOUT		2000
#define TEST_TIMEOUT		1000

bool timeout = false;
bool linkedReset = false;

VOID startTimer();
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void bidForLine();
