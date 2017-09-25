/***************************************************************************
 *
 * $Revision: 1.4 $
 * $Date: Saturday, February 6, 2016 10:08:54 UTC
 * $Author: L.M.Zuccarelli
 *
 ****************************************************************************/

#include <avr/io.h>
#include <util/delay.h>

//#define F_CPU 8000000UL

//clock I/O pin.  7 PB2 <- CLK PI 22
//data output pin.6 PB1 -> DI PI 23
//data input pin. 5 PB0 <- DO PI 27

// ATTINY85
// RESET ADC0 PB5  1    8 VCC
// ADC3 PB3        2    7 PB2 ADC1 SCK CLK
// ADC2 PB4        3    6 PB1 ADC2 MISO
// GND             4    5 PB0 AREF MOSI

// PORTB  =   –   |   –   | PORTB5 | PORTB4 | PORTB3 | PORTB2 | PORTB1 | PORTB0
// DDRB   =   –   |   –   | DDB5   | DDB4   | DDB3   | DDB2   | DDB1   | DDB0
// PINB   =   –   |   –   | PINB5  | PINB4  | PINB3  | PINB2  | PINB1  | PINB0

// ADMUX  = REFS1 | REFS0 | ADLAR  | REFS2  | MUX3   | MUX2   | MUX1   | MUX0
// ADCSRA = ADEN  | ADSC  | ADATE  | ADIF   | ADIE   | ADPS2  | ADPS1  | ADPS0
// ADCSRB = BIN   | ACME  | IPR    |   –    |   –    | ADTS2  | ADTS1  | ADTS0
// DIDR0  =   –   |   –   | ADC0D  | ADC2D  | ADC3D  | ADC1D  | AIN1D  | AIN0D
// ADCL
// ADCH

//         MSB                                                 LSB
// REG     7      6      5      4      3       2       1       0
// USIDR
// USIBR
// USICR   USISIE USIOIE USIWM1 USIWM0 USICS1  USICS0  USICLK  USITC
// USISR   USISIF USIOIF USIPF  USIDC  USICNT3 USICNT2 USICNT1 USICNT0

// For 3 wire spi mode set USIWM1=0 and USIWM0=1
// External clock rising edge set USICS1=1  USICS0=0 USICLK=0
// Enable counter overflow interrupt USIOIE=1

// A simple blink test - PB3 output pin to drive led
//
int main(void) {

  DDRA =  0b11111111; // make PB3 output
  PORTA = 0b11111111; // set PB3 low - the other pins are ignored
  PUEA =  0b00000000; 
  
  // delay  for settling
  _delay_ms(100);

  while(1) {
    
    // set PB3 hi
    PORTA = 0b00000000;
    _delay_ms(1000);
    // set PB3 lo
    PORTA = 0b11111111;
    _delay_ms(1000);
  
  }

}
