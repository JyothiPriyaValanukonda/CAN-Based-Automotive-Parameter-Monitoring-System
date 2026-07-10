// indicator_node.c

#include "headers.h"
#include "can_defines.h"
#include "can.h"
#include "delay.h"

//--------------------------------------------------------------------
// LED Definitions
//--------------------------------------------------------------------
#define LEDS (0xFF << 0)      // LEDs connected to P0.0 - P0.7 (Active LOW)

// CAN frame used for receiving indicator commands
struct CAN_Frame rxFrame;

// Stores the current indicator command
// 0/3 -> Indicators OFF
// 1   -> Left Indicator
// 2   -> Right Indicator
u8 indicator_state = 0;

// Left Indicator  : LEDs glow from Right → Left
// Right Indicator : LEDs glow from Left → Right

/*--------------------------------------------------------------------
 * Function    : main()
 * Description : Indicator Node
 *
 * Working:
 *   - Receives indicator commands from the Main ECU through CAN.
 *   - Displays Left or Right indicator animation using 8 LEDs.
 *   - Continuously checks for new CAN messages so the animation
 *     can be stopped or changed immediately.
 *-------------------------------------------------------------------*/
int main()
{
    int i, j;

    //------------------------------------------------------------
    // Configure LED pins (P0.0 - P0.7) as output
    //------------------------------------------------------------
    IODIR0 |= LEDS;

    // Turn OFF all LEDs initially (Active LOW)
    IOSET0 = LEDS;

    //------------------------------------------------------------
    // Initialize CAN controller
    //------------------------------------------------------------
    Init_CAN1();

    while (1)
    {
        //--------------------------------------------------------
        // Check if a CAN message has been received
        //--------------------------------------------------------
        if (C1GSR & RBS_BIT_READ)
        {
            // Read received CAN frame
            CAN1_Rx(&rxFrame);

            // Accept only Indicator Node messages (ID = 2)
            if (rxFrame.ID == 2)
            {
                indicator_state = rxFrame.Data1;
            }
        }

        //--------------------------------------------------------
        // LEFT Indicator Animation
        // LEDs glow from Right → Left
        //--------------------------------------------------------
        if (indicator_state == 1)
        {
            for (i = 7; i >= 0; i--)
            {
                //------------------------------------------------
                // Check for a new command while animation runs
                //------------------------------------------------
                if (C1GSR & RBS_BIT_READ)
                {
                    CAN1_Rx(&rxFrame);

                    if (rxFrame.ID == 2)
                    {
                        indicator_state = rxFrame.Data1;
                    }
                }

                // Stop animation if command changes
                if (indicator_state != 1)
                    break;

                // Turn ON current LED (Active LOW)
                IOCLR0 = (1 << i);

                // LED ON delay
                delay_ms(120);

                // Turn OFF current LED
                IOSET0 = (1 << i);
            }
        }

        //--------------------------------------------------------
        // RIGHT Indicator Animation
        // LEDs glow from Left → Right
        //--------------------------------------------------------
        else if (indicator_state == 2)
        {
            for (j = 0; j <= 7; j++)
            {
                //------------------------------------------------
                // Check for updated CAN command
                //------------------------------------------------
                if (C1GSR & RBS_BIT_READ)
                {
                    CAN1_Rx(&rxFrame);

                    if (rxFrame.ID == 2)
                    {
                        indicator_state = rxFrame.Data1;
                    }
                }

                // Stop animation if command changes
                if (indicator_state != 2)
                    break;

                // Turn ON current LED (Active LOW)
                IOCLR0 = (1 << j);

                // LED ON delay
                delay_ms(120);

                // Turn OFF current LED
                IOSET0 = (1 << j);
            }
        }

        //--------------------------------------------------------
        // Turn OFF all LEDs when no indicator is active
        //--------------------------------------------------------
        else
        {
            IOSET0 = LEDS;
        }
    }
}
