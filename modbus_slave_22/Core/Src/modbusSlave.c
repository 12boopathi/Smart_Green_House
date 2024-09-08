/*
 * modbusSlave.c
 *
 *  Created on: Aug 16, 2024
 *      Author: boobathi
 */

#include "modbusSlave.h"
#include "string.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include <math.h>

extern uint8_t RxData[256];
extern uint8_t TxData[256];
extern uint16_t Holding_Registers_Database[];
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

// Temperature sensor calibration values
#define R1 10000.0
#define C1 0.001129148
#define C2 0.000234125
#define C3 0.0000000876741

float T;
uint16_t tempInt;

char uart_buf[100];
// Function to read temperature (No Hold Master Mode)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        uint16_t adcValue = HAL_ADC_GetValue(hadc);

        char uart_buf[50];
        snprintf(uart_buf, sizeof(uart_buf), "ADC Value: %u\r\n", (unsigned int)adcValue);
        HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

        float voltage = (3.3f / 4095.0f) * adcValue;
        float R2 = R1 * ((3.3f / voltage) - 1.0);
        float logR2 = log(R2);
        float T = 1.0f / (C1 + C2 * logR2 + C3 * logR2 * logR2 * logR2);
        T -= 273.15f;  // Convert Kelvin to Celsius

        // Convert temperature to 16-bit integer
        uint16_t tempInt = (uint16_t)((T + 40.0f) * 10.0f);  // Example conversion, adjust as needed

        // Store temperature in Holding Registers
        Holding_Registers_Database[1] = tempInt;

        // Call writeHoldingRegs to update register value
        writeHoldingRegs();
    }
}

float readTemperature(void) {
    // Start ADC conversion in interrupt mode

    // Return the stored temperature for verification
    return (float)Holding_Registers_Database[1] / 10.0f - 40.0f; // Convert back to Celsius
}




// Function to send data with CRC calculation
void sendData(uint8_t *data, int size) {
    uint16_t crc = crc16(data, size);
    data[size] = crc & 0xFF;          // CRC LOW
    data[size + 1] = (crc >> 8) & 0xFF; // CRC HIGH
    HAL_UART_Transmit(&huart2, data, size + 2, 1000);
}

// Function to handle Modbus exceptions
void modbusException(uint8_t exceptionCode) {
    TxData[0] = RxData[0];          // Slave ID
    TxData[1] = RxData[1] | 0x80;   // Exception code
    TxData[2] = exceptionCode;      // Load the Exception code
    sendData(TxData, 3);
}

// Function to read Holding Registers
uint8_t readHoldingRegs(void) {
    uint16_t startAddr = ((RxData[2] << 8) | RxData[3]);  // start Register Address
    uint16_t numRegs = ((RxData[4] << 8) | RxData[5]);   // number of registers master has requested

    if ((numRegs < 1) || (numRegs > 125)) {  // maximum number of Registers as per the PDF
        modbusException(ILLEGAL_DATA_VALUE);  // send an exception
        return 0;
    }

    uint16_t endAddr = startAddr + numRegs - 1;  // end Register
    if (endAddr > 49) {  // end Register cannot be more than 49 as we only have record of 50 Registers in total
        modbusException(ILLEGAL_DATA_ADDRESS);   // send an exception
        return 0;
    }

    // Prepare TxData buffer
    // | SLAVE_ID | FUNCTION_CODE | BYTE COUNT | DATA      | CRC     |
    // | 1 BYTE   |  1 BYTE       |  1 BYTE    | N*2 BYTES | 2 BYTES |
    TxData[0] = SLAVE_ID;  // slave ID
    TxData[1] = RxData[1];  // function code
    TxData[2] = numRegs * 2;  // Byte count
    int indx = 3;  // we need to keep track of how many bytes have been stored in TxData Buffer

    for (int i = 0; i < numRegs; i++) {   // Load the actual data into TxData buffer
        if (startAddr == 1) {
            // Read the temperature from Holding Registers and convert to 16-bit integer
            uint16_t tempInt = Holding_Registers_Database[startAddr];
            TxData[indx++] = (tempInt >> 8) & 0xFF;  // extract the higher byte
            TxData[indx++] = tempInt & 0xFF;  // extract the lower byte
        } else {
            TxData[indx++] = (Holding_Registers_Database[startAddr] >> 8) & 0xFF;  // extract the higher byte
            TxData[indx++] = (Holding_Registers_Database[startAddr]) & 0xFF;   // extract the lower byte
        }
        startAddr++;  // increment the register address
    }

    sendData(TxData, indx);  // send data... CRC will be calculated in the function itself
    return 1;   // success
}

// Function to write to Holding Registers
uint8_t writeHoldingRegs(void) {
    uint16_t startAddr = ((RxData[2] << 8) | RxData[3]);
    uint16_t numRegs = ((RxData[4] << 8) | RxData[5]);

    if (numRegs < 1 || numRegs > 123) {
        modbusException(ILLEGAL_DATA_VALUE);
        return 0;
    }

    uint16_t endAddr = startAddr + numRegs - 1;
    if (endAddr > 49) {
        modbusException(ILLEGAL_DATA_ADDRESS);
        return 0;
    }

    int indx = 7;
    for (int i = 0; i < numRegs; i++) {
        Holding_Registers_Database[startAddr] = (RxData[indx] << 8) | RxData[indx + 1];
        indx += 2;
        startAddr++;
    }

    TxData[0] = SLAVE_ID;
    TxData[1] = RxData[1];
    TxData[2] = RxData[2];
    TxData[3] = RxData[3];
    TxData[4] = RxData[4];
    TxData[5] = RxData[5];

    sendData(TxData, 6);
    return 1;
}


uint8_t writeSingleReg(void) {
    uint16_t startAddr = ((RxData[2] << 8) | RxData[3]);

    if (startAddr > 49) {
        modbusException(ILLEGAL_DATA_ADDRESS);
        return 0;
    }

    Holding_Registers_Database[startAddr] = (RxData[4] << 8) | RxData[5];

    TxData[0] = SLAVE_ID;
    TxData[1] = RxData[1];
    TxData[2] = RxData[2];
    TxData[3] = RxData[3];
    TxData[4] = RxData[4];
    TxData[5] = RxData[5];

    sendData(TxData, 6);
    return 1;
}
