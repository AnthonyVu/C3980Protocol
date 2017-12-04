#include "Receive.h"
#include "Header.h"
#include "Print.h"
#include "Main.h"

void Receive() {
	DWORD bitsWritten;
	//char recieveBuffer[518];
	//memset(recieveBuffer, 0, 518);
	startTimer();
	//start timer thread
	memset(inputBuffer, 0, 518);
	int messagesRecieved = 0;
	while (!timeout && messagesRecieved != 10) {
		if (inputBuffer[0] == 22) {
			 
			//eot
			if (inputBuffer[1] == 4) {
				return;
			} else if (inputBuffer[1] == 'R') { //?undetermined
				char rvi[2];
				rvi[0] = 22;
				//rvi[1] = (char) ? ;
				WriteFile(port, rvi, sizeof(rvi), &bitsWritten, NULL);
				return;

			}
			else if (inputBuffer[1] == 2) {
				//call validation on inpuBuffer
				//if (validate(inputBuffer)) {
					char ack[2];
					ack[0] = 22;
					ack[1] = 6;
					WriteFile(port, ack, sizeof(ack), &bitsWritten, NULL);
					KillTimer(hwnd, TIMER_TEST);
					startTimer();
					print();
					memset(inputBuffer, 0, 518);
					messagesRecieved++;
				//}
				
			
			}	
		}
	}
}