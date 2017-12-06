#include "Receive.h"
#include "Header.h"
#include "Print.h"
#include "Main.h"
#include "crc.h"

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
				if (Validation(inputBuffer)) {

					print();
					KillTimer(hwnd, TIMER_TEST);
					char ack[2];
					ack[0] = 22;
					ack[1] = 6;
					writeToPort(ack, 2);
					startTimer(5000);
					memset(inputBuffer, 0, 518);
				}
			}
		}
	}
}


bool Validation(char* input)
{
	char temp[512];
	unsigned char bytes[4];
	memcpy(temp, &input[2], sizeof(char) * 512);
	uint32_t crc = CRC::Calculate(temp, strlen(temp), CRC::CRC_32());
	unsigned long shift = crc;
	bytes[0] = (shift >> 24) & 0xFF;
	bytes[1] = (shift >> 16) & 0xFF;
	bytes[2] = (shift >> 8) & 0xFF;
	bytes[3] = shift & 0xFF;
	for (int i = 0, j = 514; i < 4; i++, j++)
	{
		if (bytes[i] != temp[j])
		{
			return false;
		}
	}
	return true;
}