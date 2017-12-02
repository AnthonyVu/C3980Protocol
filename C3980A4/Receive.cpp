#include "Receive.h"
#include "Header.h"
#include "Print.h"


void Receive() {
	DWORD bitsWritten;
	char buffer[518];
	timeout = false;
	//start timer thread
	while (!timeout) {

		if (ReadFile(port, buffer, sizeof(buffer), &bitsWritten, NULL)) {
			if (bitsWritten) {
				inputBuffer = buffer;
				print();
				/*
				if (buffer[1] == 4) {
					return;
				}
				*/

				/*
				else if (outputBuffer == RVI) { //?undetermined
					char rvi[2];
					rvi[0] = (char)22;
					//rvi[1] = (char) ? ;

					WriteFile(port, (LPCVOID)&rvi, sizeof(RVI), &bitsWritten, NULL);

					return;
				}
				*/


				/*
				else if (buffer[1] == 2) {
					//call validation on inpuBuffer
					//if (validate(inputBuffer)) {
						char ack[2];
						ack[0] = (char)22;
						ack[1] = (char)6;
						//ack[2] = '\0';
						
						
						//send ACK control frame
						WriteFile(port, ack, sizeof(ack), &bitsWritten, NULL);
						inputBuffer = buffer;
						print();
					//}
					timeout = false;
					//reset timeout thread;
				}

				*/
			}
		}
	}
}