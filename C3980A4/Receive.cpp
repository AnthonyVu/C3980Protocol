#include "Receive.h"
#include "Header.h"
#include "Print.h"
#include "Main.h"

void Receive() {
	DWORD bitsWritten;
	char receiveBuffer[518];
	//char * currIndex = receiveBuffer;
	//memset(recieveBuffer, 0, 518);
	startTimer(5000);
	//start timer thread
	memset(inputBuffer, 0, 518);
	//int messagesRecieved = 0;
	while (!timeout) {
		if (inputBuffer[0] == 22) {

			//eot
			if (inputBuffer[1] == 4) {
				memset(inputBuffer, 0, 518);
				KillTimer(hwnd, TIMER_TEST);
				return;
			}
			else if (inputBuffer[1] == 7) { //RVI
				char rvi[2];
				rvi[0] = 22;
				//rvi[1] = (char) ? ;
				writeToPort(rvi, sizeof(rvi));
				return;

			}
			else if (inputBuffer[1] == 2) {
				//call validation on inpuBuffer
				//if (validate(inputBuffer)) {
				char ack[2];
				ack[0] = 22;
				ack[1] = 6;
				//memmove(inputBuffer, currIndex, strlen(inputBuffer));
				//currIndex = currIndex + strlen(inputBuffer);
				//if (strlen(receiveBuffer) == 518) {
				print();
				KillTimer(hwnd, TIMER_TEST);
				//WriteFile(port, ack, sizeof(ack), &bitsWritten, NULL);
				bool bwrite = writeToPort(ack, 2);
				startTimer(5000);
				memset(inputBuffer, 0, 518);
				memset(receiveBuffer, 0, 518);
				//	messagesRecieved++;
					//currIndex = receiveBuffer;
				//}
			//}
			}
		}
	}
}