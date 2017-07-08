/*
 * File:   main.c
 * Author: Ehsan
 *
 * Created on November 11, 2016, 4:58 PM
 */


#include <xc.h>
#include <pic16f1823.h>
#define _XTAL_FREQ 32000000 

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)


unsigned char F_HighGain = 0;
unsigned int ADC_Value = 0;
unsigned int Previous_ADC_Value = 0;


unsigned char N_IRRX = 0;
unsigned char C_IRRX = 0;
unsigned int IRRXT[32];


unsigned char START_CONE = 0;
unsigned char STOP_CONE = 0;
unsigned char C_CONE = 0;

unsigned int Sum_Of_Noise = 0;
unsigned int Sum_Of_Signal = 0;
unsigned char N_LED = 24;

void main(void) {
    
    //OSCCON = 0b01111011;
    OSCCON = 0b11110000;
    TRISA = 0b00001101;
    TRISC = 0b00000000;
    ANSELA = 0b00000101;
    OPTION_REG = 0b10000000;
    
    ADCON1bits.ADFM = 1;
    ADCON1bits.ADCS = 2;
    ADCON1bits.ADPREF = 0;
    ADCON0bits.ADON = 1;
    
    INTCON = 0b00000000;
    TMR0 = 56;
    
    // <editor-fold defaultstate="collapsed" desc="Config">
    F_HighGain = 0;
    
    LATCbits.LATC5 = 0;
    LATCbits.LATC4 = 1;
    LATCbits.LATC3 = 1;
    LATCbits.LATC2 = 1;
    
    LATCbits.LATC1 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA4 = 0;
    
    ADCON0bits.CHS = 0;
    
    LATAbits.LATA5 = 1;
    LATCbits.LATC0 = 1;
    
    // </editor-fold>
       
    __delay_ms(1200);
    
    // <editor-fold defaultstate="collapsed" desc="Power Tune">
    __delay_us(500);
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO);
    ADC_Value = ADRESH * 0x100 + ADRESL;
    if(ADC_Value > 512)
    {       
        LATAbits.LATA5 = 0;
        __delay_us(100);
        LATAbits.LATA5 = 1;
        __delay_us(100);
    }
    else
    {
        LATCbits.LATC3 = 1;//for test
        LATAbits.LATA5 = 0;
        __delay_us(200);
        LATAbits.LATA5 = 1;
        __delay_us(200);
        LATAbits.LATA5 = 0;
        __delay_us(200);
        LATAbits.LATA5 = 1;
        __delay_us(200);
        
        __delay_ms(200);
        ADCON0bits.GO = 1;
        while(ADCON0bits.GO);
        ADC_Value = ADRESH * 0x100 + ADRESL;
        if(ADC_Value > 512)
        {
            LATAbits.LATA5 = 0;
            __delay_us(100);
            LATAbits.LATA5 = 1;
            __delay_us(100);
        }
        else
        {
            F_HighGain = 1;
            LATCbits.LATC0 = 0;
            ADCON0bits.CHS = 2;
            LATAbits.LATA5 = 0;
            __delay_us(100);
            LATAbits.LATA5 = 1;
            __delay_us(100);
            //LATCbits.LATC3 = 0;//for test
        }
        
    }
    
    // </editor-fold>
    
    __delay_ms(1000);
    
    // <editor-fold defaultstate="collapsed" desc="Measuring noise">
    C_IRRX = 0;
    for(C_IRRX = 0; C_IRRX < N_LED; C_IRRX++)
    {
        if(C_IRRX < 2)
            START_CONE = 0;
        else
            START_CONE = C_IRRX - 2;
        
        if(C_IRRX > (N_LED - 2))
            STOP_CONE = N_LED - 1;
        else
            STOP_CONE = C_IRRX + 2;
            
        LATAbits.LATA5 = 0;
        __delay_ms(2);
        for(C_CONE = START_CONE; C_CONE <= STOP_CONE; C_CONE++)
        {
            // <editor-fold defaultstate="collapsed" desc="Noise">
            switch(C_CONE / 8)
            {
                // <editor-fold defaultstate="collapsed" desc="switch">
                case 0:                            
                    LATCbits.LATC5 = 0;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 1;
                break;
                case 1:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 0;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 1;
                break;
                case 2:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 0;
                    LATCbits.LATC2 = 1;
                break;
                case 3:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 0;
                break;
                // </editor-fold>
            }                

            switch(C_CONE % 8)
            {
                // <editor-fold defaultstate="collapsed" desc="switch">
                case 0:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 0;              
                break;
                case 1:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 1;              
                break;
                case 2:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 0;              
                break;
                case 3:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 1;              
                break;
                case 4:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 0;              
                break;
                case 5:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 1;              
                break;
                case 6:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 0;              
                break;
                case 7:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 1;              
                break;
                // </editor-fold>
            }

            __delay_us(700);
            ADCON0bits.GO = 1;
            while(ADCON0bits.GO);
            ADC_Value = ADRESH * 0x100 + ADRESL;
            Sum_Of_Noise = Sum_Of_Noise + ADC_Value;
            // </editor-fold>
        }

        LATAbits.LATA5 = 1;
        __delay_ms(2);
        for(C_CONE = START_CONE; C_CONE <= STOP_CONE; C_CONE++)
        {
            // <editor-fold defaultstate="collapsed" desc="Signal">
            switch(C_CONE / 8)
            {
                // <editor-fold defaultstate="collapsed" desc="switch">
                case 0:                            
                    LATCbits.LATC5 = 0;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 1;
                break;
                case 1:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 0;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 1;
                break;
                case 2:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 0;
                    LATCbits.LATC2 = 1;
                break;
                case 3:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 0;
                break;
                // </editor-fold>
            }                

            switch(C_CONE % 8)
            {
                // <editor-fold defaultstate="collapsed" desc="switch">
                case 0:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 0;              
                break;
                case 1:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 1;              
                break;
                case 2:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 0;              
                break;
                case 3:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 1;              
                break;
                case 4:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 0;              
                break;
                case 5:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 1;              
                break;
                case 6:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 0;              
                break;
                case 7:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 1;              
                break;
                // </editor-fold>
            }

            __delay_us(700);
            ADCON0bits.GO = 1;
            while(ADCON0bits.GO);
            ADC_Value = ADRESH * 0x100 + ADRESL;
            Sum_Of_Signal = Sum_Of_Signal + ADC_Value;
            // </editor-fold>
        }
        IRRXT[C_IRRX] = Sum_Of_Signal - Sum_Of_Noise - 30;
        Sum_Of_Signal = 0;
        Sum_Of_Noise = 0;
    }
    C_IRRX = 0;
    // </editor-fold>
    LATAbits.LATA5 = 0;
    __delay_us(100); 
    LATAbits.LATA5 = 1;
    
    //LATCbits.LATC0 = 0;
    //__delay_ms(1000);
    
    //__delay_ms(1000);
    LATCbits.LATC0 = 1;
    // <editor-fold defaultstate="collapsed" desc="MAIN">
    
//    LATCbits.LATC5 = 0;
//    LATCbits.LATC4 = 1;
//    LATCbits.LATC3 = 1;
//    LATCbits.LATC2 = 1;
//    
//    LATCbits.LATC1 = 0;
//    LATAbits.LATA1 = 0;
//    LATAbits.LATA4 = 0;
    C_IRRX = 0;
    while(1)
    {       
        Sum_Of_Signal = 0;
        if(C_IRRX < 2)
            START_CONE = 0;
        else
            START_CONE = C_IRRX - 2;
        
        if(C_IRRX > (N_LED - 2))
            STOP_CONE = N_LED - 1;
        else
            STOP_CONE = C_IRRX + 2;
        
        __delay_ms(2);
        for(C_CONE = START_CONE; C_CONE <= STOP_CONE; C_CONE++)
        {
            // <editor-fold defaultstate="collapsed" desc="Signal">
            switch(C_CONE / 8)
            {
                // <editor-fold defaultstate="collapsed" desc="switch">
                case 0:                            
                    LATCbits.LATC5 = 0;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 1;
                break;
                case 1:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 0;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 1;
                break;
                case 2:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 0;
                    LATCbits.LATC2 = 1;
                break;
                case 3:                            
                    LATCbits.LATC5 = 1;
                    LATCbits.LATC4 = 1;
                    LATCbits.LATC3 = 1;
                    LATCbits.LATC2 = 0;
                break;
                // </editor-fold>
            }                

            switch(C_CONE % 8)
            {
                // <editor-fold defaultstate="collapsed" desc="switch">
                case 0:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 0;              
                break;
                case 1:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 1;              
                break;
                case 2:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 0;              
                break;
                case 3:                            
                    LATCbits.LATC1 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 1;              
                break;
                case 4:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 0;              
                break;
                case 5:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA4 = 1;              
                break;
                case 6:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 0;              
                break;
                case 7:                            
                    LATCbits.LATC1 = 1;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA4 = 1;              
                break;
                // </editor-fold>
            }

            __delay_us(700);
            ADCON0bits.GO = 1;
            while(ADCON0bits.GO);
            ADC_Value = ADRESH * 0x100 + ADRESL;
            Sum_Of_Signal = Sum_Of_Signal + ADC_Value;
            // </editor-fold>
        }
        
        if(Sum_Of_Signal > IRRXT[C_IRRX])
        {
            C_IRRX ++;
            if(C_IRRX > (N_LED - 1))
                C_IRRX = 0;
            LATAbits.LATA5 = 0;
            __delay_us(100);
            LATAbits.LATA5 = 1;
            LATCbits.LATC0 = 1; 
        }
        else
           LATCbits.LATC0 = 0; 
        
        
    }
    // </editor-fold>
    
    return;
}



