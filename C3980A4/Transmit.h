#pragma once
#include <stdio.h>
#include <Windows.h>

void prepareToSend(char *outputBuffer, HANDLE hComm);
void addData();
void send(HANDLE hComm);
void addCRC();

extern bool sentCtrl;
extern int sent;
extern bool eot;
extern char control[2];
extern char line[518];
