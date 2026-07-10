#include "headers.h"

//--------------------------------------------------------------------
// DS18B20 Data Pin Definitions
//--------------------------------------------------------------------
#define D (1<<16)              // 1-Wire data line connected to P0.16
#define R (IOPIN0 & (1<<16))   // Reads the logic level of P0.16


/*--------------------------------------------------------------------
 * Function    : ResetDS18b20()
 * Description : Sends a reset pulse to the DS18B20 and checks whether
 *               the sensor responds with a presence pulse.
 *
 * Returns     : 1 -> Sensor detected
 *               0 -> Sensor not detected
 *-------------------------------------------------------------------*/
unsigned char ResetDS18b20(void)
{
    unsigned int presence;

    // Configure data pin as output
    IODIR0 |= D;

    // Keep bus HIGH (idle state)
    IOPIN0 |= D;
    delay_us(1);

    // Pull bus LOW to generate reset pulse
    IOPIN0 &= ~D;
    delay_us(478);

    // Release the bus
    IOPIN0 |= D;
    delay_us(54);

    // Read presence pulse from sensor
    presence = IOPIN0;

    // Wait for reset sequence to complete
    delay_us(423);

    // Check whether sensor pulled the line LOW
    if (presence & R)
        return 1;
    else
        return 0;
}


/*--------------------------------------------------------------------
 * Function    : ReadBit()
 * Description : Reads a single bit from the DS18B20 using the
 *               1-Wire communication protocol.
 *
 * Returns     : 0 or 1
 *-------------------------------------------------------------------*/
unsigned char ReadBit(void)
{
    unsigned int B;

    // Pull data line LOW to start read slot
    IOPIN0 &= ~D;
    delay_us(1);

    // Release the data line
    IOPIN0 |= D;

    // Configure pin as input
    IODIR0 &= ~D;
    delay_us(10);

    // Read the logic level
    B = IOPIN0;

    // Configure pin back as output
    IODIR0 |= D;

    if (B & R)
        return 1;
    else
        return 0;
}


/*--------------------------------------------------------------------
 * Function    : WriteBit()
 * Description : Writes a single bit to the DS18B20 using the
 *               1-Wire protocol.
 *
 * Parameter   : Dbit
 *               Bit value to be transmitted (0 or 1).
 *-------------------------------------------------------------------*/
void WriteBit(unsigned char Dbit)
{
    // Start write slot by pulling line LOW
    IOPIN0 &= ~D;
    delay_us(1);

    // Release line immediately for logic '1'
    if (Dbit)
        IOPIN0 |= D;

    // Maintain slot timing
    delay_us(58);

    // Return line to HIGH
    IOPIN0 |= D;
    delay_us(1);
}


/*--------------------------------------------------------------------
 * Function    : ReadByte()
 * Description : Reads one complete byte (8 bits) from the DS18B20.
 *
 * Returns     : Received byte.
 *-------------------------------------------------------------------*/
unsigned char ReadByte(void)
{
    unsigned char i;
    unsigned char Din = 0;

    // Read all 8 bits (LSB first)
    for (i = 0; i < 8; i++)
    {
        if (ReadBit())
            Din |= (0x01 << i);

        delay_us(45);
    }

    return Din;
}


/*--------------------------------------------------------------------
 * Function    : WriteByte()
 * Description : Sends one byte (8 bits) to the DS18B20.
 *
 * Parameter   : Dout
 *               Byte to be transmitted.
 *-------------------------------------------------------------------*/
void WriteByte(unsigned char Dout)
{
    unsigned char i;

    // Send 8 bits (LSB first)
    for (i = 0; i < 8; i++)
    {
        WriteBit(Dout & 0x01);

        // Shift next bit to LSB
        Dout >>= 1;

        delay_us(1);
    }

    // Recovery time after byte transmission
    delay_us(98);
}


/*--------------------------------------------------------------------
 * Function    : ReadTemp()
 * Description : Reads the temperature value from the DS18B20.
 *
 * Working     :
 *   1. Reset sensor.
 *   2. Skip ROM command (0xCC).
 *   3. Start temperature conversion (0x44).
 *   4. Wait until conversion completes.
 *   5. Reset sensor again.
 *   6. Skip ROM command.
 *   7. Read scratchpad (0xBE).
 *   8. Read first two bytes (temperature data).
 *   9. Combine MSB and LSB into a 16-bit temperature value.
 *
 * Returns     : Raw 16-bit temperature data.
 *-------------------------------------------------------------------*/
int ReadTemp(void)
{
    unsigned char n;
    unsigned char buff[2];
    int temp;

    // Reset the sensor
    ResetDS18b20();

    // Skip ROM (single device on bus)
    WriteByte(0xCC);

    // Start temperature conversion
    WriteByte(0x44);

    // Wait until conversion is complete
    while (ReadByte() == 0xFF);

    // Reset the sensor again
    ResetDS18b20();

    // Skip ROM
    WriteByte(0xCC);

    // Read scratchpad command
    WriteByte(0xBE);

    // Read first two bytes (temperature LSB and MSB)
    for (n = 0; n < 2; n++)
    {
        buff[n] = ReadByte();
    }

    // Combine MSB and LSB into 16-bit value
    temp = buff[1];
    temp <<= 8;
    temp |= buff[0];

    return temp;
}
