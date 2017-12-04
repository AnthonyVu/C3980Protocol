#pragma once
#include <stdio.h>
#include <Windows.h>

void prepareToSend(char *outputBuffer, HANDLE port);
void addData();
void send(HANDLE port);
void addCRC();
void printT();

extern bool sentCtrl;
extern int sent;
extern bool eot;
extern char control[2];
extern char line[518];
extern char * filePtr;
