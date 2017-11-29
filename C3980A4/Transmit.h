#pragma once
void prepareToSend();
void addData();
void send();
void addCRC();

bool sentCtrl = false;
int sent = 0;
bool eot = false;
char control[2];
FILE *filePtr;
char line[518];
FILE * outputBuffer;
char * inputBuffer;
