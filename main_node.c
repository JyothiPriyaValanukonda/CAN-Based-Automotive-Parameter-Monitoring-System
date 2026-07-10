// main_node.c

#include "headers.h"
#include "can_defines.h"
#include "can.h"
#include "delay.h"

//--------------------------------------------------------------------
// Global CAN Frame Structures
//--------------------------------------------------------------------
struct CAN_Frame txFrame;     // CAN frame for transmitting indicator status
struct CAN_Frame rxFrame;     // CAN frame for receiving fuel data

//--------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------
volatile u8 left_flag = 0, right_flag = 0;   // Indicator control flags
u8 fuel;                                     // Fuel percentage received from Fuel Node


/*--------------------------------------------------------------------
 * Function    : main()
 * Description : Main ECU
 *
 * Working:
 *   - Reads engine temperature from DS18B20.
 *   - Receives fuel percentage from Fuel Node via CAN.
 *   - Displays temperature, fuel level and indicator status on LCD.
 *   - Sends indicator commands to Indicator Node through CAN.
 *-------------------------------------------------------------------*/
int main()
{
    int temp;
    unsigned char tp, tpd;

    //------------------------------------------------------------
    // Enable External Interrupts for Indicator Switches
    //------------------------------------------------------------
    Enable_EINT0();      // Left Indicator Switch
    Enable_EINT1();      // Right Indicator Switch

    //------------------------------------------------------------
    // Peripheral Initialization
    //------------------------------------------------------------
    Init_CAN1();         // Initialize CAN Controller
    LCD_Init();          // Initialize LCD
    FuelBar_();          // Create custom fuel bar characters

    //------------------------------------------------------------
    // Display Project Title
    //------------------------------------------------------------
    Write_CMD_LCD(0x80);
    Write_str_LCD("---Project Menu---");
    delay_ms(1000);

    //------------------------------------------------------------
    // Display Static LCD Labels
    //------------------------------------------------------------

    // Engine Temperature
    Write_CMD_LCD(0xC0);
    Write_str_LCD("Engine Temp: ");

    // Degree Symbol
    Write_CMD_LCD(0x40 + 24);
    Degree();

    Write_CMD_LCD(0xD2);
    Write_DAT_LCD(3);
    Write_CMD_LCD(0xD3);
    Write_str_LCD("C");

    // Fuel
    Write_CMD_LCD(0x94);
    Write_str_LCD("Fuel: ");

    // Indicator
    Write_CMD_LCD(0xD4);
    Write_str_LCD("INDICATOR: ");

    // Create Left and Right Arrow Symbols
    Write_CMD_LCD(0x40 + 32);
    LI();

    Write_CMD_LCD(0x40 + 40);
    RI();

    //------------------------------------------------------------
    // CAN Frame Configuration for Indicator Node
    //------------------------------------------------------------
    txFrame.ID = 2;          // Indicator Node CAN ID
    txFrame.vbf.RTR = 0;     // Data Frame
    txFrame.vbf.DLC = 1;     // One-byte data

    while (1)
    {
        //--------------------------------------------------------
        // Read Engine Temperature from DS18B20
        //--------------------------------------------------------
        if (ResetDS18b20() == 0)
        {
            // Read raw temperature
            temp = ReadTemp();

            // Extract integer part
            tp = temp >> 4;

            // Extract fractional part (0.0 or 0.5)
            tpd = (temp & 0x08) ? '5' : '0';

            // Display temperature
            Write_CMD_LCD(0xCD);
            Write_int_LCD(tp);
            Write_DAT_LCD('.');
            Write_DAT_LCD(tpd);
        }

        //--------------------------------------------------------
        // Receive Fuel Percentage from Fuel Node
        //--------------------------------------------------------
        if (C1GSR & RBS_BIT_READ)
        {
            CAN1_Rx(&rxFrame);

            // Accept only Fuel Node messages
            if (rxFrame.ID == 1)
            {
                fuel = rxFrame.Data1;

                // Clear previous percentage
                Write_CMD_LCD(0x9F);
                Write_str_LCD("        ");

                // Display Fuel Bar
                Write_CMD_LCD(0x9A);
                FuelBar(fuel);

                // Display Fuel Percentage
                Write_CMD_LCD(0x9F);
                Write_DAT_LCD('(');
                Write_int_LCD(fuel);

                Write_CMD_LCD(0xA3);
                Write_str_LCD("%)");
            }
        }

        //--------------------------------------------------------
        // LEFT Indicator Selected
        //--------------------------------------------------------
        if (left_flag == 1 && right_flag == 0)
        {
            // Send Left Indicator command
            txFrame.Data1 = 1;
            CAN1_Tx(txFrame);

            // Show Right Arrow continuously
            Write_CMD_LCD(0xDF);
            Write_DAT_LCD(5);

            // Blink Left Arrow
            Write_CMD_LCD(0xDE);
            Write_DAT_LCD(4);
            delay_ms(100);

            Write_CMD_LCD(0xDE);
            Write_DAT_LCD(' ');
            delay_ms(100);
        }

        //--------------------------------------------------------
        // RIGHT Indicator Selected
        //--------------------------------------------------------
        else if (right_flag == 1 && left_flag == 0)
        {
            // Send Right Indicator command
            txFrame.Data1 = 2;
            CAN1_Tx(txFrame);

            // Show Left Arrow continuously
            Write_CMD_LCD(0xDE);
            Write_DAT_LCD(4);

            // Blink Right Arrow
            Write_CMD_LCD(0xDF);
            Write_DAT_LCD(5);
            delay_ms(100);

            Write_CMD_LCD(0xDF);
            Write_DAT_LCD(' ');
            delay_ms(100);
        }

        //--------------------------------------------------------
        // No Indicator Active
        //--------------------------------------------------------
        else if (right_flag == 0 && left_flag == 0)
        {
            // Send OFF command to Indicator Node
            txFrame.Data1 = 3;
            CAN1_Tx(txFrame);

            // Display both arrows
            Write_CMD_LCD(0xDE);
            Write_DAT_LCD(4);

            Write_CMD_LCD(0xDF);
            Write_DAT_LCD(5);
        }
    }
}


/*--------------------------------------------------------------------
 * Function    : eint0_isr()
 * Description : External Interrupt 0 ISR
 *               Controls Left Indicator.
 *-------------------------------------------------------------------*/
void eint0_isr(void) __irq
{
    // Toggle Left Indicator
    left_flag = !left_flag;

    // Turn OFF Right Indicator
    right_flag = 0;

    // Clear interrupt flag
    SSETBIT(EXTINT, 0);

    // Clear VIC interrupt
    VICVectAddr = 0;
}


/*--------------------------------------------------------------------
 * Function    : eint1_isr()
 * Description : External Interrupt 1 ISR
 *               Controls Right Indicator.
 *-------------------------------------------------------------------*/
void eint1_isr(void) __irq
{
    // Toggle Right Indicator
    right_flag = !right_flag;

    // Turn OFF Left Indicator
    left_flag = 0;

    // Clear interrupt flag
    SSETBIT(EXTINT, 1);

    // Clear VIC interrupt
    VICVectAddr = 0;
}


/*--------------------------------------------------------------------
 * Function    : Enable_EINT0()
 * Description : Configures External Interrupt 0.
 *-------------------------------------------------------------------*/
void Enable_EINT0(void)
{
    // Configure pin as EINT0
    CFGPIN(PINSEL0, 1, FUNC4);

    // Enable EINT0 interrupt
    SSETBIT(VICIntEnable, 14);

    // Configure interrupt vector
    VICVectCntl0 = 0x20 | 14;
    VICVectAddr0 = (unsigned)eint0_isr;

    // Clear pending interrupt
    SCLRBIT(EXTINT, 0);

    // Edge-triggered interrupt
    SETBIT(EXTMODE, 0);
}


/*--------------------------------------------------------------------
 * Function    : Enable_EINT1()
 * Description : Configures External Interrupt 1.
 *-------------------------------------------------------------------*/
void Enable_EINT1(void)
{
    // Configure pin as EINT1
    CFGPIN(PINSEL0, 3, FUNC4);

    // Enable EINT1 interrupt
    SSETBIT(VICIntEnable, 15);

    // Configure interrupt vector
    VICVectCntl1 = 0x20 | 15;
    VICVectAddr1 = (unsigned)eint1_isr;

    // Clear pending interrupt
    SCLRBIT(EXTINT, 1);

    // Edge-triggered interrupt
    SETBIT(EXTMODE, 1);
}
