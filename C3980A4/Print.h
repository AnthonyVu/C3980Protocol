#pragma once
//Global coordinates & data coordinates
#define xOffset				10
#define yOffset				20
#define xDataStart			1
#define yDataStart			1
#define xDataEnd			900
#define yDataEnd			880

//Coordinates to display current state

//Coordinates to display Num. Packets Sent
#define	xPckt				950
#define	yPckt				50
#define xPcktData			1150
//Coordinates to display Num. Bit Errors
#define xBitError			950
#define yBitError			100
#define xBitErrorData		1150
//Coordinates to display Num. ACKs & Timeouts
#define xACK				950
#define yACK				150
#define xACKData			1150
//Coordinates to display Num. ENQs
#define xENQ				950
#define yENQ				190
#define xENQData			1150
//Coordinates to display Num. Timeouts
#define xTO					950
#define yTO					230
#define xTOData				1150



void print();
void printStaticInfo();
void updateInfo(size_t);

