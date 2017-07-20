#include <msp430.h>

#include "temp.h"

#define ADC_SCALE 4096

float temp_sample() {
  ADC12CTL0 &= ~ADC12ENC;           // Disable conversions

  ADC12CTL3 |= ADC12TCMAP;
  ADC12CTL1 = ADC12SHP;
  ADC12CTL2 = ADC12RES_2;
  ADC12MCTL0 = ADC12VRSEL_1 | ADC12EOS | LIBTEMP_ADC_CHAN;
  ADC12CTL0 |= ADC12SHT02 | ADC12SHT01 | ADC12ON; // 128 cycles sample-and-hold

  while( REFCTL0 & REFGENBUSY );

  //REFCTL0 = REFVSEL_0 | REFON; // 1.2V reference (<= 70 degC)
  REFCTL0 = REF(LIBTEMP_REF_BITS) | REFON;

  // Wait for REF to settle
  // TODO: to use msp_sleep, we need to take the (clock+divider) as arg
  __delay_cycles(PERIOD_LIBMSP_REF_SETTLE_TIME);
  //msp_sleep(PERIOD_LIBMSP_REF_SETTLE_TIME);

  ADC12CTL0 |= ADC12ENC;                  // Enable conversions
  ADC12CTL0 |= ADC12SC;                   // Start conversion
  ADC12CTL0 &= ~ADC12SC;                  // We only need to toggle to start conversion
  while (ADC12CTL1 & ADC12BUSY);

  int sample = ADC12MEM0;

  ADC12CTL0 &= ~ADC12ENC;           // Disable conversions
  ADC12CTL0 &= ~(ADC12ON);  // Shutdown ADC12
  REFCTL0 &= ~REFON;

  float tempV = ((float)(sample) * LIBTEMP_REF / ADC_SCALE + 0.5 * LIBTEMP_REF / ADC_SCALE);
  float tempC = (tempV - LIBTEMP_INTERCEPT)  * LIBTEMP_SLOPE_INVERSE;

  return tempC;
}
