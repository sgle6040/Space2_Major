#include "sw_uart.h"
#include "p18f4520.h"
#include <stdio.h>
#include <stdlib.h>
#include "delays.h"
#include "Communications.h"
#define CR 0x0D
#define FULL 0xFF
#define chirpSound 'z'
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT4 0x10
#define startCh 'K'
#define safety '^' // 5E
#define shake '~' // 7F
#define endChar 'O'
#define endBuf 0xFF
#define valMask 0x0F
#define lowMask 0xF0

void DelayTXBitUART(void);
void DelayRXHalfBitUART(void);
void DelayRXBitUART(void);
void sendMsg(char*dataPack);

/**
 * @brief Initiate the UART communications on the Commander 
 * @usage allows the 2 RF modules to communicate to each other 
 * robot commander uses software serial
 */
void commSetup(void){
    OpenUART(); // set up UART 
    // TX - pin C4
    // RX - pin B1     
    INTCON2bits.RBPU = 1; //PORTB internal pull-ups disable
    PORTBbits.RB2 = 1; //set PORTB pin 2
    TRISBbits.TRISB2 = 1; //set PORTB 2 to input
    ADCON1=0x0F; //set all PORTB as digital I/O 
    
    PORTCbits.RC4 = 0; // set RC4 to 0
    TRISCbits.TRISC4 = 0; // RC4 is output

    //interrupt setup for xBee signal capture
    INTCON3bits.INT2IE = 1;	//enable PORTB2 interrupt
    INTCON3bits.INT2IP = 1;	//set PORTB2 interrupt to high priority
    INTCON3bits.INT2IF = 0;	//clear PORTB2 interrupt flag
    INTCON2bits.INTEDG2 = 0; //PORTB2 interrupt on falling edge
    //Delay10KTCYx(1000);
}

/**
 * @brief interrupt routine for receive 
 * @usage directly saves received data into a circular buffer due to the time constraints of SW serial
 * @param buffer to save the values received 
 * @param the flag that indicates the instruction 
 */
char read, *rcPtr, saveF = 0;
void receiveComms(char* receiveBuffer,char *flag) {
    read = ReadUART(); // immediately read
    if(read == startCh){
        rcPtr = receiveBuffer;
        saveF = 1; // turn on read 
    }else if(read == endChar){
        *flag = 1;
        saveF = 0; // turn off read 
    }else if(read != chirpSound && saveF == 1){ // filter out chirp 
        *rcPtr = read;
        rcPtr++;
    }
    return;
}
/**
 * @brief send the data package from commander to robot 
 * @param struct containing magnitude and direction
 */
void transmitComms(struct communicationsOutput CommsOutput) {
    char instDir = CommsOutput.instDir;
    int instMag = CommsOutput.instMag;
    //int instMag,char instDir
    
    char magPack[] = {0x10,0x10,0x10,0x10,0x00};
    char dirPack = 0x20;
    char i = 0, j = 12, k = 0;
    int store, magstore;
    magstore = instMag;
    while(i<4){
        store = magstore >> j;
        magPack[i] = magPack[i]|store;
        magstore = magstore - (store<<j);
        j = j - 4;
        i++;
    }
    magPack[i] = magPack[i]|store;
    WriteUART(startCh); // send the package initiator
    Delay10TCYx(100); // short delay to ensure the software can keep up
    WriteUART(safety); // start the message 
    Delay10TCYx(100);
    while(k<4){
        WriteUART(magPack[k]);
        k++;
        Delay10TCYx(100);
    }
    Delay10TCYx(100);
    WriteUART(instDir); // sends the instruction
    Delay10TCYx(100);
    WriteUART(endChar); // send the package that indicates the end 
    Delay10TCYx(100);
    
    return;
}
/**
 * @brief send every 5 s to ensure that there is still sound connection
 */
void handShake(void){
    WriteUART(startCh); // send the package initiator
    Delay10TCYx(100);
    WriteUART(safety); // start message 
    Delay10TCYx(100);
    WriteUART(shake); // hand shake indication 
    Delay10TCYx(100);
    WriteUART(endChar); // send the package that indicates the end 
    return;
}

/**
 * @brief when data is received, takes and stores it in respective places
 * @param buffer of stored values
 * @param IR values
 * @param instruction done flag
 * @param chirp strength
 * @param indication that connection is sound
 */
void processReceived(char* Buffer,char* IRVals, char* instructionFlag, char * chirpStr, char *connection) {
    char i,j=0; // type of response
    char test;
    if(*Buffer != safety){ // failed read
        return;
    }
    Buffer++;
    i = 0;
    if(*Buffer == shake){
        *connection = 1; // if received handshake response
        return;
    }
    test = (*Buffer)&lowMask;
    if(test == 0x20){
        while(i<3){
            
            *IRVals = (*Buffer)&valMask; // get the three values
            IRVals++;
            Buffer++;
            i++;
        }
    }
    *chirpStr = NULL;
    i = 0;
    test = (*Buffer)&lowMask;
    if(test == 0x30 || test == 0x40){
        while(i<2){
            *chirpStr = (*chirpStr) << 4;
            if (test == 0x30 ){
                *chirpStr = (*chirpStr) | ((*Buffer)- 0x30); 
            }
            if (test == 0x40 ){
                *chirpStr = (*chirpStr) | ((*Buffer)- 0x37); 
            }
            Buffer++;
            test = (*Buffer)&lowMask;
            i++;
        }

    }else{
        return;
    }

    *instructionFlag = (*Buffer)&valMask; 
    return;
    
}


/**
 * @brief the delay functions for UART, written for minimal board only
 * @usage written for serial read, stated number of Tosc
 */
void DelayTXBitUART(void){ //249 cycles
    Delay100TCYx(2);
    Delay10TCYx(4);
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();

}

void DelayRXHalfBitUART(void){ //121 cycles
    Delay100TCYx(1);
    Delay10TCYx(2);
    Nop();

}
void DelayRXBitUART(void){//247 cycles
    Delay100TCYx(2);
    Delay10TCYx(4);
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
  
}
