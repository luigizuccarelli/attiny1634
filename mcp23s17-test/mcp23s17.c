/***************************************************************************
 *
 * $Revision: 2.0 $
 * $Date: Saturday, August 7, 2017 10:08:54 UTC
 * $Author: L.M.Zuccarelli
 *
 ****************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// SPI
// clock I/O pin.   16 PC1 <- CLK PI 22
// data output pin. 20 PB1 -> DI PI 23
// data input pin.  19 PB2 <- DO PI 27

// ATTINY1634
//
// ADC5 PB0  1    20 PB1 MOSI ADC6
// ADC4 PA7  2    19 PB2 MISO ADC7
// ADC3 PA6  3    18 PB3 ADC8
// ADC2 PA5  4    17 PC0 ADC9
// ADC1 PA4  5    16 PC1 ADC10 SCK SCL
// ADC0 PA3  6    15 PC2 ADC11
//      PA2  7    14 PC3 RESET
//      PA1  8    13 PC4 XTAL2
// AREF PA0  9    12 PC5 XTAL1
//      GND  10   11 VCC

// PORTA  =   PORTA7 |   PORTA6  | PORTA5 | PORTA4 | PORTA3 | PORTA2 | PORTA1 | PORTA0
// PORTB  =   –      |   –       | –      | –      | PORTB3 | PORTB2 | PORTB1 | PORTB0
// PORTC  =   –      |   –       | PORTC5 | PORTC4 | PORTC3 | PORTC2 | PORTC1 | PORTC0
// DDRA   =   DDA7   |   DDA6    | DDA5   | DDA4   | DDA3   | DDA2   | DDA1   | DDA0
// DDRB   =   –      |   –       | –      | –      | DDB3   | DDB2   | DDB1   | DDB0
// DDRC   =   –      |   –       | DDC5   | DDC4   | DDC3   | DDC2   | DDC1   | DDC0
// PINA   =   PINA7  |   PINA6   | PINA5  | PINA4  | PINA3  | PINA2  | PINA1  | PINA0
// PINB   =   –      |   –       | –      | –      | PINB3  | PINB2  | PINB1  | PINB0
// PINC   =   –      |   –       | PINC5  | PINC4  | PINC3  | PINC2  | PINC1  | PINC0

// ADMUX  =    REFS1 |   REFS0   | REFEN  | ADC0EN | MUX3   | MUX2   | MUX1   | MUX0
// ADCSRA =    ADEN  |   ADSC    | ADATE  | ADIF   | ADIE   | ADPS2  | ADPS1  | ADPS0
// ADCSRB =    VDEN  |   VDPD    | –      | –      | ADLAR  | ADTS2  | ADTS1  | ADTS0
// DIDR0  =    ADC4D |   ADC3D   | ADC2D  | ADC1D  | ADC0D  | AIN1D  | AIN0D  | AREFD
// DIDR1  =    –     |   –       | –      | –      | ADC8D  | ADC7D  | ADC6D  | ADC5D
// DIDR2  =    –     |   –       | –      | –      | –      | ADC11D | AIN10D | ADC9D
// ADCL
// ADCH

//         MSB                                                 LSB
// REG     7      6      5      4      3       2       1       0
// USIDR
// USIBR
// USISR   USISIF USIOIF USIPF  USIDC  USICNT3 USICNT2 USICNT1 USICNT0
// USICR   USISIE USIOIE USIWM1 USIWM0 USICS1  USICS0  USICLK  USITC

// For 3 wire spi mode set USIWM1=0 and USIWM0=1
// External clock rising edge set USICS1=1  USICS0=0 USICLK=0
// Enable counter overflow interrupt USIOIE=1

// MCP23S17 SPI Slave Device
#define SPI_SLAVE_WRITE 0x40
#define SPI_SLAVE_READ  0x41
#define SPI_MSB_START   0x80
#define SPI_BIT_HI      0x04 // PA2 hi
#define SPI_BIT_LO      0x00 // PA2 lo
#define SPI_BIT_READ    0x02 // PA1

// MCP23S17 Registers Definition for BANK=0 (default)
#define IODIRA 0x00
#define IODIRB 0x01
#define IOCONA 0x0A
#define GPPUA  0x0C
#define GPPUB  0x0D
#define GPIOA  0x12
#define GPIOB  0x13


// The master spi using miso,mosi, and scl are controlled by the pi (or external device)
// The internal spi to address the mcp23s17 uses PA0-PA2
// PA0 - CLK (mcp23s17 - output for attiny)  
// PA1 - SO  (mcp23s17 - input for attiny)
// PA2 - SI  (mcp23s17 - output for attiny)

void internalSpiInit(void) {
  // Configure port directions.
  DDRA  = 0b00000101;   // PA0 and PA2 output, PA1 input
  PORTA = 0b00000000;   // All outputs lo 
  PUEA  = 0b00000010;   // PA1 pull up
  DDRC  = 0b00100000;   // PC5 output 
  PUEC  = 0b00100000;   // PC5 pull up
  PORTC = 0b00100000;   // PC5 will be CS - initialise hi 
  _delay_ms(5);
}


// functions for internal spi

// clock and data serializer utility
unsigned char serializeData(unsigned char data) {
  unsigned char bit = 0x00;
  unsigned char mask = SPI_MSB_START;
  unsigned char result = 0x00;

  while (mask != 0) {
    bit = (mask & data) == mask?SPI_BIT_HI:SPI_BIT_LO;
    // set bit and clock lo
    PORTA = 0b00000100 & bit;
    // set clock hi
    PORTA = 0b00000001 | bit;
    // set clock lo
    PORTA = 0b00000100 & bit;
    // read PA1
    if (((PINA & SPI_BIT_READ) == SPI_BIT_READ)) {
      result |= mask;
    }
    // set all lo
    PORTA = 0b00000000;
		mask = mask >> 1;
  }
  return result;
}


unsigned char internalSpiReadWrite(unsigned char addr,unsigned char data,unsigned char read) {
  unsigned char result = 0x00;
  // Activate the CS pin
  PORTC = 0b00000000;
  // set clock low
  PORTA = 0b00000000;
  // transmit the opcode
  if ((read & 0x01) == 0x01) {
    serializeData(SPI_SLAVE_READ);
  } else {
    serializeData(SPI_SLAVE_WRITE);
  }
  // transmit address
  serializeData(addr);
  // read or write
  result = serializeData(data);
  // CS pin hi
  PORTC = 0b00100000; 
  _delay_us(150);
  return result;
}

// functions for the master spi

void spiInitSlave(void) {
  // Configure port directions.
  DDRB =  0b00000010; // PB1 output
  PORTB = 0b00000101; // PB0 and PB2 pull up

  // Configure USI to 3-wire slave mode with no overflow interrupt.
  USICR = 0b00011000;
  USISR = 0b00000000;
}

unsigned char spiGet(void) {
  USICR = 0b00011000;
  USISR = 0b01000000;
  while ( (USISR & 0b01000000) != 0b01000000 ){}
  //in = USIDR;
  USISR = 0b00000000;
  USICR = 0b00010000;
  return USIDR;
}

void spiPut(unsigned char byte) {
  USIDR = byte;
  USICR = 0b00011000;
  USISR = 0b01000000;
  while ( (USISR & 0b01000000) != 0b01000000 ){}
  USISR = 0b00000000;
  USICR = 0b00010000;
}

// adc init
void initADC(void) {
  PRR    = 0b00000000;
  ADMUX  = 0b10000000; // bits 7 set to 1 Internal 1.1v ref
  ADCSRA = 0b10000110; // ADC turned on; conversion not started; auto trigger off; interrupt flag; interrupt disabled; Prescaler divide by 64
  ADCSRB = 0b00000000; // Free running
}


int main(void) {

  unsigned char inData = 0x00;
  unsigned char ioChannel = 0x00;
  unsigned char channels[8] = {0b10000000,0b10000001,0b10000010,0b10000011,0b10000100,0b10000101,0b10001000,0b10001001};
  unsigned char inSpi = 0x00;
  unsigned char lo;
  unsigned char hi;
  int adcResults[8];
  int limits[2][8] = {{23,23,23,23,23,23,23,23},{24,24,24,24,24,24,24,24}};
  int count = 0;
  int nLoop = 0;
  double t[8];

  initADC();
  internalSpiInit();
  //spiInitSlave();
  
  // Initial the MCP23S17 SPI I/O Expander
  internalSpiReadWrite(IOCONA,0x00,0x00);   // I/O Control Register: BANK=0, SEQOP=0, HAEN=0 (Disable Addressing)
  internalSpiReadWrite(IODIRA,0x00,0x00);   // GPIOA As Output
  internalSpiReadWrite(IODIRB,0xFF,0x00);   // GPIOB As Input
  internalSpiReadWrite(GPPUB,0xFF,0x00);    // Enable Pull-up Resistor on GPIOB
  internalSpiReadWrite(GPIOA,0x00,0x00);    // Reset Output on GPIOA

  _delay_ms(20);

  while(1) {

    // first check if a request for limits - GPBA0 lo
    inData = internalSpiReadWrite(GPIOB,0x00,0x01);
    if ((0xFE & inData) == inData) {

      // wait for input from master
      inSpi = spiGet();
      // valid address
      if ((inSpi & 0b11111111) == 0b00000011) {
        // send request for limits
        spiPut(0x01);
        _delay_us(5);
        for (count = 0; count < 8; count++) {
          limits[0][count] = spiGet();
          _delay_us(5);
          limits[1][count] = spiGet();
          _delay_us(5);
        }
      }
    }
      
    // start adc for 8 channels
    for (count = 0; count < 8; count++) {
      // 1.1V as ref + channel[x] lookup
      ADMUX  = channels[count];      
      _delay_us(200);
      // ensure adc has stabilised
      for (nLoop = 0; nLoop < 5 ; nLoop++) {
        ADCSRA |= (1<<ADSC);
        while (!(ADCSRA & (1<<ADSC))){}
        lo = ADCL;
        hi = ADCH;
      }
      // perform temp conversion
      // T coefficient is 10mv/C
      // Tout at 0 Deg is 500mv
      adcResults[count] = ( hi * 256 ) + lo;
      t[count] = ( (((1100.0/1023.0) * (double)adcResults[count]) - 500.0)/10.0 );
      //if (t[count] > (double)limits[1][count]) {
      if (t[count] > 23.0) {
        ioChannel |= (1<<count);
      }
    }
    internalSpiReadWrite(GPIOA,ioChannel,0x00);
    _delay_ms(20);
    ioChannel = 0x00;
    // wait for master
    //inSpi = spiGet();
    // valid address
    //if ((inSpi & 0b11111111) == 0b00000011) {
      // send lo and hi byte to master
      //    spiPut(lo);
      // /   _delay_us(10);
      //    spiPut(hi);
    //}
  }

  return(0);
}
