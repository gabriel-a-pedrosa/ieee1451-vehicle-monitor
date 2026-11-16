#include <xc.inc>
    
GLOBAL _conf ;define the function to link it with the C definition

    
PSECT text0,local,class=CODE,reloc=2

    
//CONFIG FEXTOSC=0b100 ;deactivate external oscillator (to allow write to RA7)
CONFIG CSWEN=0b1 ;allow editing NDIV and NOSC for CLK config
CONFIG WDTE=OFF ;required to avoid WDT restarting micro all the time

    
_conf:
    
    ;===============
    ;CONFIGURE PORTA
    ;===============   
    BANKSEL PORTA
    CLRF PORTA    ;Set all PORTA bits to 0
    
    MOVLW 0b00000001 
    BANKSEL TRISA
    MOVWF TRISA,1 ;RA0 set to input, all others set to output
    
    MOVLW 0b00000001
    BANKSEL ANSELA
    MOVWF ANSELA,1 ;RA0 set to analog, all others to digital
    

    ;===============
    ;CONFIGURE PORTC
    ;===============   
    BANKSEL PORTC
    CLRF PORTC    ;Set all PORTC bits to 0
    
    BANKSEL TRISC
    CLRF TRISC,1    ;Set all PORTC pins as output
    BSF  TRISC,7    ;except RC7, that will be used as RX1
    
    BANKSEL ANSELC
    CLRF ANSELC,1   ;Set all PORTC pins as digital
    
    BANKSEL RC4PPS
    MOVLW 0x09	     
    MOVWF RC4PPS    ;place the EUSART1 TX in RC4 
    
    BANKSEL RX1PPS
    MOVLW 0b00010111  
    MOVWF RX1PPS    ;place the EUSART1 RX in RC7
    
    
    ;===============
    ;CONFIGURE PORTD
    ;===============
    BANKSEL PORTD
    CLRF PORTD    ;Set all PORTD bits to 0
    
    BANKSEL TRISD
    CLRF TRISD,1 ;Set all PORTD pins as output
    BSF  TRISD,3 ;except RD3, that will be used as RX2
    
    BANKSEL ANSELD
    CLRF ANSELD,1 ;Set all PORTD pins as digital
    
    BANKSEL RD2PPS
    MOVLW 0x0B  
    MOVWF RD2PPS   ;place the EUSART2 TX in RD2
    
    BANKSEL RX2PPS
    MOVLW 0b00011011
    MOVWF RX2PPS  ;place the EUSART2 RX in RD3
    
    
    ;===============
    ;CONFIGURE CLOCK
    ;===============
    BANKSEL OSCCON1
    MOVLW 0b01100000  
    MOVWF OSCCON1,1 ;Set sistem clock to HFINTOSC
    
    BANKSEL OSCFRQ
    MOVLW 0b00000110 ;Set HFINTOSC frequency to 32MHz
    MOVWF OSCFRQ,1
    
    BANKSEL OSCEN
    MOVLW 0b01000000 ;Enable HFINTOSC
    MOVWF OSCEN,1
    
    
    ;================
    ;CONFIGURE TIMER0
    ;================
    BANKSEL T0CON0
    MOVLW 0b00000000  
    MOVWF T0CON0      ;Set timer0 to 8-bit mode without no post scaler

    BANKSEL T0CON1
    MOVLW 0b01001111  ;Set timer frequency to Fosc/4 with prescaler 1:32768
    MOVWF T0CON1      ;The time for the timer to go from 0 to FF is ~1s
	
    BANKSEL TMR0L     
    CLRF TMR0L	      ;Reset timer's counter
    BANKSEL TMR0H     ;Set the fixed tmr register to FF
    SETF TMR0H	      ;When TMR0L equals TMR0H, an interrupt should happen	

    
    ;===============
    ;CONFIGURE USART1
    ;===============
    BANKSEL SP1BRGL
    MOVLW 0b00110011
    MOVWF SP1BRGL     ;Set Baud rate to 9600 
    
    BANKSEL TX1STA
    MOVLW 0b00100000  
    MOVWF TX1STA      ;Enable TX1
    
    BANKSEL RC1STA
    MOVLW 0b10010000	
    MOVWF RC1STA      ;Enable USART and RC1
    
    
    ;===============
    ;CONFIGURE USART2
    ;===============
    BANKSEL PMD4
    BCF PMD4, 7	      ;Enable EUSART2

    BANKSEL SP2BRGL
    movlw 0b00110011  ;Set baud rate to 9600
    MOVWF SP2BRGL
    
    BANKSEL TX2STA
    movlw 0b00100000   
    MOVWF TX2STA      ;Enable TX2
    
    movlw 0b10010000	
    BANKSEL RC2STA
    MOVWF RC2STA      ;Enable USART and RC2
    
    
    ;=============
    ;CONFIGURE ADC
    ;=============
    BANKSEL ADPCH
    MOVLW 0b00000000  ;set RA0 as ADC input
    MOVWF ADPCH,1
    
    BANKSEL ADREF
    MOVLW 0b00000000  ;Set Vref to vdd and vss
    MOVWF ADREF,1
    
    BANKSEL ADCLK
    MOVLW 0b111111    ;Set adc clock frequency to 250kHz
    MOVWF ADCLK,1
    
    BANKSEL ADCON0 
    CLRF ADCON0
    BSF ADCON0, 7  ;Enable adc
    
    
    ;==========
    ;INTERRUPTS
    ;==========    
    BANKSEL PIR0
    BCF PIR0, 5 ;clear timer0 interrupt flag
    
    BANKSEL PIR1
    BCF PIR1,0  ;clear ADC interrupt flag
    
    BANKSEL PIR3 
    BCF PIR3,5  ;Clear RX1 interrupt flag 
    BCF PIR3,7  ;Clear RX2 interrupt flag 
    
    
    BANKSEL PIE0
    BSF PIE0, 5 ;Enable tmr0 interrupt
    BANKSEL PIE1
    BSF PIE1, 0 ;Enable adc interrupt
    BANKSEL PIE3
    BSF PIE3, 5 ;Enable RX1 interrupt
    BSF PIE3, 7 ;Enable RX2 interrupt
    

    BANKSEL INTCON
    BSF INTCON, 7 ;Enable Global Interrupts
    BSF INTCON, 6 ;Enable Peripheral Interrupts
		  
    
    
    RETURN  


