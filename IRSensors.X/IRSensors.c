#include <stdio.h>
#include <stdlib.h>
#include <p18f4520.h>
//#include "IRSensors.h"
#include <math.h>
//#include "IRSensors.c"

//#include "ConfigRegs_18F4520.h"
//#include "IRSensor.c"
//#include "ConfigRegs_18F4520.h"



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


