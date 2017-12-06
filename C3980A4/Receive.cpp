/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Recieve.cpp - Handles the receiving section of the protocol
--
-- PROGRAM: C3980A4
--
-- FUNCTIONS:
-- void Receive()
-- bool Validation(char* input)
--
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew, Anthony Vu
--
-- NOTES:
-- This handles the receiving and validation of frames to be printed to the screen. 
----------------------------------------------------------------------------------------------------------------------*/
#include "Receive.h"
#include "Header.h"
#include "Print.h"
#include "Main.h"
#include "crc.h"
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Receive
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Matthew Shew
--
-- INTERFACE: void Receive()
--
-- RETURNS: void
--
-- NOTES:
-- This functions checks the inputBuffer and processes the data in the inputBuffer depending on the what the data contains. 
-- If the inputBuffer contains an EOT frame, the function will clear the inputBuffer, close the timer, and return from the functionn to idle state. 
-- If the inputBuffer contains a data frame, the function will check the data for validation. If the data frame is valid the function will call 
-- print to print the data to the screen. The inputBuffer will then be cleared and an ack will be sent to the sender to confirm successful 
-- transmission. The timer will be reset and the function will continue to check the inputBuffer for more data. If the user has entered an 'r' on
-- the keyboard to say they want immediate access to the line (RVI) then the function will send an RVI frame to the sender and will clear the inputBuffer, 
-- close the timer and return to idle state. If the timer ends before any data has been put into the inputBuffer then the function will return and go to idle state.
----------------------------------------------------------------------------------------------------------------------*/
void Receive() {
	DWORD bitsWritten;
	startTimer(2000);
	memset(inputBuffer, 0, 518);
	while (!timeout) {
		if (inputBuffer[0] == SYN)//22 
		{
			//EOT
			if (inputBuffer[1] == EOT)//4 
			{
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
			else if (inputBuffer[1] == STX)//2 
			{
				//call validation on inpuBuffer
				if (Validation(inputBuffer)) {

					print();
					KillTimer(hwnd, TIMER_TEST);
					char ack[2];
					ack[0] = 22;
					ack[1] = 6;
					writeToPort(ack, 2);
					startTimer(2000);
					memset(inputBuffer, 0, 518);
				}
				else {
					memset(inputBuffer, 0, 518);
				}
				
			}
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Validation
--
-- DATE: December 3, 2017
--
-- REVISIONS: None
--
-- DESIGNER: Matthew Shew, Anthony Vu, Wilson Hu, Haley Booker
--
-- PROGRAMMER: Anthony Vu
--
-- INTERFACE: bool Validation(char* input)
--					char* input - the data from inputBuffer
--
-- RETURNS: bool - whether the input data was valid or not
--
-- NOTES:
-- This function calculates the CRC of the data frame and compares the calculated CRC with the CRC that was sent with the data frame. 
-- If the calculated CRC is the same as the CRC that was sent with the data frame then the data was valid and the function will return true. Otherwise 
-- function return false. 
----------------------------------------------------------------------------------------------------------------------*/
bool Validation(char* input)
{
	char temp[512];
	unsigned char bytes[4];
	memcpy(temp, &input[2], sizeof(char) * 512);
	uint32_t crc = CRC::Calculate(temp, 512, CRC::CRC_32());
	unsigned long shift = crc;
	bytes[0] = (shift >> 24) & 0xFF;
	bytes[1] = (shift >> 16) & 0xFF;
	bytes[2] = (shift >> 8) & 0xFF;
	bytes[3] = shift & 0xFF;
	for (int i = 0, j = 514; i < 4; i++, j++)
	{
		char a = bytes[i];
		char b = input[j];
		if (a != b)
		{
			updateInfo(&numBitErrors);
			return false;
		}
	}
	return true;
}