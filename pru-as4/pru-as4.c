#include <stdint.h>
#include <stdbool.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "sharedDataStruct.h"

#define STR_LEN         8       // # LEDs in our string
#define oneCyclesOn     700/5   // Stay on 700ns
#define oneCyclesOff    800/5
#define zeroCyclesOn    350/5
#define zeroCyclesOff   600/5
#define resetCycles     60000/5 // Must be at least 50u, use 60u

// Reference for shared RAM:
// https://markayoder.github.io/PRUCookbook/05blocks/blocks.html#_controlling_the_pwm_frequency


// P8_11 for output (on R30), PRU0
#define DATA_PIN 15       // Bit number to output on

// GPIO Configuration
// ----------------------------------------------------------
volatile register uint32_t __R30; // output GPIO register
volatile register uint32_t __R31; // input GPIO register

// GPIO Output: P8_12 = pru0_pru_r30_14
//      = LEDDP2 (Turn on/off right 14-seg digit) on Zen cape
#define DIGIT_ON_OFF_MASK (1 << 14)
// GPIO Input: P8_15 = pru0_pru_r31_15
//      = JSRT (Joystick Right) on Zen Cape
#define JOYSTICK_RIGHT_MASK (1 << 15)


// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM 0x00000           // Address of DRAM
#define OFFSET 0x200                    // Skip 0x100 for Stack,
                                        // 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedMemStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;

static void driveLeds()
{
    __delay_cycles(resetCycles);

    for(int j = 0; j < STR_LEN; j++) {
        for(int i = 31; i >= 0; i--) {
            if(pSharedMemStruct->ledStrip[j] & ((uint32_t)0x1 << i)) {
                __R30 |= 0x1<<DATA_PIN;      // Set the GPIO pin to 1
                __delay_cycles(oneCyclesOn-1);
                __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
                __delay_cycles(oneCyclesOff-2);
            } else {
                __R30 |= 0x1<<DATA_PIN;      // Set the GPIO pin to 1
                __delay_cycles(zeroCyclesOn-1);
                __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
                __delay_cycles(zeroCyclesOff-2);
            }
        }
    }

    // Send Reset
    __R30 &= ~(0x1<<DATA_PIN);   // Clear the GPIO pin
    __delay_cycles(resetCycles);
}

static void setAllLeds(volatile sharedMemStruct_t* sharedMem, uint32_t colour)
{
    for (int i = 0; i < STR_LEN; i++){
        sharedMem->ledStrip[i] = colour;
    }
}

void main(void)
{
    // Initialize:
    pSharedMemStruct->isLedOn = true;
    pSharedMemStruct->isButtonPressed = false;

    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    while (true) {

        // Drive LED from shared memory
        if (pSharedMemStruct->isLedOn) {
            __R30 |= DIGIT_ON_OFF_MASK;
        } else {
            __R30 &= ~DIGIT_ON_OFF_MASK;
        }

        // Sample button state to shared memory
        pSharedMemStruct->isButtonPressed = (__R31 & JOYSTICK_RIGHT_MASK) != 0;

        if ((__R31 & JOYSTICK_RIGHT_MASK) != 0) {
            setAllLeds(pSharedMemStruct, 0x0f000000);
        } else {
            setAllLeds(pSharedMemStruct, 0x000f0000);
        }

        driveLeds();
    }
}