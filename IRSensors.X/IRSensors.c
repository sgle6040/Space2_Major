#include <stdio.h>
#include <stdlib.h>
#include <p18f4520.h>
#include "IRSensors.h"
#include <math.h>
//#include "IRSensors.c"

//#include "ConfigRegs_18F4520.h"
//#include "IRSensor.c"
//#include "ConfigRegs_18F4520.h"
#define MAXIMUM_IR 3

int readIR( void );
void changeSensor(void);
void storeBuffer(int high);
void changeSensor();

void IRThresholds(int* IRVals, int* detectArray,char numAverage);
void timerSetup();
void IRDistances(int numAverage, int* vals);


    unsigned int IRBuffer1[MAXIMUM_IR]  = {0};
    unsigned int IRBuffer2[MAXIMUM_IR]  = {0};
    unsigned int IRBuffer0[MAXIMUM_IR]  = {0};
    unsigned char IRIndex1 = 0;
    unsigned char IRIndex2 = 0;
    unsigned char IRIndex0 = 0;
    
    unsigned char counter = 0;

void IRSetup() {
    
    //Set portA to inputs
    TRISA = 0xFF;
    
    
    ADCON0 = 0b00000000;
    ADCON1 = 0b00000000;
    ADCON2 = 0b00000000;
    
    //Sensor 1
    ADCON0bits.CHS = 0b0000;
    
    //Sensor 2 
    //ADCON0bits.CHS = 0b0001;
    
    //Sensor 3
    //ADCON0bits.CHS = 0b0010;

    // Turn on AD
    ADCON0bits.ADON = 1;
    ADCON0bits.GO = 1;
    
    
}





void timerSetup() {
    
    T1CON = 0b10000001;
    T3CON = 0b00000000;
            
    CCP1CON = 0b00000010;
    PIE1bits.CCP1IE = 1;     // Enables CCP1 interrupts
    IPR1bits.CCP1IP = 1;     // Sets CCP1 interrupt to high priority
    PIR1bits.CCP1IF = 0;
    
    
    //PIE1 = 0b00000100;      

    //IPR1 = 0b00000100;     

    RCON = 0b10000000;      // Enables priority levels on interrupts 

    CCPR1L = 0x82;         // Set period to 30Hz  
    //CCPR1H = 0x35;
    
    //slower value
    CCPR1H = 0xF5;
}




void IRDetect( int numAverage,int* detectArray) {
    int IRVals[3];
    // Return the sum of 
        IRDistances(numAverage,IRVals);
     
        IRThresholds(IRVals,detectArray,numAverage);
        
}
















void IRThresholds(int* IRVals, int* detectArray,char numAverage) {
    int i;
    for (i = 0; i < 3; i++) {
            if (IRVals[i] > 150) {
                //Detection on 1
               
                IRVals[i] = 0;
                detectArray[i] = 1;
                    
            }
            else {
                detectArray[i] = 0;
            }
    }
        
}


void IRDistances(int numAverage, int* vals) {
    
    // used as generic index
    int i;
    
    //Stores buffer index being read from
    int readIndex;
    
    
    //Index of last buffer input
    int index0 = IRIndex0-1;
    int index1 = IRIndex1-1;
    int index2 = IRIndex2-1;
    
    //stores sum of buffers
    int IRSum0 = 0;
    int IRSum1 = 0;
    int IRSum2 = 0;
    
    int holder;

    
            /////////// Loops through buffer
    for (i = 0; i < numAverage; i++) {
        
        readIndex =  (MAXIMUM_IR+IRIndex0-1-i)%MAXIMUM_IR ;
        
        holder = IRBuffer0[readIndex];
        
        IRSum0 += IRBuffer0[readIndex];
        
        
    }
    vals[0]= IRSum0;
    
    
    for (i = 0; i < numAverage; i++) {
        
        readIndex =  (MAXIMUM_IR+IRIndex1-1-i)%MAXIMUM_IR ;
        
        IRSum1 += IRBuffer1[readIndex];
        
        
    }
    vals[1]= IRSum1;
    

    
    for (i = 0; i < numAverage; i++) {
        
        readIndex =  (MAXIMUM_IR+IRIndex2-1-i)%MAXIMUM_IR ;
        
        IRSum2 += IRBuffer2[readIndex];
    
        
    }
    
    vals[2]= IRSum2;
    
    //reset sums
    //IRSum0 = 0; 
    //IRSum1 = 0;
    //IRSum2 = 0;
    //return vals;
}














//From lecture slides


void sampleIR() {
    int rawVal;
    //Read value
    rawVal = readIR();
    //Store in buffer
    storeBuffer(rawVal);
    
    //Change sensor
    changeSensor();

    //PIR1bits.CCP1IF = 0; //turn off interrupt flag
}




int readIR() {
    int sensorVal;
    
    //wait until conversion complete
    while (PIR1bits.ADIF == 0) {}

    //return high byte
    return ADRESH;
    
}


void changeSensor() {
    if (counter == 0) {
 
    //Sensor 2 
        ADCON0bits.CHS = 0b0001;
        //
        counter = 1;
    }
    
    //Second Sensor
    else if (counter == 1) {
        //Sensor 3
        ADCON0bits.CHS = 0b0010;
        counter = 2;
        
    }
    
    //Third Sensor
    else if (counter == 2) {
        
            //Sensor 1
        ADCON0bits.CHS = 0b0000;
        //reset timer
        counter = 0;

    }
}

void storeBuffer(int high) {
    //check sensor number
    //store in buffer
    if (counter == 0) {
        //reset buffer index if reached max
        if (IRIndex0 == MAXIMUM_IR) {
            IRIndex0 = 0;
        }
        
        //store in buffer
        
        IRBuffer0[IRIndex0] = high;
        IRIndex0++;
        
    }
    
    else if (counter == 1) {
        if (IRIndex1 == MAXIMUM_IR) {
            IRIndex1 = 0;
        }
        
        //store in buffer
        IRBuffer1[IRIndex1] = high;
        IRIndex1++;
        
    }
    
    
    
    else if (counter == 2) {
        if (IRIndex2 == MAXIMUM_IR) {
            IRIndex2 = 0;
        }
        
        //store in buffer
        IRBuffer2[IRIndex2] = high;
        IRIndex2++;
        
    }
    
    
}



