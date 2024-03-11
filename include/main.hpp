/* main.hpp */
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
/*===============================================================================================*/
/* macros */
/*===============================================================================================*/
/* enums, typedef, structs, unions */
/*===============================================================================================*/
/* variables */
/*===============================================================================================*/
/* methods */
/** @brief The arduino setup function */
void setup(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The arduino loop function */
void loop(void);

/*_______________________________________________________________________________________________*/
/* state machine states*/

/** @brief The state to read the NEC-Protocol from the IR-Receiver */
void readNecState(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The state to read the JSON-String on the SoftSerial interface */
void readSerialState(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The state to write the JSON-String to the SoftSerial interface */
void writeSerialState(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief The state to print the 4 symbols code to the segment display */
void printSegmentsState(void);

/*_______________________________________________________________________________________________*/
/* state machine transitions */

/** @brief Transition to itself, entrypoint for statemachine */
bool printSegmentsPrintSegmentsTransition(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the transition to reading the NEC-Protocol form the IR-Receiver to writing the data to
 * serial.
 */
bool readNecWriteSerialTransition(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to printing the segments, to reading the IR-Receiver */
bool printSegmentsReadNecTransition(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to printing the segments, to reading the SoftSerial interface */
bool printSegmentsReadSerialTransition(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to reading the SoftSerial interface , to printing the segments*/
bool readSerialPrintSegmentsTransition(void);

/*-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -  */
/** @brief the Transition to printing the segments, to reading the SoftSerial interface */
bool writeSerialPrintSegmentsTransition(void);

/*===============================================================================================*/
/* end of file */