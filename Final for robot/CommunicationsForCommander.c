#include "sw_uart.h"
#include "p18f4520.h"
#include <stdio.h>
#include <stdlib.h>
#include "delays.h"
#define CR 0x0D
#define FULL 0xFF
#define chirpSound 'z'
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT4 0x10
#define startCh 'K'
#define endChar 'O'
#define endBuf 0xFF
#define valMask 0x0F
#define lowMask 0xF0

void DelayTXBitUART(void);
void DelayRXHalfBitUART(void);
void DelayRXBitUART(void);
void sendMsg(char*dataPack);
void handShake(void);
void commSetup(void);
void receiveComms(char* receiveBuffer,char *flag);
void transmitComms(int instMag,char instDir);
void processReceived(char* Buffer,char* IRVals, char* instructionFlag, char * chirpStr, char *connection);

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
    PORTBbits.RB2 = 0; //set PORTB pin 2
    TRISBbits.TRISB2 = 1; //set PORTB 2 to input
    ADCON1=0x0F; //set all PORTB as digital I/O 
    
    PORTCbits.RC4 = 0; // set RC4 to 0
    TRISCbits.TRISC4 = 0; // RC4 is output

    //interrupt setup for xBee signal capture
    INTCON3bits.INT2IE = 1;	//enable PORTB2 interrupt
    INTCON3bits.INT2IP = 1;	//set PORTB2 interrupt to high priority
    INTCON3bits.INT2IF = 0;	//clear PORTB2 interrupt flag
    INTCON2bits.INTEDG2 = 0; //PORTB2 interrupt on falling edge

}

/**
 * @brief interrupt routine for receive 
 * @usage directly saves received data into a circular buffer due to the time constraints of SW serial
 * @param buffer to save the values received 
 * @param the flag that indicates the instruction 
 */
char read, *rcPtr;
void receiveComms(char* receiveBuffer,char *flag) {
    read = ReadUART(); // immediately read
    if(read == startCh){
        rcPtr = receiveBuffer;
    }else if(read == endChar){
        *flag = 1;
    }else if(read != chirpSound){
        *rcPtr = read;
        rcPtr++;
    }
    INTCON3bits.INT2IF = 0;	//clear PORTB2 interrupt flag
    return;
}
/**
 * @brief send the data package from commander to robot 
 * @param magnitude
 * @param direction 
 * @usage byte 1: high instMag, 2: low instMag 3: dir
 */
void transmitComms(int instMag,char instDir) {
    char magPack[2];
    char magH,magL;
    magH = (instMag >> 8)&0x00FF; // high byte
    magL = instMag&0x00FF; // low byte
    WriteUART(startCh); // send the package initiator
    WriteUART(magH); // high and low byte of mag
    WriteUART(magL);
    WriteUART(instDir); // sends the instruction
    WriteUART(endChar); // send the package that indicates the end 
    return;
}
/**
 * @brief send every 5 s to ensure that there is still sound connection
 */
void handShake(void){
    WriteUART(startCh); // send the package initiator
    WriteUART('+'); // hand shake indication 
    WriteUART(endChar); // send the package that indicates the end 
    return;
}
/**
 * @brief send the data package from commander to robot 
 * @param magnitude
 * @param direction 
 * @usage byte 1: high instMag, 2: low instMag 3: dir
 */
//void transmitComms(struct communicationsOutput) {
//    char magPack[2];
//    char magH,magL;
//    magH = (communicationsOutput.instMag >> 8)&0x00FF;
//    magL = communicationsOutput.instMag&0x00FF;
//    WriteUART(startCh); // send the package initiator
//    WriteUART(magH); // high and low byte of mag
//    WriteUART(magL);
//    WriteUART(instDir); // sends the instruction
//    WriteUART(endChar); // send the package that indicates the end 
//    return;
//}
/**
 * @brief when data is received, takes and stores it
 * @param buffer of stored values
 * @param IR values
 * @param instruction done flag
 * @param chirp strength
 */
void processReceived(char* Buffer,char* IRVals, char* instructionFlag, char * chirpStr, char *connection) {
    char type,i,j=0; // type of response
    Buffer ++;
    i = 0;
    if(*Buffer == '+'){
        *connection = 1; // if received handshake response
        return;
    }
    while(i<3){
        if(*Buffer != chirpSound){ // filter out potential detected chirps
            *IRVals = *Buffer; // get the three values
            IRVals++;
            Buffer++;
        }
        i++;
    }
    *chirpStr = NULL;
    i = 0;
    if((*Buffer) != 0x30){ // check if strength is zero
        while (i<2){
            if (*Buffer != chirpSound){
                *chirpStr = *chirpStr << j;
                *chirpStr = (*chirpStr) | ((*Buffer)&valMask); 
                j = j + 4;
                i++;
            }
            Buffer++;
        }
        
    }else{
        Buffer ++;
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