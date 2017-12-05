#pragma once
#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
void prepareToSend(char *outputBuffer, HANDLE hComm);
void addData();
void send(HANDLE hComm);
void addCRC(char*, unsigned char*);
void printT();

extern bool sentCtrl;
extern int sent;
extern bool eot;
extern char control[2];
extern char line[518];
extern char * filePtr;
