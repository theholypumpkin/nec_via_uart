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
enum statemachine_e{
    READ_SERIAL_STATE,
    READ_NEC_STATE,
    WRITE_SERIAL_STATE,
    WRITE_DISPLAY_STATE
};
/*===============================================================================================*/
/* global variables */
uint8_t sevensegfonttable[] = {
        0b11111111, //   (space) all high
        0b01111001, // ! like '1' with decimal point
        0b01011111, // " (double aporstroph)
        0b11111111, // # (pound) doesn't exit so all is high
        0b11111111, // $ (dollar) doesn't exit so all is high
        0b11111111, // % (percent) doesn't exit so all is high
        0b11111111, // & (ampersand) doesn't exit so all is high
        0b01111111, // '
        0b10001110, // (
        0b00110100, // )
        0b11111111, // * doesn't exit so all is high
        0b11111111, // + doesn't exit so all is high
        0b11111011, // , (comma)
        0b11111110, // - (minus)
        0b11111011, // . (periode)
        0b01101110, // /
        0b00000101, // 0
        0b01111101, // 1
        0b00100110, // 2
        0b00110100, // 3
        0b01011100, // 4
        0b10010100, // 5
        0b10000100, // 6
        0b00111101, // 7
        0b00000100, // 8
        0b00010100, // 9
        0b11111111, // : (colon) all high can't be displayed
        0b11111111, // ; (semi colon) all high can't be displayed
        0b11111111, // < (less than) all high can't be displayed
        0b11110110, // = (equal)
        0b11111111, // > (greater than) all high can't be displayed
        0b00101010, // ? (questionmark)
        0b00100100, // @ (at-sign)
        // capitalization is ignored so the tabes repeats twice
        0b00001100, // A
        0b11000100, // B is lower case
        0b10000111, // C
        0b01100100, // D is lower case
        0b10000110, // E
        0b10001100, // F
        0b10000101, // G
        0b01001100, // H
        0b01111101, // I same as '1'
        0b00100101, // J
        0b10001100, // K is lower case, looks funky
        0b11000111, // L
        0b10101101, // M is lower case 
        0b11101100, // N is lower case 
        0b00000101, // O same as '0'
        0b00001110, // P
        0b00000001, // Q same as '0' with decimal point
        0b11101110, // R is lower case
        0b10010100, // S same as '5'
        0b11000110, // T is lower case
        0b01000101, // U
        0b01000101, // V same as 'U'
        0b01010111, // W is lower case
        0b11111111, // X doesn't exit so all is high
        0b01010100, // Y is lower case
        0b00100110, // Z same as '2'
        0b10001110, // [ same as (
        0b11011100, // (backslash)
        0b00110100, // ] same as )
        0b11111111, // ^ (caret) doesn't exit so all is high
        0b11110111, // _
        0b11111111, // ` (grave) doesn't exit so all is high
        //repeating the same values again but this time mapped to lower case
        // capitalization is still ignored
        0b00001100, // a is upper case
        0b11000100, // b 
        0b11100110, // c
        0b01100100, // d
        0b10000110, // e is upper case
        0b10001100, // f is upper case
        0b00010100, // g same as '9'
        0b11001100, // h
        0b11111101, // i
        0b11110101, // j
        0b10001100, // k looks funky
        0b11001111, // l
        0b10101101, // m
        0b11101100, // n
        0b11100100, // o
        0b00001110, // p is upper case
        0b00011100, // q
        0b11101110, // r
        0b10010100, // s same as '5'
        0b11000110, // t
        0b11100101, // u
        0b11100101, // v same as 'U'
        0b01010111, // w 
        0b11111111, // x doesn't exit so all is high
        0b01010100, // y
        0b00100110, // z same as '2'
        0b10001110, // { same as (
        0b11001111, // | (pipe) same as lower case 'L'
        0b00110100, // } same as )
        0b11111110, // ~ (tilde) same as minus
        0b11111111  // <- (DEL) doesn't exist
    };
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
    SoftSerial.setTimeout(2); // only wait for at max 2.5 bytes before giving up to read
    //Serial.begin(115200); // USB Serial only for debugging.

    IrReceiver.begin(IR_RECEIVE_PIN, true); // begin the IrReceiver
    /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
    //begin the SPI interface for the MCP23S17
    SPI.begin(); 
    MCP.begin();
    // init the MCP23S17
    // set all pins as output (technically GPA0 to GPA7 are inputs, as they are the cathode)
    MCP.pinMode16(0x0000);
}
/*_______________________________________________________________________________________________*/

void loop() {
    static statemachine_e e_state; // No State at init so we will go into default
    static String jsonToSend = "";
    static uint32_t compensateValue = 0;
    static uint8_t fontMapping[256];

    switch(e_state){
        case READ_SERIAL_STATE:
            {   
                e_state = WRITE_DISPLAY_STATE; // go back to default state after this one
                /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                //Terminate on null terminator
                String jsonToReceive = SoftSerial.readStringUntil('\0');
                JsonDocument jsonDoc;
                DeserializationError err = deserializeJson(jsonDoc, jsonToReceive);
                // if it is not a valid json go back to default state
                if (err != DeserializationError::Ok) break;
            // extract array
                /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                JsonArray jsonArr = jsonDoc["sgm"].as<JsonArray>();
                uint8_t cnt = 0;

                for (JsonVariant value : jsonArr){
                    fontMapping[cnt] = (uint8_t) value.as<const char*>()[0] - 0x20;
                    cnt++; // wraps around after 256 bytes
                }
            }
            break;
        /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
        case READ_NEC_STATE:
            {
                JsonDocument jsonDoc;
                jsonDoc["addr"] = IrReceiver.decodedIRData.address;
                jsonDoc["cmd"] = IrReceiver.decodedIRData.command;
                /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                /*Serialize Json into String*/
                compensateValue = serializeJson(jsonDoc, jsonToSend) * IR_COMPENSATION_CONSTANT;
                /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                // Allow next ir-signal to be received.
                IrReceiver.resume();
                // Transition unter the curcumstance, when jsonToSend String is not an empty String.
                // Set transitions below switch case
                //jsonDoc.clear(); // empty JsonDocument
            }
            break;
        /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
        case WRITE_SERIAL_STATE:
            IrReceiver.stop(); // stop the IR-Receiver, so we can Print to Serial
            /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
            // calculate compensation 
            SoftSerial.print(jsonToSend); // Write JSON-String to Serial
            SoftSerial.flush(); // empty buffer
            jsonToSend = ""; // empty the string
            /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
            delayMicroseconds(IR_DELAY_US);
            //Restart the Receiver and compensate for stop time.
            IrReceiver.start(compensateValue + IR_DELAY_US);
            /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
            e_state = WRITE_DISPLAY_STATE; // resume default state
            break;
        /* -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   - */
        case WRITE_DISPLAY_STATE:
        default:
            // TODO allow more then 4 character by scrolling? 
            // Right now I can read in up to 256 characters but only ever print 4.
            for (uint8_t x = 0, y = 1; x < 4; x++){
                // upper byte controls segments (cahode), lower byte controls digit (anode)
                uint16_t mcpRegisterValue = (sevensegfonttable[fontMapping[x]] << 8) | y;
                y = y << 1; // next digit select shift anodle line.
                /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                MCP.write16(mcpRegisterValue);
                /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                delayMicroseconds(5208); // 48 fps
            }
            e_state = WRITE_DISPLAY_STATE;
            break;
    };
    /*___________________________________________________________________________________________*/
    // Transition between states
    if(SoftSerial.available()){ e_state = READ_SERIAL_STATE; }
    if(IrReceiver.decode()){ e_state = READ_NEC_STATE; }
    // if not equal to empty string transition.
    if(!jsonToSend.equals("")){e_state = WRITE_SERIAL_STATE; }
}

/*===============================================================================================*/
/* end of file */