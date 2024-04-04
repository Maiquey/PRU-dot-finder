// High-level module for handling game logic
// spawns two threads to continuously monitor and sample the joystick (via PRU Driver) and accelerometer
// calls the necessary functions from pruDriver interface when certain events triggered
// passed LED commands to the pruDriver

#ifndef _GAME_LOGIC_H_
#define _GAME_LOGIC_H_

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include "hal/accelerometer.h"
#include "hal/timing.h"
#include "hal/segDisplay.h"
#include "hal/buzzer.h"

void gameLogic_init(pthread_cond_t* stopCondVar);
void gameLogic_cleanup(void);

#endif