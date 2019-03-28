#include "Arduino.h"
#include "spc_bus.h"
#include "spc_writer.h"
#include "uart.h"

#define PORT_0_PIN 8
#define PORT_1_PIN 9

#define READ_PIN 10
#define WRITE_PIN 11

#define DATA_0_PIN 12
#define DATA_1_PIN 13
#define DATA_2_PIN 2
#define DATA_3_PIN 3
#define DATA_4_PIN 4
#define DATA_5_PIN 5
#define DATA_6_PIN 6
#define DATA_7_PIN 7

#define RESET_PIN A0

#define BAUD_RATE 115200


SpcBus spcBus(READ_PIN, WRITE_PIN, RESET_PIN);
SpcWriter spcWriter(spcBus);


void readPorts() {
    for (int i = 0; i < 4; i++) {
        Serial.write(spcBus.read(i));
    }
}

void handleSerialCommand() {
    if (Serial.available()) {
        char result = Serial.read();
        int error;
        uint16_t ram_address;
        uint8_t data;

        switch (result) {
            case 'Q': // Read value of all four ports.
                readPorts();
                break;

            case 'R': // Reset SPC.
                spcWriter.reset();
                Serial.write('R');
                break;

            case 'S': // Start SPC execution from given ram address.
                ram_address = Uart::readShort(&error);
                if (error)
                    break;
                spcWriter.start(ram_address);
                Serial.write((uint8_t)(ram_address >> 8));
                Serial.write((uint8_t)(ram_address & 0xff));
                break;

            case 'D': // Write 128 DSP registers with dsp address and data.
                for (uint8_t i = 0; i < 128; i++) {
                    data = Uart::readByte(&error);
                    if (error)
                        break;
                    spcWriter.setAddress(0x00F2);
                    spcWriter.write(i); // Write DSP register address
                    spcWriter.write(data); // Write register value
                    Serial.write(data);
                }
                break;

            case 'B': // Write block of bytes with ram address and length.
                ram_address = Uart::readShort(&error);
                if (error)
                    break;
                Serial.write((uint8_t)(ram_address >> 8));
                Serial.write((uint8_t)(ram_address & 0xff));
                uint16_t length = Uart::readShort(&error);
                if (error)
                    break;
                Serial.write((uint8_t)(length >> 8));
                Serial.write((uint8_t)(length & 0xff));
                spcWriter.setAddress(ram_address);
                for (unsigned int i = 0; i < length; i++) {
                    data = Uart::readByte(&error);
                    if (error)
                        break;
                    spcWriter.write(data);
                    Serial.write(data);
                }
                break;
        }
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    spcBus.setPortPins(PORT_0_PIN, PORT_1_PIN);
    spcBus.setDataPins(DATA_0_PIN, DATA_1_PIN, DATA_2_PIN, DATA_3_PIN, DATA_4_PIN, DATA_5_PIN, DATA_6_PIN, DATA_7_PIN);
    spcWriter.reset();
    Serial.write('R');
}

void loop() {
    handleSerialCommand();
}