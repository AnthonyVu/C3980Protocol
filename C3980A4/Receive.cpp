#include "Receive.h"
#include "Header.h"
#include "Print.h"


void Recieve() {
	DWORD bitsWritten;
	while (timerGlobal != true) {
		if (inputBuffer[1] == 4) {
			return;
		}
		else if (outputBuffer == RVI) { //?undetermined
			char rvi[3];
			rvi[0] = (char)22;
			//rvi[1] = (char) ? ;

			WriteFile(port, rvi, sizeof(RVI), &bitsWritten, NULL);
			
			return;
		}
		else if (inputBuffer[1] == 2) {
			//call validation on inpuBuffer
			if (validate(inputBuffer)) {
				char ack[3];
				ack[0] = (char)22;
				ack[1] = (char)6;
				ack[2] = '\0';

				WriteFile(port, ack, sizeof(RVI), &bitsWritten, NULL);
				//send ACK control frame
				print();
			}
			timerGlobal = false;
		}
		
	}
}