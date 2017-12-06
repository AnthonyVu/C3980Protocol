#pragma once
#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
void addData();
void addCRC(char*, unsigned char*);
void prepareToSend(char *outputBuffer, HANDLE port);
void addData();
void send(HANDLE port);

extern bool sentCtrl;
extern int sent;
extern bool eot;
extern char control[2];
extern char line[518];
extern char * filePtr;
