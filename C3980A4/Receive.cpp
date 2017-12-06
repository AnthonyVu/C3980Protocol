#include "Receive.h"
#include "Header.h"
#include "Print.h"
#include "Main.h"

void Receive() {
	DWORD bitsWritten;
	startTimer(5000);
	memset(inputBuffer, 0, 518);
	while (!timeout) {
		if (inputBuffer[0] == 22) {

			//EOT
			if (inputBuffer[1] == 4) {
				memset(inputBuffer, 0, 518);
				KillTimer(hwnd, TIMER_TEST);
				return;
				//RVI
			}
			else if (rvi) {
				char rviFrame[2];
				rviFrame[0] = 22;
				rviFrame[1] = 7;
				writeToPort(rviFrame, sizeof(rviFrame));
				memset(inputBuffer, 0, 518);
				KillTimer(hwnd, TIMER_TEST);
				rvi = false;
				return;
				//STX
			}
			else if (inputBuffer[1] == 2) {
				//call validation on inpuBuffer
				//if (validate(inputBuffer)) {

				print();
				KillTimer(hwnd, TIMER_TEST);
				char ack[2];
				ack[0] = 22;
				ack[1] = 6;
				writeToPort(ack, 2);
				startTimer(5000);
				memset(inputBuffer, 0, 518);
				//}
			}
		}
	}
}