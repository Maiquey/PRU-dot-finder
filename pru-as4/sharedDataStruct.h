#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>
// #include <stdalign.h>

//  WARNING:
//  Fields in the struct must be aligned to match ARM's alignment
//      bool/char, uint8_t: byte aligned
//      int/long, uint32_t: word (4 byte) aligned
//      double, uint64_t: dword (8 byte) aligned
//  Add padding fields (char _p1) to pad out to alignment.

//  My Shared Memory Structure
// ----------------------------------------------------------------
typedef struct {
    bool isLedOn;
    bool isButtonPressed;  
    uint8_t pad1;
    uint8_t pad2;
    uint32_t ledStrip[8];
} sharedMemStruct_t;
#endif