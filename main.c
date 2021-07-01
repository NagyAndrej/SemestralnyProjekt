
#include <pic18f46k22.h>

#pragma config FOSC = HSMP    
#pragma config PLLCFG = ON   
#pragma config PRICLKEN = ON    
#pragma config FCMEN = OFF     
#pragma config WDTEN = OFF      

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "lcd.h"
#include <stdbool.h>
#include <ctype.h>
#include <math.h>



#define _XTAL_FREQ 32E6
#define SETDUTY(x) CCPR1L = x
#define PI 3.14159265

#define BTN1 PORTCbits.RC0
#define BTN2 PORTAbits.RA4
#define BTN3 PORTAbits.RA3
#define BTN4 PORTAbits.RA2
    
#define LED1 LATDbits.LATD2
#define LED2 LATDbits.LATD3
#define LED3 LATCbits.LATC4
#define LED4 LATDbits.LATD4
#define LED5 LATDbits.LATD5
#define LED6 LATDbits.LATD6
 
typedef struct
{
char data[10];
int full;

} Premenne;


volatile Premenne vstup = {0,0};


void init();
void uartinit();
void ovladanie();
bool goout();
void rezimy();
void had();
void kalk();
void blik();
void ADC();
void ADCinit();
void DAC();
void pong();
char getHP(int hp);
void initPWM();
void putch(unsigned char data);

int rezim = 1;
volatile int i = 0;

void __interrupt() ISR(void){

if(RC1IF & RC1IE){
      vstup.data[i] = RCREG1;
      i++;

if(RCREG1 == '='){
      vstup.full = 1;
      i = 0;
      return;
    }
  }
}


int main(void) {

    init();
    while(1){
        ovladanie();
        __delay_ms(50);
    }
    
    return 0;                           // nikdy se neprovede
}

void init(){    
      LCD_Init();
     LED1 = 1;LED2 = 1;LED3 = 1;LED4 = 1;LED5 = 1;LED6 = 1;
      
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISCbits.TRISC4 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD6 = 0;
    
    TRISCbits.TRISC0 = 1;
    TRISAbits.TRISA4 = 1;
    TRISAbits.TRISA3 = 1;
    ANSELAbits.ANSA3 = 0;
    ANSELAbits.ANSA2 = 0;
    TRISAbits.TRISA2 = 1;  
    
    T1CONbits.TMR1CS = 0b00;
    T1CONbits.T1CKPS = 0b11;
    TMR1ON = 1;

    
}

void uartinit(){
    ANSELC = 0x00; 
    TRISD = 0x00; 
    TRISCbits.TRISC6 = 1; 
    TRISCbits.TRISC7 = 1;

    /*baudrate*/
    SPBRG1 = 51; 

    TXSTA1bits.SYNC = 0; 
    RCSTA1bits.SPEN = 1;
    TXSTA1bits.TXEN = 1; 
    RCSTA1bits.CREN = 1; 

    RC1IE = 1; 
    PEIE = 1;
    GIE = 1; 
}

void initPWM(){
        //init - PWM
    TRISDbits.RD5 = 1;     
    TRISCbits.RC2 = 1;            
    CCPTMRS0bits.C1TSEL = 0b00;    
    PR2 = 199;                   
    CCP1CONbits.P1M = 0b00;         
    CCP1CONbits.CCP1M = 0b1100;     
    CCPR1L = 0;                  
    TMR2IF = 0;                   
    TMR2ON = 1;                    
    while(!TMR2IF){};              
    PSTR1CON |= 0b11;               
    
    TRISDbits.RD5 = 0;           
    TRISCbits.RC2 = 0;             
}

void ADCinit(){
    ANSELA |= (1 << 5);             
    ANSELE = 0b1;                 
 
    ADCON2bits.ADFM = 1;         
    ADCON2bits.ADCS = 0b110;        
    ADCON2bits.ACQT = 0b110;       
    ADCON0bits.ADON = 1;        
}

void ovladanie(){
    rezimy();
    while(1){
      
    if(BTN1){
        rezim = rezim - 1;
        
        if(rezim <= 0){
            rezim = 6;
        }
        rezimy();
        __delay_ms(200);
     
        break;
    }
    else if(BTN2){
        rezim = rezim + 1;
        
        if(rezim >=7){
            rezim = 1;
        }
        rezimy();
         __delay_ms(200);
        
         break;
    }
    else if(BTN3){
       if(rezim == 1){
           had();
       }else if(rezim == 2){
           kalk();
       }else if(rezim == 3){
           blik();
       }else if(rezim == 4){
           ADC();
       }else if(rezim == 5){
           DAC();
       }else if(rezim == 6){
           pong();
       }
       
        break;
    }
    
}
}


void rezimy(){
   
    char text[17] = "";
    switch(rezim){
            case(1):
                 sprintf(text, "HAD             ");
                 break;
            case(2):
                 sprintf(text, "KALKULACKA      ");
                 break;
            case(3):
                 sprintf(text, "BLIKANIE-SIN    ");
                 break;
            case(4):
                 sprintf(text, "BARGRAPH        ");
                 break;
            case(5):
                 sprintf(text, "GENERATOR       ");
                 break;
            case(6):
                 sprintf(text, "GAME - PONG     ");
                 break;
     }
    LCD_ShowString(1, text);
    LCD_ShowString(2, "                ");
                    
                  
}

bool goout(){
    if(BTN4)
        return true;
    else
        return false;
   
}

void had(){
    int index = 1;
    int sign = 1;
    while(1){
        LED1 = 1;LED2 = 1;LED3 = 1;LED4 = 1;LED5 = 1;LED6 = 1;
        switch(index){
            
            case(1):
               LED1 = 0;LED2 = 0;
                __delay_ms(400);
                break;
            case(2):
               LED2 = 0;LED3 = 0;
               __delay_ms(400);
                break; 
            case(3):
               LED3 = 0;LED4 = 0;
               __delay_ms(400);
                break;
            case(4):
               LED4 = 0;LED5 = 0;
               __delay_ms(400);
                break;
            case(5):
               LED5 = 0;LED6 = 0;
               __delay_ms(400);
                break;
        }
        
        if(index>=5 || index <= 0){
            sign = sign * -1;
        }
        
        if(goout()){
            LED1 = 1;LED2 = 1;LED3 = 1;LED4 = 1;LED5 = 1;LED6 = 1;
            break;
        }
        
        index = index + sign;
        __delay_ms(200);
    }
    LED1 = 1;LED2 = 1;LED3 = 1;LED4 = 1;LED5 = 1;LED6 = 1;
}

 void vycisti(){
     vstup.full = 0;
for(int a=0; a <12 ; a++){
     vstup.data[a] = '\0';
   }
}

void kalk(){
    uartinit();
    printf("\nKalkulacka vie +-*/ \nTreba zadavat a+b=\n");
    
    while(1){
        if(goout()){
            init();
            break;
        }
        
        if(vstup.full){
            
            int prveCislo = 0;
            int druheCislo = 0;
            int vysledok = 0;

            char* p_vyraz = vstup.data;

            char cislo1[6] = "0";
            char cislo2[6] = "0";
            int index1 = 0;
            int index2 = 0;
            char sign;


                while(isdigit(*p_vyraz)){
                    cislo1[index1] = *(p_vyraz);
                    index1++;
                    p_vyraz++;
                }

                sign = *p_vyraz;
                 p_vyraz++;

                while(isdigit(*p_vyraz)){
                    cislo2[index2] = *(p_vyraz);
                    index2++;
                    p_vyraz++;
                }

                prveCislo = atoi(cislo1);
                druheCislo = atoi(cislo2);
              
                switch(sign) {

                    case '+':
                        vysledok = prveCislo + druheCislo;
                        break;

                    case '-':
                         vysledok = prveCislo - druheCislo;
                        break;	
                    case '*':
                        vysledok = prveCislo * druheCislo;
                        break;

                    case '/':
                         vysledok = prveCislo / druheCislo;
                        break;	    
                    }
                printf("%d \n",vysledok);
                vycisti();	
        }    
    }
    printf("\nvystup z funkcie\n");
}


void putch(unsigned char data){
while(!TX1IF);
TXREG1 = data;
}

void blik(){
    
    initPWM();
  
    // Zhasnutie lediek
    LATD2 = 1;
    LATD3 = 1;
    LATC4 = 1;
    LATD4 = 1;
    LATD5 = 1;
    LATD6 = 1;
    
    int index = 1;                 
    int x = 0;
    
    while (1){
         x = abs(sin(index*(PI/180))*255);
        SETDUTY(x);  
        index = index + 1;                 
        
        if(goout()){
            LED1 = 1;LED2 = 1;LED3 = 1;LED4 = 1;LED5 = 1;LED6 = 1;
            PSTR1CON = 0;
            break;
        }
        
        __delay_ms(20);
         
   }
}

void ADC(){
    
    ADCinit();
    
    uint16_t pot1;              
    char text2[16] = "";            
    char text1[16] = "";       
    char vynulovanie[16] = "                ";        
    
    while(1){
        
        ADCON0bits.CHS = 5;                       
        GODONE = 1;                        
        while(GODONE);                         
        pot1 = (uint16_t)((ADRESH << 8) | ADRESL);  
        
        
        uint16_t x =round(((pot1*16)/1023));
       
        char str[16] = "";
        char a = 219;
        int i = 0;
        int c = 0;
       
        
        for(i;i<=14;i++){
            if(i<=x)
                strncat(str,a,1);
        }
        
        sprintf(text2, "%s",str);
        LCD_ShowString(2, vynulovanie);
        LCD_ShowString(2, str);
        __delay_ms(40);
        
         if(goout()){
             LCD_ShowString(2, "                ");
            break;
         }
    }
}

void pong(){
    
    int hp1 = 3;        //zivoty hraca 1
    int hp2 = 3;        //zivoty hraca 2
    char palka = 219;
    char lopta = 111;
    char riadok1[16];
    char riadok2[16] = "                ";
    char a = 219;
    int sign = 1;
    
    while(1){
        
         int index = 7;
         
         if(hp1 <= 0){
               LCD_ShowString(1, "PLAYER2 WIN     ");
               LCD_ShowString(2, "                ");
               __delay_ms(2000);
               break;
         }
         if(hp2 <= 0){
               LCD_ShowString(1, "PLAYER1 WIN     ");
               LCD_ShowString(2, "                ");
               __delay_ms(2000);
               break;
         }
         
    
         while(1){
            index = index + sign;
            riadok2[1] = palka;
            riadok2[15] = palka;
                i = 2;
                for(i;i<=14;i++){
                    riadok2[i] = 32;
                }
                riadok2[index] = lopta;
     
                
                if(index == 15 || index == 1){
                    if(index == 15 && BTN4){
                    }else if(sign > 0){
                        hp2 = hp2 - 1;
                        break;
                    }
                    
                    if(index == 1 && BTN1){
                    }else if(sign < 0){
                        hp1 = hp1 - 1;
                        break;
                    }
                }     
                 
                       
                if(index<=1 || index>=15){
                    sign = sign*(-1);
                }
                    sprintf(riadok1,"%d              %d",hp1,hp2);
                    LCD_ShowString(1, riadok1);
                    LCD_ShowString(2, riadok2);
                       __delay_ms(170);
                    
                }
            
    }
}

void DAC(){
    LCD_ShowString(1, "Ooops           ");
    LCD_ShowString(2, "app not found   ");
    __delay_ms(2000);

}

