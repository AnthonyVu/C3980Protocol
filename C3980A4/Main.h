#pragma once
#define IDT_TIMER1			101
#define IDT_TIMER2			102
#define TIMER_TEST			999
#define TRANSMIT_TIMEOUT	2000
#define RECEIVE_TIMEOUT		2000
#define TEST_TIMEOUT		1000

#define MENU_CONNECT			103
#define MENU_DISCONNECT			104
#define MENU_QUIT				105
#define MENU_FILE				106

#define windowHeight			600
#define windowWidth				400

extern bool timeout;
extern bool linkedReset;
extern char * inputBuffer;
