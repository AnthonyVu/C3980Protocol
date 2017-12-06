#pragma once
#define STRICT
#include <Windows.h>
#include <stdint.h>

#define IDT_TIMER1				101
#define IDT_TIMER2				102
#define TIMER_TEST				999
#define TRANSMIT_TIMEOUT		2000
#define RECEIVE_TIMEOUT			2000
#define TEST_TIMEOUT			2000

#define MENU_CONNECT			103
#define MENU_DISCONNECT			104
#define MENU_QUIT				105
#define MENU_FILE				106

#define windowHeight			900
#define windowWidth				1200

#define MAX_FILE_SIZE			32768
#define MAX_FILENAME_SIZE		128

#define ENQ						5
#define ACK						6
#define SYN						22
#define STX						2
#define EOT						4
#define BEL						7

extern bool timeout;
extern bool linkedReset;
extern char inputBuffer[];
extern bool rvi;
extern char inputFileBuffer[MAX_FILE_SIZE];

extern size_t numPacketsSent;
extern size_t numPacketsReceived;
extern size_t numBitErrors;
extern size_t numACKSent;
extern size_t numACKReceived;
extern size_t numENQSent;
extern size_t numENQReceived;
extern size_t numTimeouts;


VOID startTimer(unsigned int time);
BOOL writeToPort(char* writeBuffer, DWORD dwNumToWrite);

