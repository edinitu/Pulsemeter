/* definitions / defines file */
#define DEFS_H

#define SW_VERSION		13   /* i.e. major.minor software version nbr. */

#ifndef NULL
#define NULL  0
#endif
        
// logix ...
#define TRUE	1
#define FALSE	0 
#define DUMMY	0

#define wdogtrig()			#asm("wdr") // call often if Watchdog timer enabled

#define CR				0x0D
#define LF				0x0A  


#define ADMUX_NOCHANNEL 0b01000000

// digits: bit 0 for inactive segments, 1 for active
#define ZERO 0x7E
#define ONE  0x30
#define TWO  0x6D
#define THREE 0x79
#define FOUR 0x33
#define FIVE 0x5B
#define SIX 0x5F
#define SEVEN 0x71
#define EIGHT 0x7F
#define NINE  0x7B

//displays: bit 0 for active segment, 1 for inactive
#define SEG1 0x0E  // first display
#define SEG2 0x0D  // second display
#define SEG3 0x0B  // third display

#define LED1 PORTD.6        // PORTx is used for output
#define SW1 PIND.5          // PINx is used for input
#define TESTP PORTD.4       // test bit durations
#define SEGMENTS PORTC
#define SELECTOR PORTA
#include "funct.h"

