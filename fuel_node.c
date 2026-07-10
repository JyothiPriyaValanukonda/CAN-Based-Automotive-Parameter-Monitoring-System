// fuel_node.c

#include "headers.h"
#include "can_defines.h"
#include "can.h"
#include "delay.h"

// CAN frame structure used for transmitting fuel percentage
struct CAN_Frame txFrame;

/*--------------------------------------------------------------------
 * Function    : main()
 * Description : Fuel Monitoring Node
 *
 * Working:
 *   - Reads the analog voltage from the fuel sensor using ADC.
 *   - Converts the ADC value into fuel percentage.
 *   - Sends the fuel percentage to the Main ECU through the
 *     CAN bus every 500 ms.
 *-------------------------------------------------------------------*/
int main()
{
    f32 fuel, voltage;
    f32 empty_tank, full_tank;

    // Initialize ADC for reading fuel sensor
    Init_ADC();

    // Initialize CAN controller
    Init_CAN1();

    while (1)
    {
        //------------------------------------------------------------
        // Read ADC value from Fuel Sensor (Channel 0)
        //------------------------------------------------------------
        voltage = Read_ADC(CH0);

        //------------------------------------------------------------
        // ADC values corresponding to Empty and Full fuel tank
        // These values are obtained through sensor calibration.
        //------------------------------------------------------------
        empty_tank = ((0.238 / 3.3) * 1023);
        full_tank  = ((2.364 / 3.3) * 1023);

        //------------------------------------------------------------
        // Convert ADC value into Fuel Percentage
        //
        // Formula:
        // Fuel(%) = ((Current - Empty) / (Full - Empty)) × 100
        //------------------------------------------------------------
        fuel = (((voltage - empty_tank) /
                (full_tank - empty_tank)) * 100);

        //------------------------------------------------------------
        // Prepare CAN frame
        //------------------------------------------------------------
        txFrame.ID = 1;          // CAN ID for Fuel Node
        txFrame.vbf.RTR = 0;     // Data Frame (not Remote Frame)
        txFrame.vbf.DLC = 1;     // Sending 1 byte of data
        txFrame.Data1 = (u8)fuel;// Fuel percentage

        //------------------------------------------------------------
        // Wait until CAN Transmit Buffer 1 becomes available
        //------------------------------------------------------------
        while (!(C1GSR & TCS1_BIT_READ));

        //------------------------------------------------------------
        // Transmit Fuel Percentage over CAN Bus
        //------------------------------------------------------------
        CAN1_Tx(txFrame);

        //------------------------------------------------------------
        // Update every 500 ms
        //------------------------------------------------------------
        delay_ms(500);
    }
}
