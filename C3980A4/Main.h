#pragma once
#define STRICT
#include <Windows.h>

#define IDT_TIMER1				101
#define IDT_TIMER2				102
#define TIMER_TEST				999
#define TRANSMIT_TIMEOUT		2000
#define RECEIVE_TIMEOUT			2000
#define TEST_TIMEOUT			5000

#define MENU_CONNECT			103
#define MENU_DISCONNECT			104
#define MENU_QUIT				105
#define MENU_FILE				106

#define windowHeight			900
#define windowWidth				1200

extern bool timeout;
extern bool linkedReset;
extern char inputBuffer[];

VOID startTimer(unsigned int time);
BOOL writeToPort(char* writeBuffer, DWORD dwNumToWrite);
