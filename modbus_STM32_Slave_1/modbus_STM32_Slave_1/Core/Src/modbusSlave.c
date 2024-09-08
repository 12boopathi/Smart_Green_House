

#include "modbusSlave.h"
#include "string.h"
#include "main.h"
extern uint8_t RxData[256];
extern uint8_t TxData[256];
extern UART_HandleTypeDef huart2;

#define RELAY_REGISTER_ADDRESS 0x05


void turnRelayOn(void)
{
    HAL_GPIO_WritePin(Relay_out_GPIO_Port, Relay_out_Pin, GPIO_PIN_SET);  // Replace GPIOx and GPIO_PIN_y with actual GPIO port and pin
}

// Function to turn the relay off
void turnRelayOff(void)
{
    HAL_GPIO_WritePin(Relay_out_GPIO_Port, Relay_out_Pin, GPIO_PIN_RESET);  // Replace GPIOx and GPIO_PIN_y with actual GPIO port and pin
}

void sendData (uint8_t *data, int size)
{
	// we will calculate the CRC in this function itself
	uint16_t crc = crc16(data, size);
	data[size] = crc&0xFF;   // CRC LOW
	data[size+1] = (crc>>8)&0xFF;  // CRC HIGH

	HAL_UART_Transmit(&huart2, data, size+2, 1000);
}

void modbusException (uint8_t exceptioncode)
{
	//| SLAVE_ID | FUNCTION_CODE | Exception code | CRC     |
	//| 1 BYTE   |  1 BYTE       |    1 BYTE      | 2 BYTES |

	TxData[0] = RxData[0];       // slave ID
	TxData[1] = RxData[1]|0x80;  // adding 1 to the MSB of the function code
	TxData[2] = exceptioncode;   // Load the Exception code
	sendData(TxData, 3);         // send Data... CRC will be calculated in the function
}


uint8_t readHoldingRegs (void)
{
	uint16_t startAddr = ((RxData[2]<<8)|RxData[3]);  // start Register Address

	uint16_t numRegs = ((RxData[4]<<8)|RxData[5]);   // number to registers master has requested
	if ((numRegs<1)||(numRegs>125))  // maximum no. of Registers as per the PDF
	{
		modbusException (ILLEGAL_DATA_VALUE);  // send an exception
		return 0;
	}

	uint16_t endAddr = startAddr+numRegs-1;  // end Register
	if (endAddr>49)  // end Register can not be more than 49 as we only have record of 50 Registers in total
	{
		modbusException(ILLEGAL_DATA_ADDRESS);   // send an exception
		return 0;
	}

	// Prepare TxData buffer

	//| SLAVE_ID | FUNCTION_CODE | BYTE COUNT | DATA      | CRC     |
	//| 1 BYTE   |  1 BYTE       |  1 BYTE    | N*2 BYTES | 2 BYTES |

	TxData[0] = SLAVE_ID;  // slave ID
	TxData[1] = RxData[1];  // function code
	TxData[2] = numRegs*2;  // Byte count
	int indx = 3;  // we need to keep track of how many bytes has been stored in TxData Buffer

	for (int i=0; i<numRegs; i++)   // Load the actual data into TxData buffer
	{
		TxData[indx++] = (Holding_Registers_Database[startAddr]>>8)&0xFF;  // extract the higher byte
		TxData[indx++] = (Holding_Registers_Database[startAddr])&0xFF;   // extract the lower byte
		startAddr++;  // increment the register address
	}

	sendData(TxData, indx);  // send data... CRC will be calculated in the function itself
	return 1;   // success
}


uint8_t writeHoldingRegs (void)
{
	uint16_t startAddr = ((RxData[2]<<8)|RxData[3]);  // start Register Address

	uint16_t numRegs = ((RxData[4]<<8)|RxData[5]);   // number to registers master has requested
	if ((numRegs<1)||(numRegs>123))  // maximum no. of Registers as per the PDF
	{
		modbusException (ILLEGAL_DATA_VALUE);  // send an exception
		return 0;
	}

	uint16_t endAddr = startAddr+numRegs-1;  // end Register
	if (endAddr>49)  // end Register can not be more than 49 as we only have record of 50 Registers in total
	{
		modbusException(ILLEGAL_DATA_ADDRESS);   // send an exception
		return 0;
	}

	/* start saving 16 bit data
	 * Data starts from RxData[7] and we need to combine 2 bytes together
	 * 16 bit Data = firstByte<<8|secondByte
	 */
	int indx = 7;  // we need to keep track of index in RxData
	for (int i=0; i<numRegs; i++)
	{
		Holding_Registers_Database[startAddr++] = (RxData[indx++]<<8)|RxData[indx++];
	}

	// Prepare Response

	//| SLAVE_ID | FUNCTION_CODE | Start Addr | num of Regs    | CRC     |
	//| 1 BYTE   |  1 BYTE       |  2 BYTE    | 2 BYTES      | 2 BYTES |

	TxData[0] = SLAVE_ID;    // slave ID
	TxData[1] = RxData[1];   // function code
	TxData[2] = RxData[2];   // Start Addr HIGH Byte
	TxData[3] = RxData[3];   // Start Addr LOW Byte
	TxData[4] = RxData[4];   // num of Regs HIGH Byte
	TxData[5] = RxData[5];   // num of Regs LOW Byte

	sendData(TxData, 6);  // send data... CRC will be calculated in the function itself
	return 1;   // success
}

uint8_t writeSingleReg(void)
{
    uint16_t coilAddr = ((RxData[2] << 8) | RxData[3]);  // Coil Address
    uint16_t coilValue = ((RxData[4] << 8) | RxData[5]);  // Coil Value

    // Ensure the coil address is within valid range


    if(coilAddr == RELAY_REGISTER_ADDRESS ){
    // Handle coil value to set or reset coil
    if (coilValue == 0xFF00) {
         // Reset coil (turn relay off)
         Coils_Database[coilAddr / 8] &= ~(1 << (coilAddr % 8));
         turnRelayOff();
     } else if (coilValue == 0x0000) {
         // Set coil (turn relay on)
         Coils_Database[coilAddr / 8] |= (1 << (coilAddr % 8));
         turnRelayOn();
     } else {
         modbusException(ILLEGAL_DATA_VALUE);  // Invalid value for a coil
         return 0;
     }

    }

    // Prepare response
    TxData[0] = SLAVE_ID;  // Slave ID
    TxData[1] = RxData[1];  // Function code
    TxData[2] = RxData[2];  // Coil address high byte
    TxData[3] = RxData[3];  // Coil address low byte
    TxData[4] = RxData[4];  // Coil value high byte
    TxData[5] = RxData[5];  // Coil value low byte

    // Send response with CRC calculation
    sendData(TxData, 6);
    return 1;  // Success
}




