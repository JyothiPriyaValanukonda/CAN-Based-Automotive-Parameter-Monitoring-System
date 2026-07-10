#include "headers.h"

/*--------------------------------------------------------------------
 * Function    : Init_ADC()
 * Description : Initializes the ADC (Analog-to-Digital Converter).
 *               - Powers ON the ADC.
 *               - Sets the ADC clock divider.
 *               - Selects the default ADC channel.
 *-------------------------------------------------------------------*/
void Init_ADC(void)
{
    ADCR = PDN_BIT | CLKDIV | CHANNEL_SEL;
}

/*--------------------------------------------------------------------
 * Function    : Read_ADC()
 * Description : Reads the analog value from the specified ADC channel.
 *
 * Parameter   : chNo
 *               ADC channel number (e.g., 0 to 7).
 *
 * Returns     : 10-bit ADC conversion result (0 - 1023).
 *-------------------------------------------------------------------*/
f32 Read_ADC(u8 chNo)
{
    u16 adcVal = 0;    // Variable to store the converted ADC value

    // Select the required ADC channel
    WRITEBYTE(ADCR, 0, chNo);

    // Start ADC conversion
    SETBIT(ADCR, ADC_START_BIT);

    // Small delay to allow conversion to begin
    delay_us(3);

    // Wait until ADC conversion is complete (DONE bit becomes 1)
    while (!READBIT(ADDR, DONE_BIT));

    // Stop ADC conversion
    CLRBIT(ADCR, ADC_START_BIT);

    // Extract the 10-bit conversion result from bits [15:6]
    adcVal = (ADDR >> 6) & 0x3FF;

    // Return the ADC value
    return adcVal;
}
