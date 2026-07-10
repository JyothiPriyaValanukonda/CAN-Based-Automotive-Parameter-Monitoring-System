// delay.c

/*--------------------------------------------------------------------
 * Function    : delay_us()
 * Description : Generates an approximate delay in microseconds.
 *
 * Parameter   : tdly
 *               Delay time in microseconds.
 *
 * Note        : The delay is generated using a software loop.
 *               The actual delay depends on the processor clock
 *               frequency and compiler optimization.
 *-------------------------------------------------------------------*/
void delay_us(unsigned int tdly)
{
    // Multiply to compensate for loop execution time
    tdly *= 12;

    // Execute delay loop
    while (tdly--);
}

/*--------------------------------------------------------------------
 * Function    : delay_ms()
 * Description : Generates an approximate delay in milliseconds.
 *
 * Parameter   : tdly
 *               Delay time in milliseconds.
 *
 * Note        : Software delay based on processor clock frequency.
 *-------------------------------------------------------------------*/
void delay_ms(unsigned int tdly)
{
    // Convert milliseconds to loop count
    tdly *= 12000;

    // Execute delay loop
    while (tdly--);
}

/*--------------------------------------------------------------------
 * Function    : delay_s()
 * Description : Generates an approximate delay in seconds.
 *
 * Parameter   : tdly
 *               Delay time in seconds.
 *
 * Note        : Uses a software loop. Delay accuracy depends on
 *               CPU clock frequency and compiler optimization.
 *-------------------------------------------------------------------*/
void delay_s(unsigned int tdly)
{
    // Convert seconds to loop count
    tdly *= 12000000;

    // Execute delay loop
    while (tdly--);
}
