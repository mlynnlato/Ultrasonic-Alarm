#include <msp430.h> 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


/**
 * main.c
 */

typedef enum{
    INITIALIZE,SET,BUZZ,MAX_STATES
}state_t;

typedef struct{
    bool buttonPressed;
    float ultrasonicDist;
    float armedDist;
} inputs_t;

state_t initState(inputs_t* inputs);
state_t armedState(inputs_t* inputs);
state_t alarmState(inputs_t* inputs);
state_t (*state_table[MAX_STATES])(inputs_t*)= {initState, armedState, alarmState};

void init(){
    //Buzzer in J13 (A)
    P2DIR |= BIT1; //SIG
    //P2REN ^= BIT1;
    P2OUT &= ~BIT1; //Sets to low
    //Button
    P4DIR &= ~BIT1;
    P4REN |=BIT1;
    P4OUT |=BIT1;
    //Sensor
    P6DIR |= BIT3;
    P6OUT &= ~BIT3;

    P1DIR |=BIT0;
    P6DIR |=BIT6;


}

void buzzz(bool en){
    if(en){
        P2OUT = BIT1;
    }else{
        P2OUT &= ~BIT1;
    }

}

void setGreen(bool en) {
    if(en) {
        P6OUT|=BIT6;
    }

    else {
        P6OUT &=~BIT6;
    }
}

void setRed(bool en) {
    if(en==true) {
        P1OUT |=BIT0;
    }

    else {
        P1OUT &=~BIT0;
    }
}

float getDistance(){
    float count =0;
    float dist=0;
    P6DIR &= ~BIT3;
    P6OUT &= ~BIT0;
    TB1R=0;
    P6DIR |= BIT3; //OUTPUT
    P6OUT |= BIT3;
    __delay_cycles(10);
    P6OUT &= ~BIT3;
    P6DIR &= ~BIT3; //INPUT

    while((P6IN & BIT3)==0x00); //goes high
    TB1CTL= TBSSEL__SMCLK | MC__CONTINUOUS;

    while((P6IN & BIT3));
    TB1CTL= TBSSEL__SMCLK | MC__STOP;
    count = TB1R;

    TB1CTL=TBCLR;
    dist=(count*0.000001*343)/2; //divide by 2 because it is going 2 ways (out from sensor and back to sensor)

    return (dist*39.3701);
}

bool buttonPressedGO() {
    if((P4IN & BIT1)==0x00) {
        __delay_cycles(100000); //used for de-bouncing

        if((P4IN & BIT1)==0x00) {
            while((P4IN & BIT1) == 0x00){ //conditional while loop to check if button is released
                         __delay_cycles(100000);   //delay to avoid any occurrence of bouncing
                    }
            return true;
        }
    }

    return false;
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 = 0xFFFE;
	float referenceCount=0;
	float num=0;
	init();
	referenceCount=getDistance();
	state_t currentState=INITIALIZE;
	state_t nextState= currentState;
	inputs_t inputs = {false,0.0,0.0};

	while(1){
////	    Test statement when using button
//	    if((P4IN & BIT1) == 0x00){
//	                    __delay_cycles(100000);    //used for de-bouncing
//	                    if((P4IN & BIT1) == 0x00){
//	                        while((P4IN & BIT1) == 0x00){ //conditional while loop to check if button is released
//	                                                 __delay_cycles(100000);   //delay to avoid any occurrence of bouncing
//	                                            }
//	                        referenceCount = getDistance();
//	                    }
//	    }else{
//	        num = getDistance();
//	        buzzz(false);
//	    }
//
//	    if(num>referenceCount*(1-0.1) && num < referenceCount*(1+0.1)){
//	        buzzz(true);
//	    }



	        inputs.ultrasonicDist = getDistance();
	        inputs.buttonPressed= buttonPressedGO();

	        if(currentState<MAX_STATES){
	            nextState= state_table[currentState](&inputs);
	            currentState = nextState;
	        }

	}
	return 0;
}

	state_t initState(inputs_t* inputs){
	    buzzz(false);
	    setRed(false);
	    setGreen(true);
	    if(inputs->buttonPressed){
	        inputs->armedDist = inputs->ultrasonicDist;
	        inputs->buttonPressed =false;


	        return SET;
	    }
	    return INITIALIZE;
	}

	state_t armedState(inputs_t* inputs){
	    buzzz(false);
	    setRed(false);
	    setGreen(true);
	    if(inputs->ultrasonicDist>inputs->armedDist*(1-0.2) && inputs->ultrasonicDist < inputs ->armedDist*(1+0.2)){
	        return SET;
	    }
	    return BUZZ;
	}

	state_t alarmState(inputs_t* inputs){
	    buzzz(true);
	    setRed(true);
	    setGreen(false);
	    if(inputs->buttonPressed){
	        inputs->buttonPressed=false;
	        inputs->armedDist= inputs->ultrasonicDist;
	        return SET;
	    }
	    return BUZZ;
	}


