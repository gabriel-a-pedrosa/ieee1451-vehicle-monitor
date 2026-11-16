#include <xc.inc>

    
GLOBAL _int_handler, _send_value, _send_location, _message_handler, _get_char, _erro    ;declare global functions and variables
  
PSECT intcode



;================================================================================================================================
;FUNCTION: Interrupt Handler
;ENTRY: Automatic when an interrupt occurs
;EXIT: Identifies the source of the interrupt, executes the corresponding handler, clears the respective interrupt flag, and 
;      returns to where the program's was interrupted
;================================================================================================================================  
_int_handler:    

    
    BANKSEL PIR1
    BTFSC PIR1, 0	   ;check if the ADC interrupt flag is set 
    CALL _adc_int_handler  ;If so, go to adc_int_handler. If not, skip.
    BCF PIR1, 0		   ;Clear interrupt flag
    
    BANKSEL PIR3
    BTFSC PIR3, 5   ;Check if the EUSART1 RX flag is set (message recieved from the NCAP) 
    CALL _gotch	    ;If so, go to _gotch2 
    BCF PIR3, 5	    ;Clear interrupt flag
    
    BANKSEL PIR3
    BTFSC PIR3, 7   ;Check if the EUSART2 RX flag is set (message recieved from the gps)
    CALL _gotch2    ;If so, go to _gotch2 
    BCF PIR3, 7	    ;Clear interrupt flag
    
    BANKSEL PIR0
    BTFSC PIR0, 5   
    CALL _timer_int_handler
    BCF PIR0, 5

    
    RETFIE
    
    
;=================================================================================================================================
;FUNCTION: ADC Interrupt Handler
;ENTRY: Called when the ADC interrupt flag is set.
;EXIT: Takes the ADC result and jumps to _send_value for processing.
;=================================================================================================================================      
_adc_int_handler: 
    BANKSEL ADRESH    
    MOVFF ADRESH, WREG
    GOTO _send_value

 
;=================================================================================================================================
;FUNCTION: Get Char - EUSART1
;ENTRY: EUSART1 RX interrupt flag is set.
;EXIT: Retrieves the received character from the EUSART1 receive register (RC1REG) and goes to _message_handler.
;=================================================================================================================================   
_gotch:
    BANKSEL TMR0L ;new byte recieved, so we can reset timer
    CLRF TMR0L 
    BANKSEL T0CON0
    BSF T0CON0, 7 ;enable command time-out timer
    
    BANKSEL RC1REG
    MOVFF RC1REG, _get_char
    MOVFF RC1REG, WREG
    GOTO _message_handler
    
    
;=================================================================================================================================
;FUNCTION: Get Char - EUSART2 
;ENTRY: EUSART2 RX interrupt flag is set.
;EXIT: Retrieves the received character from the EUSART2 receive register (RC2REG) and goes to _message_handler.
;=================================================================================================================================     
_gotch2:
    BANKSEL RC2REG
    MOVFF RC2REG, WREG
    GOTO _send_location

    
    
;=================================================================================================================================
;FUNCTION: Timer interrupt Handler
;ENTRY: Timer0 interrupt flag is set.
;EXIT: Clears Timer0 Enable bit, Clears Timer0 counter, sends error message
;=================================================================================================================================     
_timer_int_handler:
    BANKSEL T0CON0
    BCF T0CON0, 7 ;Disable Timer
    
    BANKSEL TMR0L
    CLRF TMR0L	  ;Reset clock 
    
    GOTO _erro	  ;Send error message