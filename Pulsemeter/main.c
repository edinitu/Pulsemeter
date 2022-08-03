/*********************************************
Project : Test software
**********************************************
Chip type: ATmega164A
Clock frequency: 20 MHz
Compilers:  CVAVR 2.x
*********************************************/

#include <mega164a.h>
#include <stdio.h>
#include <delay.h>  
#include <string.h> 
#include <stdlib.h>
#include "defs.h"    
 
// function that uses the ADC in single conversion mode, without interrupts
// it waits until the conversion is done then it returns the voltage
 
float read_voltage(unsigned char channel) {
    channel &= 0b00000111; // max 111
    ADMUX = ADMUX_NOCHANNEL | channel; // channel selection
    ADCSRA |= 0b01000000; // start conversion with ADSC = 1
    while (ADCSRA & 0b01000000); // wait for conversion result 
    ADCSRA |= 0b00010000; // delete flag ADIF
    return (float)(ADCW) * 0.005; // 
}

/*
  declaration of volatile variables that hold the Pulse Waveform information
*/

volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile float Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats
volatile int Pulse = 0;     // 1 when User's live heartbeat is detected. 0 when not a "live beat"
          
 
volatile int rate[10];                    // array to hold last ten IBI values 
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find IBI
volatile float P = 2.50;                      // used to find peak in pulse wave, seeded
volatile float T = 2.50;                     // used to find trough in pulse wave, seeded
volatile float thresh = 2.55;                // used to find instant moment of heart beat, seeded
volatile float amp = 0;                   // used to hold amplitude of pulse waveform, seeded
volatile int firstBeat = 1;        // used to seed rate array so we startup with reasonable BPM
volatile int secondBeat = 0;      // used to seed rate array so we startup with reasonable BPM

/*
 * Timer 1 Output Compare A interrupt is used to measure heart waveform every 2ms
 */ 
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
int N;
int i;
int runningTotal;

    #asm("cli")   //disable interrupts while this is executed
    Signal = read_voltage(4);  //read voltage on channel 4
    sampleCounter += 2;
    N = sampleCounter - lastBeatTime;
    if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T){                        // T is the trough
        T = Signal;                         // keep track of lowest point in pulse wave
        }
    }

    if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
        P = Signal;                             // P is the peak
    }                                        // keep track of highest point in pulse wave
       
    if (N > 250){                                   // avoid high frequency noise
        if ( (Signal > thresh) && (Pulse == 0) && (N > (IBI/5)*3) ){
            Pulse = 1;                               // set the Pulse flag when we think there is a pulse
            LED1 = 1;                // turn on pin 13 LED
            IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
            lastBeatTime = sampleCounter;               // keep track of time for next pulse

        if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat = 0;                  // clear secondBeat flag
            for(i=0; i<=9; i++){             // seed the running total to get a realisitic BPM at startup
                rate[i] = IBI;
            }
        }

        if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
            firstBeat = 0;                   // clear firstBeat flag
            secondBeat = 1;                   // set the second beat flag
            #asm("sei")                               // enable interrupts again
            return;                              // IBI value is unreliable so discard it
      }


      // keep a running total of the last 10 IBI values
        runningTotal = 0;                  // clear the runningTotal variable

        for(i=0; i<=8; i++){                // shift data in the rate array
            rate[i] = rate[i+1];                  // and drop the oldest IBI value
            runningTotal += rate[i];              // add up the 9 oldest IBI values
        }

        rate[9] = IBI;                          // add the latest IBI to the rate array
        runningTotal += rate[9];                // add the latest IBI to runningTotal
        runningTotal /= 10;                     // average the last 10 IBI values
        BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
    }
  }

    if (Signal < thresh && Pulse == 1){   // when the values are going down, the beat is over
        LED1 = 0;            // turn off pin 13 LED
        Pulse = 0;                         // reset the Pulse flag so we can do it again
        amp = P - T;                           // get amplitude of the pulse wave
        thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
        P = thresh;                            // reset these for next time
        T = thresh;
  }

    if (N > 2500){                           // if 2.5 seconds go by without a beat
        thresh = 2.6;                          // set thresh default
        P = 2.55;                               // set P default
        T = 2.55;                               // set T default
        lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
        firstBeat = 1;                      // set these to avoid noise
        secondBeat = 0;                    // when we get the heartbeat back
  }
  #asm("sei")  // enable interrups again
}                                  

// function that displays a digit on a 7Segment display when given a number as argument
void displayPulse(int digit) {
    if(digit == 0) SEGMENTS = ZERO;
    else if(digit == 1) SEGMENTS = ONE;
    else if(digit == 2) SEGMENTS = TWO;
    else if(digit == 3) SEGMENTS = THREE;
    else if(digit == 4) SEGMENTS = FOUR;
    else if(digit == 5) SEGMENTS = FIVE;
    else if(digit == 6) SEGMENTS = SIX;
    else if(digit == 7) SEGMENTS = SEVEN;
    else if(digit == 8) SEGMENTS = EIGHT;
    else SEGMENTS = NINE;
}   
   
/*
 * main function of program
 */
void main (void)
{          
unsigned char dig1, dig2, dig3;
int count = 0;
	Init_initController();  // this must be the first "init" action/call!
	#asm("sei")             // enable interrupts  
	LED1 = 0;           	// initial state, will be changed by timer 1
 //   voltage = read_voltage(4);
	while(TRUE)
	{
		wdogtrig();	        // call often else processor will reset
	    
        // find the digits of the BPM
        dig1 = BPM / 100;
        if(dig1 >= 1) {
            dig2 = (BPM%100)/10;
            dig3 = BPM%10;
        } else {
            dig2 = BPM/10;
            dig3 = BPM%10;
        }
        
        //display them by countinously changing the SELECTOR
        // this way the user will see a digit on all three displays
        // even though at one moment in time only one segment is selected
        if(count == 3) {count = 0;}
            if(count == 2) {
                displayPulse(dig3);
                SELECTOR = SEG3;
                count ++;
            }
            if(count == 1) {
                displayPulse(dig2);
                SELECTOR = SEG2;
                count ++;
            }
            if(count == 0) {
                displayPulse(dig1);
                SELECTOR = SEG1;
                count ++;    
            }
    } 

            
}// end main loop 


