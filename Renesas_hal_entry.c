#include "hal_data.h"
/*
 *
 */
static unsigned char status=0;
static const uint8_t ErrorMessage[5] =
{ 'E', 'r', 'r', 'o', 'r' };
static volatile uint8_t Status[7] =
{ '1', '2', '3', '4', '5', '6', '7' };
//Taken from example code provided by Renesas
#define TRANSFER_LENGTH 3
static uint8_t g_dest[1];
static const uint8_t g_src[3] =
{ 'A', 'c', 'k' };
uint32_t g_transfer_complete = 0;
uint32_t g_receive_complete = 0;
//end of example code
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER
#define ERROR 0xFF
#define START 's'
#define LED1Pin BSP_IO_PORT_09_PIN_14
#define StartPin BSP_IO_PORT_01_PIN_13
#define Input1 BSP_IO_PORT_04_PIN_09
#define Input2 BSP_IO_PORT_01_PIN_04
#define Input3 BSP_IO_PORT_01_PIN_07
#define Input4 BSP_IO_PORT_03_PIN_02
#define Input5 BSP_IO_PORT_05_PIN_00
#define ConfigPin(pin,Mode) R_BSP_PinCfg(pin,Mode);
#define ConfigInput(pin) ConfigPin(pin,BSP_IO_DIRECTION_INPUT)
#define ConfigOutput(pin) ConfigPin(pin,BSP_IO_DIRECTION_OUTPUT)
#define ReadPin(pin) (R_BSP_PinRead(pin))
#define WritePin(pin,level) R_BSP_PinWrite(pin,level)
#define HIGH 1
#define LOW 0
#define configLEDpins \
		ConfigInput(Input1);\
		ConfigInput(Input2);\
		ConfigInput(Input3);\
		ConfigInput(Input4);\
		ConfigInput(Input5);\

//define with a bit field to save memory.
typedef struct LEDS
{
    unsigned char led1 :1;
    unsigned char led2 :1;
    unsigned char led3 :1;
    unsigned char led4 :1;
    unsigned char led5 :1;
} LEDS;
//Compiler optimizes this without volatile.  We must use volatile since the values can change unknowingly.
static volatile LEDS ActiveLED =
{ 0, 0, 0, 0, 0 };
void InitIO(void);
static inline void CheckStatusLEDS(void);
static inline void AcknowledgeUser(void);
static inline void WaitForUser(void);
static void SendStateToUser(unsigned char*);
static void StartInput(void);
void InitIO(void)
{
    //initalize the IO pins we need
    R_BSP_PinAccessEnable ();
    ConfigOutput(LED1Pin);
    ConfigOutput(StartPin);
    configLEDpins
}
//LEDs are in a non normal pattern inform user of an error.
static void SendError(void)
{
    fsp_err_t errt = R_SCI_UART_Write (&g_uart9_ctrl, ErrorMessage, 5);
    while (!g_transfer_complete)
    {
        errt = R_LPM_LowPowerModeEnter (&g_lpm0_ctrl);
        assert(errt == FSP_SUCCESS);
    }
    g_transfer_complete = 0;
}
//send the current state
static void SendStateToUser(unsigned char *data)
{
    if (*data == ERROR)
    {
        SendError ();
        return;
    }
    fsp_err_t errt = R_SCI_UART_Write (&g_uart9_ctrl, data, 1);
    while (!g_transfer_complete)
    {
        errt = R_LPM_LowPowerModeEnter (&g_lpm0_ctrl);
        assert(errt == FSP_SUCCESS);
    }
    g_transfer_complete = 0;
}
static inline void CheckStatusLEDS(void)
{
    unsigned char bytetosend = 0;
    ActiveLED.led1 = ReadPin(Input1);
    ActiveLED.led2 = ReadPin(Input2);
    ActiveLED.led3 = ReadPin(Input3);
    ActiveLED.led4 = ReadPin(Input4);
    ActiveLED.led5 = ReadPin(Input5);
    //each LED is off
    if (!ActiveLED.led1 && !ActiveLED.led2 && !ActiveLED.led3 && !ActiveLED.led4 && !ActiveLED.led5)
       {
           bytetosend = Status[0];
           status=0;
       }
    //Each LED is on
    else if (ActiveLED.led1 && ActiveLED.led2 && ActiveLED.led3 && ActiveLED.led4 && ActiveLED.led5)
        {
            bytetosend = Status[6];
            status=6;
        }
    else if (ActiveLED.led1)
    {
        bytetosend = Status[1];
        status=1;
    }
    else if (ActiveLED.led2)
    {
        bytetosend = Status[2];
        status=2;
    }
    else if (ActiveLED.led3)
    {
        bytetosend = Status[3];
        status=3;
    }
    else if (ActiveLED.led4)
    {
        bytetosend = Status[4];
        status=4;
    }
    else if (ActiveLED.led5)
    {
        bytetosend = Status[5];
        status=5;
    }
    //error has occurred send error string
    else
    {
        bytetosend = ERROR;
    }
    SendStateToUser (&bytetosend);
}
//only allowed to run when all LEDs are off or on
static void StartInput(void){
    if (status==0){
    WritePin(StartPin,HIGH);
    //since the other microcontroller will not read the input until it has been debounced.
    R_BSP_SoftwareDelay(60, BSP_DELAY_UNITS_MILLISECONDS);
    WritePin(StartPin,LOW);
    }
    else if (status==6){
       WritePin(StartPin,HIGH);
       R_BSP_SoftwareDelay(60, BSP_DELAY_UNITS_MILLISECONDS);
       WritePin(StartPin,LOW);
       WritePin(StartPin,HIGH);
       R_BSP_SoftwareDelay(60, BSP_DELAY_UNITS_MILLISECONDS);
       WritePin(StartPin,LOW);
    }
}
static inline void AcknowledgeUser(void)
{
    //acknowledge that we have been awaken.
    fsp_err_t errt;
    fsp_err_t err = R_SCI_UART_Write (&g_uart9_ctrl, g_src, TRANSFER_LENGTH);
    assert(err == FSP_SUCCESS);
    while (!g_transfer_complete)
    {
        //sleep while transmitting
        errt = R_LPM_LowPowerModeEnter (&g_lpm0_ctrl);
        assert(errt == FSP_SUCCESS);
    }
    g_transfer_complete=0;
}
unsigned char charactersent=0;
//Listen on bluetooth until user sends us data;
static inline void WaitForUser(void)
{
    fsp_err_t err = R_SCI_UART_Read (&g_uart9_ctrl, g_dest, 1);
    assert(err == FSP_SUCCESS);
    //sleep
    while (!g_receive_complete){
    fsp_err_t errt = R_LPM_LowPowerModeEnter (&g_lpm0_ctrl);
    assert(errt == FSP_SUCCESS);
    }
    g_receive_complete=0;
    charactersent=g_dest[0];
}
//main
void hal_entry(void)
{
    InitIO ();
    fsp_err_t err = R_SCI_UART_Open (&g_uart9_ctrl, &g_uart9_cfg);
    assert(err == FSP_SUCCESS);
    fsp_err_t errt = R_LPM_Open (&g_lpm0_ctrl, &g_lpm0_cfg);
    assert(errt == FSP_SUCCESS);

    while (1)
    {
        WaitForUser ();
        AcknowledgeUser ();
        CheckStatusLEDS ();
        if (charactersent=='s'){
                StartInput();
            }
    }
#if BSP_TZ_SECURE_BUILD
    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif
}
void g_uart9_callback(uart_callback_args_t *p_args)
{

    if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        g_receive_complete = 1;

    }
    if (p_args->event == UART_EVENT_TX_COMPLETE)
    {
        g_transfer_complete = 1;

    }

}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open (&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);

#if BSP_CFG_SDRAM_ENABLED

        /* Setup SDRAM and initialize it. Must configure pins first. */
        R_BSP_SdramInit(true);
#endif
    }
}

#if BSP_TZ_SECURE_BUILD

FSP_CPP_HEADER
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ();

/* Trustzone Secure Projects require at least one nonsecure callable function in order to build (Remove this if it is not required to build). */
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ()
{

}
FSP_CPP_FOOTER

#endif
