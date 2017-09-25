/***************************************************************************
 *
 * $Revision: 2.0 $
 * $Date: Saturday, August 7, 2017 10:08:54 UTC
 * $Author: L.M.Zuccarelli
 *
 ****************************************************************************/



#include <stdio.h>
#include <stdlib.h>

int main() {
  int a = 0x80;
  int x = 0;
  int nu = 0x40;
  int bit = 0x00;
  int count = 0;
  int r,value;
  double t[8];
  int adcResults[8];
  unsigned char ioChannel = 0x00;
  int limits[2][8] = {{23,23,23,23,23,23,23,23},{19,19,19,19,19,19,19,19}};

  srand(0xb3);
  for (x = 0;x<10;x++) {
    for (count = 0; count < 8; count++) {
      r=rand();
      value = (r % 3) + 1;
      adcResults[count] = ( 2 * 256 ) + 128 + value;
      t[count] = ( (((1100.0/1023.0) * (double)adcResults[count]) - 500.0)/10.0 );
      if (t[count] > (double)limits[1][count]) {
        ioChannel |= (1<<count);
        printf("adcResults %d : t %1f : count %d : position 0x%02x true\n",adcResults[count],t[count],count,(1<<count));
      } else {
        printf("adcResults %d : t %1f : count %d : position 0x%02x false\n",adcResults[count],t[count],count,(1<<count));
      }
    }
    printf("ioChannel 0x%02x\n",ioChannel);
    char str[] = "19.03";
    double d;
    sscanf(str, "%lf", &d);
    printf("str2double : %lf\n", d);
    printf("end section %d \n\n",x);
    ioChannel = 0x00;
  }

  /*
  for (x = 0; x < 8; x++) {
    //bit = (a >> x) & nu;
    bit = (((a >> x) & nu) == (a >> x))?0x01:0x00;
  
    printf("PORTA lo 0x%02x\n", (bit << 2) | 0x00);
    printf("PORTA hi 0x%02x\n", (bit << 2) | 0x01);
    printf("PORTA lo 0x%02x\n", (bit << 2) | 0x00);
  }
  int adc = (0x02 * 256) + 0xB7;
  double z = ((1100.0/1023.0) * (double)adc);
  double t = ( z - 500.0)/10.0 ;
  printf("z      %1f\n", z);
  printf("temp      %1f\n", t);
  printf("ADCP      %d\n\n", adc);
  */

  return(0);

}
