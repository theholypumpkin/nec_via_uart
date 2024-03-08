/* main.cpp */
/*===============================================================================================*/
/*
    GNU GENERAL PUBLIC LICENSE Version 3
    See LICENCE-File for details
*/
/*===============================================================================================*/
/*
    author: theholypumpkin
    date: 2024/02/07
    version: 0.0
*/
/*===============================================================================================*/
/* header files */
#include "Arduino.h"
#include <MCP23S17.h>
#include <IRremote.hpp>
#include <SoftwareSerial.h> // TX pin is broken so bitbanging

#include <ArduinoJson.h> 
/*===============================================================================================*/
/* macros */
#define SOFTWARE_SERIAL_TX_PIN 2
#define SOFTWARE_SERIAL_RX_PIN 1
#define SOFTWARE_SERIAL_BAUD 9600

#define IR_RECEIVE_PIN 3
#define MCP23S17_CS_PIN 7
#define IR_DELAY_US 30000
#define IR_COMPENSATION_CONSTANT 1.0f / SOFTWARE_SERIAL_BAUD * 1000000

#define DECODE_NEC
/*===============================================================================================*/
/* enums, typedef, structs, unions */
/*===============================================================================================*/
/* global variables */
/*===============================================================================================*/
/* global objects */
SoftwareSerial SoftSerial(SOFTWARE_SERIAL_RX_PIN, SOFTWARE_SERIAL_TX_PIN);
MCP23S17 MCP(MCP23S17_CS_PIN);
/*===============================================================================================*/
/* methods */
void setup(void) {
    /* Bitbang UART as the Seeedino XIAO I use for the project, has 4 out of 11 Puis being broken.
     * including on of the Hardware UART pins. So here we go I bitbang it.
     */
    SoftSerial.begin(SOFTWARE_SERIAL_BAUD); // SoftSerial is slow
    Serial.begin(115200); // USB Serial only for debugging.

    IrReceiver.begin(IR_RECEIVE_PIN, true); // begin the IrReceiver

    SPI.begin(); //begin the SPI interface for the MCP23S17
    MCP.begin();
}
/*_______________________________________________________________________________________________*/

void loop() {
    //uint16_t address = 0, command = 0;
    if (IrReceiver.decode()) {
        // generate a JSON document
        JsonDocument doc;
        doc["addr"] = IrReceiver.decodedIRData.address;
        doc["cmd"] = IrReceiver.decodedIRData.command;
        /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
        /* pause the ir-receiver to allow SoftSerial to work, as the ir-receiver is using 
         * interrupts. This conflixts with the Timings of SoftSerial.
         */
        IrReceiver.stop();
        /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
        /*send the JSON String via SoftSerial*/
        size_t jsonStringLength = serializeJson(doc, SoftSerial);
        /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
        // delay and calculate by how much we have to compensate to restart the ir-receiver
        delayMicroseconds(IR_DELAY_US); // so we don't get double commands
        uint32_t compensateValue = IR_COMPENSATION_CONSTANT * jsonStringLength + IR_DELAY_US;
        IrReceiver.start(compensateValue); //Restart the Receiver and  compensate for stop time.
        /*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
        // Allow next ir-signal to be received.
        IrReceiver.resume();

    }
}

/*===============================================================================================*/
/* end of file */