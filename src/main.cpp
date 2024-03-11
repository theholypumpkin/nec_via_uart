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
#include <StateMachine.h>
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
String jsonToSend = "";
/*===============================================================================================*/
/* global objects */
SoftwareSerial SoftSerial(SOFTWARE_SERIAL_RX_PIN, SOFTWARE_SERIAL_TX_PIN);
MCP23S17 MCP(MCP23S17_CS_PIN);

/* Why an OPP Statemachine? Because I always wanted to use the library, but ist not worth it 
 * compared to a simple switch case. But here I have no more than 4 states and no real performace
 * or power constrains. So why not do it the hard way.
 */
StateMachine machine= StateMachine();
State *ReadNec      = machine.addState(&readNecState);
State *ReadSerial   = machine.addState(&readSerialState);
State *WriteSerial  = machine.addState(&writeSerialState);
State *PrintSegment = machine.addState(&printSegmentsState);
/*===============================================================================================*/
/* methods */
void setup(void) {
    /* Bitbang UART as the Seeedino XIAO I use for the project, has 4 out of 11 Puis being broken.
     * including on of the Hardware UART pins. So here we go I bitbang it.
     */
    SoftSerial.begin(SOFTWARE_SERIAL_BAUD); // SoftSerial is slow
    Serial.begin(115200); // USB Serial only for debugging.

    IrReceiver.begin(IR_RECEIVE_PIN, true); // begin the IrReceiver
    /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
    //begin the SPI interface for the MCP23S17
    SPI.begin(); 
    MCP.begin();
    // init the MCP23S17
    // set all pins as output (technically GPA0 to GPA7 are inputs, as they are the cathode)
    MCP.pinMode16(0x0000);
    /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
    // transition to itself as entrypoint into the statemachine
    PrintSegment->addTransition(&printSegmentsPrintSegmentsTransition, PrintSegment);
    // transitions to other states
    PrintSegment->addTransition(&printSegmentsReadNecTransition, ReadNec);
    PrintSegment->addTransition(&printSegmentsReadSerialTransition, ReadSerial);
    ReadNec->addTransition(&readNecWriteSerialTransition, WriteSerial);
    WriteSerial->addTransition(&writeSerialPrintSegmentsTransition, PrintSegment);
    ReadSerial->addTransition(&readSerialPrintSegmentsTransition, PrintSegment);
}
/*_______________________________________________________________________________________________*/

void loop() { machine.run(); }

/*_______________________________________________________________________________________________*/
/* state machine states*/

/** @brief The state to read the NEC-Protocol from the IR-Receiver */
void readNecState(void){
    JsonDocument doc;
    doc["addr"] = IrReceiver.decodedIRData.address;
    doc["cmd"] = IrReceiver.decodedIRData.command;
    /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
    /*Serialize Json into String*/
    size_t jsonStringLength = serializeJson(doc, jsonToSend);
    /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
    // Allow next ir-signal to be received.
    IrReceiver.resume();
}

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The state to read the JSON-String on the SoftSerial interface */
void readSerialState(void){
    String jsonString = SoftSerial.readStringUntil('\0'); //Terminate on null terminator
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonString);

    if (err == DeserializationError::Ok){
        // TODO deal with the JSONArray to display on 7Segment Display
        //JsonArray arr = doc["sgm"].as<JsonArray>();
        //NOTE Temporary for debugging
        Serial.println(doc["sgm"].as<String>()); // converts array to string
    }
    else{
        Serial.print("Error");
        Serial.println(jsonString);
    }
}

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The state to write the JSON-String to the SoftSerial interface */
void writeSerialState(void){
    IrReceiver.stop(); // stop the IR-Receiver, so we can Print to Serial
    // calculate compensation 
    uint32_t compensateValue = IR_COMPENSATION_CONSTANT * jsonStringLength;
    SoftSerial.write(jsonToSend); // Write JSON-String to Serial
    SoftSerial.flush(); // empty buffer
    IrReceiver.start(compensateValue); //Restart the Receiver and compensate for stop time.
}

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The state to print the 4 symbols code to the segment display */
void printSegmentsState(void){
    
    uint8_t arr[4] =  {1,2,3,4}; // TODO replace with global buffer
    
    for (uint8_t x = 0, y = 1; x < 4; x++){
        // upper byte controlls cathods to selegt each led, lower byte controls segments a,b,c,d
        uint16_t mcpRegisterValue = (seven_segment_lut[arr[x]] << 8) | y;
        y = y << 1; // next segment selct line
        /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
        MCP.write16(mcpRegisterValue);
        /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
        delayMicroseconds(5208); // 48 fps
    }
}

/*_______________________________________________________________________________________________*/
/* state machine transitions */

/** @brief Transition to itself, entrypoint for statemachine */
bool printSegmentsPrintSegmentsTransition(void){
    return true; // BUG this could cause a death loop, as this is the first transistion to be
    // evaluated and it retruns true
}

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the transition to reading the NEC-Protocol form the IR-Receiver to writing the data to
 * serial. Always returns true;
 */
bool readNecWriteSerialTransition(void){ return true; }

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to printing the segments, to reading the IR-Receiver */
bool printSegmentsReadNecTransition(void){
    return IrReceiver.decode(); // return true when we can decode a new NEC-Transmission.
}

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to printing the segments, to reading the SoftSerial interface */
bool printSegmentsReadSerialTransition(void){
    return SoftSerial.available(); // return true of there is a JSON-String to read.
}

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to reading the SoftSerial interface , to printing the segments 
 * Always returns true
 */
bool readSerialPrintSegmentsTransition(void){ return true; }

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to printing the segments, to reading the SoftSerial interface 
 * Always returns true
 */
bool writeSerialPrintSegmentsTransition(void){ return true; }

/*===============================================================================================*/
/* end of file */