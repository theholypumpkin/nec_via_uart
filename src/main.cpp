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
#include <IRremote.hpp>
#include <ArduinoJson.h>
/*===============================================================================================*/
/* macros */
#define IR_SEND_PIN 4
#define DISABLE_CODE_FOR_RECEIVER // we don't receive NEC via IR, only via UART.
#define NEC_NO_REPEAT 0
#define ENABLE_LED_FEEDBACK true
/*===============================================================================================*/
/* enums, typedef, structs, unions */
enum statemachine_e{
    READ_SERIAL_STATE,
    SEND_NEC_STATE,
    WRITE_SERIAL_STATE,
    WRITE_DISPLAY_STATE
};
/*===============================================================================================*/
/* variables */
/*===============================================================================================*/
/* global objects */
/*===============================================================================================*/
/* methods */

/** @brief setup for the first core handeling Serial */
void setup(void) {
    Serial.begin(9600); // not just debugging, it is needed for the project
    IrSender.begin(ENABLE_LED_FEEDBACK);

}

/* _____________________________________________________________________________________________ */
/** @brief main loop for the first core */
void loop(void) {
  IrSender.sendNEC(0x8474, 0xFF00, NEC_NO_REPEAT);
  delay(2500);
}

/* _____________________________________________________________________________________________ */
/** @brief setup for the second core the IR-Sending */
void setup1(void) {

}
/* _____________________________________________________________________________________________ */
/** @brief main loop for the second core */
void loop1(void) {
  // put your main code here, to run repeatedly:
}

/* _____________________________________________________________________________________________ */
/** @brief Free-RTOS Task to launch the second core with setup1 and loop1*/
void esploop1(void* pvParameters) {
  setup1();

  for (;;)
    loop1();
}

/*===============================================================================================*/
/* end of file */