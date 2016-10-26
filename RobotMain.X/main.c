   
/**
 * @file robotmain.c
 * @date 15 Oct 2016
 * @brief Implements control of the robot
 * 
 * Utilises timers, interrupts and function calls to implement control of the robot
 * 
 * @author Samuel Gleeson
 *
 * @bugs Still Writing
 */


/* -- Includes -- */
#include <p18f4520.h>            
//#include "ConfigRegs.h"
#include "ConfigRegs_18F4520.h"
#include "IRSensors.h"
#include "Communications.h"






/* -- Function Prototypes -- */
void high_interrupt(void);


// Encoders
//void SetupEncoders(void);
//void sampleEncoders(int* encoderValues);
//void calcEncoderValues(void);

char DriveMotors(unsigned int,char,char,unsigned int,char*);
void setup(void);


//Motors
void motorSetup(void);
//char DriveMotors(int magnitude,char direction,char mainFlag);






/* -- Global Variables -- */

//Target number of encoder increments (transmitted from main)
int targetEncoders[2] = {10,10};

//signal strength is modified by the receive interrupt
char signalStrength[] = {0x31,0x33,0x00};

//The current encoder values, modified in the readEncoders function
int currentEncoderVals[2];




//Buffer to store receive data
char receiveBuffer[50] = {0x00};

//Chirp Strength
char chirpStr;

//ReceiveFlag triggered when receive entered
char receiveFlag=0;
char *flaggerTest;
char crflag = 0;
char saver;

//MotorInstructionFlag
char instructionFlag2;
int instMag;
char instDir;
char Speed;





/* Interrupt Declarations */
#pragma code high_interrupt_vector = 0x000008
void goto_high_ISR(void) {
    _asm goto high_interrupt _endasm
}
#pragma code




/** 
 * @brief Main Function
 *  
 * Implements Robot Control
 * 
 *  
 * @return 
 */



void main(void) {
    int i;
    char detectVals[] = {0x01,0x02,0x03,0x00};
    char detailedFlagName = 0;

    instMag= 2000;
    instDir = 's';
    instructionFlag2 = 1; 
    //timerSetup();
    //SetupIR();
    
    //SetupEncoders();
    commsSetup();
    motorSetup();
    // setup();
    //Set up IR sensors
    //IRSetup();
    
    //debugSetup();
    
    //Timer 1 Setup
    //IPR1bits.TMR1IP = 1; // timer 1 overflow
    //PIE1bits.TMR1IE = 1;
    //T1CON = 0b10110001;
    receiveBuffer[49] = 0xFF;
    //Timer 3 Setup
    IPR2bits.TMR3IP = 1;
    PIE2bits.TMR3IE = 1;
    T3CON = 0b10110001;
    ADCON0bits.GO = 1;
    
    //Turn on timer 3

    
    
    INTCONbits.PEIE = 1;
    RCONbits.IPEN = 0;
    INTCONbits.GIE = 1;
    instructionFlag2 = 0;
    Speed = 25; 
    instMag= 0;
    instDir = 's';
    instructionFlag2 = 0; 
    
    transmitData(detectVals,signalStrength,instructionFlag2);
    
    /* Loop */
    while(1){
        //
        int test1;
        
        //Process Receive Function
            //add inputs global variables
        if (receiveFlag == 1) {

            processReceived(receiveBuffer, &instMag,&instDir,&instructionFlag2);
            receiveFlag=0;
            
        }
        test1 = instMag;
        if (instMag != 13288) {
            int test = 0;
        }
        //Perform PID or similar and drive motors
        instructionFlag2 = DriveMotors(instMag,instDir,instructionFlag2,Speed,&detailedFlagName);

        //if(detailedFlagName == 1){
            getRSSI(receiveBuffer, signalStrength, &receiveFlag, &crflag, &saver);
            transmitData(detectVals,signalStrength,instructionFlag2);
            detailedFlagName = 0;
        //}
        //Read IR sensor buffer and return result
        //IRDetect(2,detectVals);
        
        //if (detectVals[0] > 0) {
        //    i++;
        //}
        //if (detectVals[1] > 0) {
        //    i++;
       // }
       // if (detectVals[2] > 0) {
      //      i++;
      //  }
    Delay10KTCYx(250);   
    }
}

void high_interrupt(void) {

    
    // Disable Interrupts
    INTCONbits.GIE = 0;
    
 
    
        /* IRSensors Timer Interrupt */
    //if ((PIR1bits.TMR1IF == 1)&&(PIE1bits.TMR1IE == 1)) {
    //reset clock
        //TMR1H = 0;
        //TMR1L = 0;
        
        

        //sampleIR();
        //Begin new IR conversion
        //ADCON0bits.GO = 1;
    
        //Clear timer flag
        //PIR1bits.TMR1IF = 0;
        
        //PIR1 = 0;
       // PIR2 = 0;
        
        
    

    //}
    
    
    
    
        /* IRSensors Timer Interrupt */
    if ((PIR2bits.TMR3IF == 1)&&(PIE2bits.TMR3IE == 1)) {
        PIR2bits.TMR3IF = 0;
    //reset clock
        TMR3H = 0;
        TMR3L = 0;
        
        
        
//        sampleIR();
        //Begin new IR conversion
        ADCON0bits.GO = 1;
    
        //Clear timer flag
 
        
        
        PIR1 = 0;
        PIR2 = 0;
        
            IPR2bits.TMR3IP = 1;
    PIE2bits.TMR3IE = 1;
    T3CON = 0b10110001;
    ADCON0bits.GO = 1;
        
        
        
        
//    sampleEncoders(currentEncoderVals);
        //sampleIR();
        //ADCON0bits.GO = 1;
   
    
        //Clear timer flag
        

    }
    
    
    
    
    
        /*Serial Receive Interrupt*/
    if (PIR1bits.RCIF == 1) {
        PIR1bits.RCIF = 0; 
        
        receiveData(receiveBuffer, &crflag, &receiveFlag, &saver);
        if (receiveFlag) {
            int test = 1;
        }


    }
    
    

    //Enable Interrupts
    INTCONbits.GIE = 1;
    return;

    
}
