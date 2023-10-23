/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project. It is intended to
    be used as the starting point for CISC-211 Curiosity Nano Board
    programming projects. After initializing the hardware, it will
    go into a 0.5s loop that calls an assembly function specified in a separate
    .s file. It will print the iteration number and the result of the assembly 
    function call to the serial port.
    As an added bonus, it will toggle the LED on each iteration
    to provide feedback that the code is actually running.
  
    NOTE: PC serial port should be set to 115200 rate.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <float.h>
#include <malloc.h>
#include "definitions.h"                // SYS function prototypes

/* RTC Time period match values for input clock of 1 KHz */
#define PERIOD_100MS                            102
#define PERIOD_500MS                            512
#define PERIOD_1S                               1024
#define PERIOD_2S                               2048
#define PERIOD_4S                               4096

#define MAX_PRINT_LEN 400

static volatile bool isRTCExpired = false;
static volatile bool changeTempSamplingRate = false;
static volatile bool isUSARTTxComplete = true;
static uint8_t uartTxBuffer[MAX_PRINT_LEN] = {0};
static uint8_t decryptBuffer[MAX_PRINT_LEN] = {0};
static char * pass = "pass";
static char * fail = "fail";

// VB COMMENT:
// The ARM calling convention permits the use of up to 4 registers, r0-r3
// to pass data into a function. Only one value can be returned from a C
// function call. The assembly language routine stores the return value
// in r0. The C compiler will automatically use it as the function's return
// value.
//
// Function signature
// For this lab, return the larger of the two floating point values passed in.
extern char * asmEncrypt(char *, uint32_t);



static char * inpTextArray[] = { "ABCXYZ",
                                 "abcxyz",
                                 "\"Whoa, really?\", he said."
                                };
static uint32_t keyArray[] = {0,1,13,25};

#define USING_HW 1

#if USING_HW
static void rtcEventHandler (RTC_TIMER32_INT_MASK intCause, uintptr_t context)
{
    if (intCause & RTC_MODE0_INTENSET_CMP0_Msk)
    {            
        isRTCExpired    = true;
    }
}
static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUSARTTxComplete = true;
    }
}
#endif


// return failure count. A return value of 0 means everything passed.
static int testResult(int testNum, 
                      char * origText, 
                      char * cipherText, 
                      uint32_t key) 
{
    int failCount = 0;
    char *s1 = pass;
    char *s2 = pass;
    int origLen = strlen(origText);
    int encryptedLen = strlen(cipherText);
    if(origLen != encryptedLen)
    {
        // since we can't compare unequal length strings,
        // we won't even try to decrypt. This counts as two failures
        failCount = 2;
        s1 = fail;
        s2 = fail;
    }
    // char * decryptedText = (char *) malloc(encryptedLen)+1;
    unsigned char * dPtr = decryptBuffer;
    char * encCharPtr = cipherText;
    // don't try to decrypt if lengths aren't equal
    if(failCount == 0)
    {
        char inpChar;
        // should really check key to make sure it's in range...
        // for(int i = 0; i < origLen; ++i)
        while ( (inpChar = *encCharPtr++) )
        {
            if ((inpChar >= 'a') && (inpChar<='z'))
            {
                inpChar -= key;
                if (inpChar < 'a' )
                {
                    inpChar += 26; // move it back into range
                }
            }
            else if ((inpChar >= 'A') && (inpChar<='Z'))
            {
                inpChar -= key;
                if (inpChar < 'A' )
                {
                    inpChar += 26; // move it back into range
                }
            }
            // if it's any other character, just copy the value over
            *dPtr++ = inpChar;
        }
        *dPtr = 0; // add trailing null
        // Check to see that encrypted strings matched.
        if(strcmp((const char *) decryptBuffer,(const char *)origText) != 0)
        {
            s2 = fail;
            ++failCount;
        }
    }
    
       
    snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
            "========= Test Number: %d\r\n"
            "key: %ld\r\n"
            "input     text: %s\r\n"
            "encrypted text: %s\r\n"
            "decrypted text: %s\r\n"
            "test results: length: %s; decrypted string: %s\r\n"
            "\r\n",
            testNum,key,
            origText,cipherText,decryptBuffer, s1, s2); 

#if USING_HW 
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartTxBuffer, \
        (const void *)&(SERCOM5_REGS->USART_INT.SERCOM_DATA), \
        strlen((const char*)uartTxBuffer));
#endif
    // free (decryptedText);
    return failCount;
    
}


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    
 
#if USING_HW
    /* Initialize all modules */
#if 0
    SYS_Initialize ( NULL );
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
    RTC_Timer32CallbackRegister(rtcEventHandler, 0);
    RTC_Timer32Start();
#endif
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
    RTC_Timer32CallbackRegister(rtcEventHandler, 0);
    RTC_Timer32Compare0Set(PERIOD_100MS);
    RTC_Timer32CounterSet(0);
    RTC_Timer32Start();
#else // using the simulator
    isRTCExpired = true;
    isUSARTTxComplete = true;
#endif //SIMULATOR

    uint32_t numKeys = sizeof(keyArray)/sizeof(keyArray[0]);
    uint32_t numStrings = sizeof(inpTextArray)/sizeof(inpTextArray[0]);
    // uint32_t numTests = numStrings*numKeys;
    uint32_t testCount = 0;
    int32_t totalFailCount = 0;
    
    while ( true )
    {
        // select a key
        for(int keyIndex = 0; keyIndex<numKeys; keyIndex++)
        {
            // apply the selected key to each string
            for(int numString = 0; numString<numStrings; numString++)
            {
                // Toggle the LED to sho we're running a new testcase
                LED0_Toggle();
                
                // reset the state variables
                isRTCExpired = false;
                isUSARTTxComplete = false;
                
                // extract the key from the key array
                int k = keyArray[keyIndex];
                
                // create a variable to store the returned pointer
                char * encryptedText;
                
                // select the input text
                char * inpText = inpTextArray[numString];
                
                // encrypt it
                encryptedText = asmEncrypt(inpText,k);
                
                // check the result
                int numFails = 0;
                numFails = testResult(testCount, inpText, encryptedText,k);
                totalFailCount += numFails;
                ++testCount;
#if USING_HW
                // spin here until the UART has completed transmission
                // and the timer has expired
                //while  (false == isUSARTTxComplete ); 
                while ((isRTCExpired == false) ||
                       (isUSARTTxComplete == false));
#endif
            } // for each string...
        } // for each key
        
        break; // end program
    } // while ...
    int32_t idleCount = 0;

    /* slow down the timer to slow down text from scrolling off the screen */
    RTC_Timer32Compare0Set(PERIOD_4S);
    RTC_Timer32CounterSet(0);

    while(1)
    {
        // Toggle the LED to show proof of life
        LED0_Toggle();
        
        // reset the state variables
        isRTCExpired = false;
        isUSARTTxComplete = false;
        snprintf((char*)uartTxBuffer, MAX_PRINT_LEN,
                "========= ALL TESTS COMPLETE!\r\n"
                "Post-test idle Count: %ld; "
                "Total Passing Tests: %ld/%ld\r\n"
                "\r\n",
                idleCount, (testCount-totalFailCount),testCount); 

#if USING_HW 
        DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartTxBuffer, \
            (const void *)&(SERCOM5_REGS->USART_INT.SERCOM_DATA), \
            strlen((const char*)uartTxBuffer));
        // spin here until the UART has completed transmission
        // and the timer has expired
        //while  (false == isUSARTTxComplete ); 
        ++idleCount;
        while ((isRTCExpired == false) ||
               (isUSARTTxComplete == false));

        // STUDENTS: UNCOMMENT THE NEXT LINE OF CODE IF YOU WANT TO
        // STOP EXECUTION AFTER ALL TEST CASES ARE COMPLETE
        // return 0;

#endif
    }
                
            
    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}
/*******************************************************************************
 End of File
*/

