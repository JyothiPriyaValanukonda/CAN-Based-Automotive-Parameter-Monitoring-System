  #include "headers.h"

//--------------------------------------------------------------------
// LCD Pin Definitions
//--------------------------------------------------------------------
#define LCD_DAT 8      // LCD Data bus connected to P0.8 - P0.15
#define RS      17     // Register Select pin
#define RW      19     // Read/Write pin
#define EN      18     // Enable pin


/*--------------------------------------------------------------------
 * Function    : LCD_Init()
 * Description : Initializes the LCD in 8-bit mode.
 *
 * Working:
 *   - Configures LCD pins as output.
 *   - Performs LCD initialization sequence.
 *   - Clears display.
 *   - Sets cursor movement.
 *   - Turns ON display with cursor OFF.
 *-------------------------------------------------------------------*/
void LCD_Init(void)
{
    // Configure LCD data pins as output
    WRITEBYTE(IODIR0, LCD_DAT, 0xFF);

    // Configure LCD control pins as output
    WRITEBIT(IODIR0, RS, 1);
    WRITEBIT(IODIR0, RW, 1);
    WRITEBIT(IODIR0, EN, 1);

    // Wait for LCD power-up
    delay_ms(16);

    // LCD initialization sequence
    Write_CMD_LCD(0x30);
    delay_ms(6);

    Write_CMD_LCD(0x30);
    delay_ms(1);

    Write_CMD_LCD(0x30);
    delay_ms(1);

    // 8-bit mode, 2-line display, 5x7 font
    Write_CMD_LCD(0x38);

    // Display OFF
    Write_CMD_LCD(0x10);

    // Clear LCD
    Write_CMD_LCD(0x01);

    // Cursor auto-increment
    Write_CMD_LCD(0x06);

    // Display ON, Cursor OFF
    Write_CMD_LCD(0x0C);
}


/*--------------------------------------------------------------------
 * Function    : Write_CMD_LCD()
 * Description : Sends a command to the LCD.
 *
 * Parameter   : cmd
 *               LCD command.
 *-------------------------------------------------------------------*/
void Write_CMD_LCD(char cmd)
{
    // RS = 0 → Command Register
    WRITEBIT(IOCLR0, RS, 1);

    Write_LCD(cmd);
}


/*--------------------------------------------------------------------
 * Function    : Write_DAT_LCD()
 * Description : Sends one data byte (character) to the LCD.
 *
 * Parameter   : dat
 *               ASCII character.
 *-------------------------------------------------------------------*/
void Write_DAT_LCD(char dat)
{
    // RS = 1 → Data Register
    WRITEBIT(IOSET0, RS, 1);

    Write_LCD(dat);
}


/*--------------------------------------------------------------------
 * Function    : Write_LCD()
 * Description : Sends one byte to LCD.
 *               Used internally by command and data functions.
 *
 * Parameter   : ch
 *               Command/Data byte.
 *-------------------------------------------------------------------*/
void Write_LCD(char ch)
{
    // RW = 0 → Write operation
    WRITEBIT(IOCLR0, RW, 1);

    // Put data on LCD bus
    WRITEBYTE(IOPIN0, LCD_DAT, ch);

    // Generate Enable pulse
    WRITEBIT(IOSET0, EN, 1);
    WRITEBIT(IOCLR0, EN, 1);

    // Wait for LCD execution
    delay_ms(2);
}


/*--------------------------------------------------------------------
 * Function    : Write_str_LCD()
 * Description : Displays a null-terminated string on LCD.
 *
 * Parameter   : p
 *               Pointer to string.
 *-------------------------------------------------------------------*/
void Write_str_LCD(char *p)
{
    while (*p)
        Write_DAT_LCD(*p++);
}


/*--------------------------------------------------------------------
 * Function    : Write_int_LCD()
 * Description : Displays a signed integer on LCD.
 *
 * Parameter   : n
 *               Integer value.
 *-------------------------------------------------------------------*/
void Write_int_LCD(signed int n)
{
    char a[10], i = 0;

    // Display negative sign if required
    if (n < 0)
    {
        Write_DAT_LCD('-');
        n = -n;
    }

    // Convert integer to ASCII
    do
    {
        a[i++] = (n % 10) + '0';
        n /= 10;
    } while (n);

    // Display digits in correct order
    for (; i > 0; i--)
        Write_DAT_LCD(a[i - 1]);
}


/*--------------------------------------------------------------------
 * Function    : Write_float_LCD()
 * Description : Displays a floating-point number.
 *
 * Parameters  :
 *      f - Float value.
 *      i - Number of digits after decimal point.
 *-------------------------------------------------------------------*/
void Write_float_LCD(float f, char i)
{
    unsigned long int n = f;

    // Display integer part
    Write_int_LCD(n);

    // Display decimal point
    Write_DAT_LCD('.');

    // Display fractional part
    for (; i > 0; i--)
    {
        f *= 10;
        n = f;
        Write_DAT_LCD((n % 10) + '0');
    }
}


/*--------------------------------------------------------------------
 * Function    : Degree()
 * Description : Creates a custom degree (°) symbol in CGRAM.
 *-------------------------------------------------------------------*/
void Degree(void)
{
    u8 i;

    // Degree symbol pattern
    u8 LUT[] = {0x07,0x09,0x07,0x00,0x00,0x00,0x00,0x00};

    for (i = 0; i < 8; i++)
        Write_DAT_LCD(LUT[i]);
}


/*--------------------------------------------------------------------
 * Function    : FuelBar_()
 * Description : Creates custom characters used for the fuel bar.
 *
 * Character 0 -> Empty block
 * Character 1 -> Half block
 * Character 2 -> Full block
 *-------------------------------------------------------------------*/
void FuelBar_(void)
{
    u8 i;

    u8 empty[8] = {0x1F,0x11,0x11,0x11,0x11,0x11,0x1F,0x00};
    u8 half[8]  = {0x1F,0x11,0x11,0x11,0x1F,0x1F,0x1F,0x00};
    u8 full[8]  = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x00};

    // CGRAM starting address
    Write_CMD_LCD(0x40);

    // Store Empty block (Location 0)
    for (i = 0; i < 8; i++)
        Write_DAT_LCD(empty[i]);

    // Store Half block (Location 1)
    for (i = 0; i < 8; i++)
        Write_DAT_LCD(half[i]);

    // Store Full block (Location 2)
    for (i = 0; i < 8; i++)
        Write_DAT_LCD(full[i]);
}


/*--------------------------------------------------------------------
 * Function    : FuelBar()
 * Description : Displays graphical fuel level using five blocks.
 *
 * Parameter   : fuel
 *               Fuel percentage (0–100%).
 *-------------------------------------------------------------------*/
void FuelBar(u8 fuel)
{
    u8 blocks = fuel / 20;      // Number of full blocks
    u8 rem = fuel % 20;         // Remaining percentage
    u8 i;

    // Display full blocks
    for (i = 0; i < blocks; i++)
        Write_DAT_LCD(2);

    // Display half block if required
    if (rem >= 10)
    {
        Write_DAT_LCD(1);
        blocks++;
    }

    // Fill remaining positions with empty blocks
    for (i = blocks; i < 5; i++)
        Write_DAT_LCD(0);
}


/*--------------------------------------------------------------------
 * Function    : LI()
 * Description : Creates custom Left Indicator arrow.
 *-------------------------------------------------------------------*/
void LI(void)
{
    u8 j;

    u8 LUT1[] = {0x03,0x07,0x0F,0x1F,0x0F,0x07,0x03};

    for (j = 0; j < 8; j++)
        Write_DAT_LCD(LUT1[j]);
}


/*--------------------------------------------------------------------
 * Function    : RI()
 * Description : Creates custom Right Indicator arrow.
 *-------------------------------------------------------------------*/
void RI(void)
{
    u8 k;

    u8 LUT2[] = {0x18,0x1C,0x1E,0x1F,0x0E,0x1C,0x18};

    for (k = 0; k < 8; k++)
        Write_DAT_LCD(LUT2[k]);
}
