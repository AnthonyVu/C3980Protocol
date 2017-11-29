#pragma once
#include <stdio.h>
#include <Windows.h>

void prepareToSend(FILE *outputBuffer);
void addData();
void send();
void addCRC();

extern bool sentCtrl;
extern int sent;
extern bool eot;
extern char control[2];
extern char line[518];
