#include <xc.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "header.c"

#define _XTAL_FREQ 32000000   //the clock frequency defined in config() must be provided here for the delay function

#pragma config WDTE=OFF

//rx1 = rc7
//tx1 = rc4
//rx2 = rd3
//tx2 = rd2

//declaration of global variables and functions
void conf(void);
void start_adc();

void __interrupt() int_handler(void); //the interrupt handler routine must be declared as __interrupt(). When an INT happens, this function is called
void putch(char); //function used to send data via UART. The printf uses this function too. 

char get_char;
char nmea_length = 0; 

void message_handler(void);
void cmd_interpreter(void);
unsigned char message_state = 0;
unsigned int bytes_to_read = 0;
uint8_t comand[16];
uint8_t i = 0; // position in comand where the next received byte is stored


struct RECIEVED_MESSAGE RM;
unsigned char* rm_ptr = (unsigned char*)&RM;  // Pointer to the RM structure as an unsigned char array


void define_METATEDS(void);
void define_TCTEDS1(void); //TEDS for the vibration sensor
void define_TCTEDS2(void); //TEDS for the gps
void define_TCTEDS3(void); //TEDS for the buzzer


void send_value(char);
void send_METATEDS(char);
void send_channelTEDS(char, char);
void send_location(char);

void atuar(void);
void erro(void);

struct METATEDS_TEMPLATE METATED;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS1;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS2;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS3;



void main(void) {
            
    conf();  //this ASM function configures the microcontroller
   
    define_METATEDS();
    define_TCTEDS1();
    define_TCTEDS2();
    define_TCTEDS3();
    
    while(1){
        ; 
    } 
}



void putch(char byte){
    while (PIR3bits.TX1IF == 0); //wait until the previous message has been sent
    TX1REG = byte;
    
    return;
}




void send_value(char byte){ 
    
    putch(1); //success
    
    putch(0);
    putch(1); //lenght of data to be sent (always 1 for now)
    

    putch(byte);
    return;
}



void send_location(char p){
    if (nmea_length==0){
        putch(1);
        putch(0);
        putch(p);  
        nmea_length = p;
        return;
    }
    putch(p);
    nmea_length--;
    
    return;
}



void atuar(){
    uint8_t estado = comand[1];
    PORTAbits.RA4=estado;
    
    putch(1);
    putch(0);
    putch(0);
    
    return;
}

void erro(){ 
    putch(0); //send
    putch(0); //error
    putch(0); //messsage
    
    //reset variables to allow new instructions
    i = 0;   
    bytes_to_read = 0;
    message_state = 0;  
      
    return;
}


//==================================================================================
//FUNCTION: Message Handler
//ENTRY: Called when a new character is received via EUSART1.
//EXIT: Parses the received character and calls Command Interpreter.
//==================================================================================
void message_handler(){        
    if (i < bytes_to_read){ 
        comand[i] = get_char;
        i++;
        if (i == bytes_to_read){
            T0CON0bits.T0EN=0; //message fully recieved, so we can disable the time out timer.
            cmd_interpreter();
            i=0;
            bytes_to_read=0;
        }
        return;
    }
    
    if (message_state < 6 && bytes_to_read == 0){//NCAP -> STIM requests have 6 encoding bytes.
        rm_ptr[message_state++] = get_char; //We keep track of which ones we already read using message_state
    }
    if (message_state == 6 && bytes_to_read ==  0){ //this section is entered in the same call for messge_handler as the LEN_LSB
        bytes_to_read = (unsigned int)RM.LEN_LSB + (unsigned int)RM.LEN_MSB*256; //the number os bytes to follow in the same message is given by this
        message_state = 0; //ready for the next message
    }
    return;
}


//====================================================================================
//FUNCTION: Command Interpreter
//ENTRY: Called after a complete message is received
//EXIT: Executes the command specified by the message received 
//      or returns an error in case an invalid command was recieved
//====================================================================================
void cmd_interpreter(void){
    if (RM.CMD_FUNC == 1){ //Read transducer value (table31)
        if (RM.CHANNEL_LSB == 1){ //sensor 1 = vibration sensor
            if (RM.CMD_CLASS == 3 || RM.CMD_CLASS == 7){
                ADCON0bits.GO=1; //start adc
                return;
            }
        }
        if (RM.CHANNEL_LSB == 2){ //sensor 2 = GPS
            if (RM.CMD_CLASS == 3 || RM.CMD_CLASS == 7){
                while (PIR3bits.TX2IF == 0); //wait until the previous message has been sent
                char request = 1;
                TX2REG = request;
                return;
            }
        }
    }    
    
    if (RM.CMD_FUNC == 2){ //Write transducer value (table31)
        if (RM.CHANNEL_LSB == 3){
            if (RM.CMD_CLASS == 3 || RM.CMD_CLASS == 7){
                atuar();
                return;
            }
        }
    }
    
    
    if (RM.CMD_CLASS==01){ //regarding TEDS (table15)
        if (RM.CMD_FUNC == 02){ //read TEDS (table16)
            if (comand[0] == 1){ //MetaTEDS
                send_METATEDS(comand[1]);
                return;
            }
            if (comand[0] == 3){
                if (RM.CHANNEL_LSB == 1 || RM.CHANNEL_LSB == 2 || RM.CHANNEL_LSB == 3 ){
                    send_channelTEDS(RM.CHANNEL_LSB, comand[1]); //we need to pass which channel to read the TEDS from and the offset for the reading
                    return;
                }
            }
        }
    }
    erro();
    return;
 }


//==================================================================================
//FUNCTION: Send MetaTEDS
//ENTRY: Called to send MetaTEDS data based on the specified offset.
//EXIT: Transmits the MetaTEDS byte-by-byte via UART.
//==================================================================================
void send_METATEDS(char offset){
    putch(1);
    putch(0);
    putch(22);
    for (int i = 0; i < 6; i++) {
        putch(METATED.TEDSID[i]);
    }
    for (int j = 0; j < 12; j++) {
        putch(METATED.UUID[j]);
    }
    for (int k = 0; k < 4; k++) {
        putch(METATED.MAXCHAN[k]);
    }
    return;
}


//==================================================================================
//FUNCTION: Send Channel TEDS
//ENTRY: Called to send the TEDS data of a specific channel and offset.
//EXIT: Transmits the requested channel's TEDS, byte-by-byte via UART.
//==================================================================================
void send_channelTEDS(char c, char offset){
    putch(1);
    putch(0);

    int i=0;
    if (c == 1){ //piezzo
        putch(36);
        for (i = 0; i < 6; i++) putch(TCTEDS1.TEDSID[i]);       
        for (i = 0; i < 3; i++) putch(TCTEDS1.CHANNEL_TYPE[i]);        
        for (i = 0; i < 12; i++)putch(TCTEDS1.UNITS[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS1.LOW_RANGE_LIMIT[i]);        
        for (i = 0; i < 3; i++) putch(TCTEDS1.HIGH_RANGE_LIMIT[i]);        
        for (i = 0; i < 3; i++) putch(TCTEDS1.DATA_MODEL[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS1.DATA_MODEL_LENGTH[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS1.MODEL_SIG_BITS[i]);
        
    }
    if (c == 2){ //gnss
        putch(43);
        for (int i = 0; i < 6; i++) putch(TCTEDS2.TEDSID[i]);
        for (int i = 0; i < 3; i++) putch(TCTEDS2.CHANNEL_TYPE[i]);
        for (int i = 0; i < 12; i++)putch(TCTEDS2.UNITS[i]);
        for (int i = 0; i < 6; i++) putch(TCTEDS2.LOW_RANGE_LIMIT[i]);
        for (int i = 0; i < 6; i++) putch(TCTEDS2.HIGH_RANGE_LIMIT[i]);
        for (int i = 0; i < 3; i++) putch(TCTEDS2.DATA_MODEL[i]);
        for (int i = 0; i < 3; i++) putch(TCTEDS2.DATA_MODEL_LENGTH[i]);
        for (int i = 0; i < 4; i++) putch(TCTEDS2.MODEL_SIG_BITS[i]);
        
    }
    if (c == 3){ //buzzer
        putch(36);
        for (i = 0; i < 6; i++) putch(TCTEDS3.TEDSID[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS3.CHANNEL_TYPE[i]);
        for (i = 0; i < 12; i++)putch(TCTEDS3.UNITS[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS3.LOW_RANGE_LIMIT[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS3.HIGH_RANGE_LIMIT[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS3.DATA_MODEL[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS3.DATA_MODEL_LENGTH[i]);
        for (i = 0; i < 3; i++) putch(TCTEDS3.MODEL_SIG_BITS[i]);
        
    }
 
    return;
}



void define_METATEDS(){
    uint8_t array1[] = {3, 4, 0, 1, 0, 1}; //Tabela 41
    memcpy(METATED.TEDSID, array1, 6); 
    uint8_t array2[] = {4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}; 
    memcpy(METATED.UUID, array2, 12);
    uint8_t array3[] = {13, 2, 0, 3};
    memcpy(METATED.MAXCHAN, array3, 4);

    return;
}

void define_TCTEDS1(void) { //TEDS for the vibration sensor
    uint8_t array1[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS1.TEDSID, array1, 6); 
    
    uint8_t array2[] = {11, 1, 0};
    memcpy(TCTEDS1.CHANNEL_TYPE, array2, 3);
                     
    uint8_t array3[] = {12, 10, 0, 128, 128, 130, 128, 124, 128, 128, 128, 128}; 
    memcpy(TCTEDS1.UNITS, array3, 12);
    
    uint8_t array4[] = {13, 1, 0}; 
    memcpy(TCTEDS1.LOW_RANGE_LIMIT, array4, 3);
    
    uint8_t array5[] = {14, 1, 255};
    memcpy(TCTEDS1.HIGH_RANGE_LIMIT, array5, 3);
    
    uint8_t array6[] = {40, 1, 0};
    memcpy(TCTEDS1.DATA_MODEL, array6, 3);
    
    uint8_t array7[] = {41, 1, 1};
    memcpy(TCTEDS1.DATA_MODEL_LENGTH, array7, 3);
    
    uint8_t array8[] = {42, 1, 8};
    memcpy(TCTEDS1.MODEL_SIG_BITS, array8, 3);

    return;
}

void define_TCTEDS2(void) { // TEDS for the gps
    uint8_t array1[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS2.TEDSID, array1, 6); 

    uint8_t array2[] = {11, 1, 0};
    memcpy(TCTEDS2.CHANNEL_TYPE, array2, 3);
                        
    uint8_t array3[] = {12, 10, 4, 128, 128, 128, 128, 128, 128, 128, 128, 128};
    memcpy(TCTEDS2.UNITS, array3, 12);

    uint8_t array4[] = {13, 4, 0x7F, 0xFF, 0xFF, 0xFF}; //NAN
    memcpy(TCTEDS2.LOW_RANGE_LIMIT, array4, 6); 

    uint8_t array5[] = {14, 4, 0x7F, 0xFF, 0xFF, 0xFF}; //NAN
    memcpy(TCTEDS2.HIGH_RANGE_LIMIT, array5, 6); 

    uint8_t array6[] = {40, 1, 5}; 
    memcpy(TCTEDS2.DATA_MODEL, array6, 3);

    uint8_t array7[] = {41, 1, 63}; 
    memcpy(TCTEDS2.DATA_MODEL_LENGTH, array7, 3);

    uint8_t array8[] = {42, 2, 01, 0xF8};
    memcpy(TCTEDS2.MODEL_SIG_BITS, array8, 4);

    return;
}

void define_TCTEDS3(void) { // TEDS for the buzzer
    uint8_t array1[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS3.TEDSID, array1, 6); 

    uint8_t array2[] = {11, 1, 1};
    memcpy(TCTEDS3.CHANNEL_TYPE, array2, 3);
      
    uint8_t array3[] = {12, 10, 3, 128, 128, 132, 130, 122, 128, 128, 128, 128}; 
    memcpy(TCTEDS3.UNITS, array3, 12);

    uint8_t array4[] = {13, 1, 0};
    memcpy(TCTEDS3.LOW_RANGE_LIMIT, array4, 3); // LOW_RANGE_LIMIT: 0 dB (off)

    uint8_t array5[] = {14, 1, 1};
    memcpy(TCTEDS3.HIGH_RANGE_LIMIT, array5, 3); // HIGH_RANGE_LIMIT: 85 dB (on)

    uint8_t array6[] = {40, 1, 4};  // The buzzer might be considered Boolean, after all, it will have only two states (on/1 and off/0)
    memcpy(TCTEDS3.DATA_MODEL, array6, 3);

    uint8_t array7[] = {41, 1, 1}; 
    memcpy(TCTEDS3.DATA_MODEL_LENGTH, array7, 3);

    uint8_t array8[] = {42, 1, 1}; 
    memcpy(TCTEDS3.MODEL_SIG_BITS, array8, 3);

    return;

}


