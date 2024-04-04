// High-level module for sampling user input on the zen cape
// spawns two threads to continuously monitor and sample the joystick and accelerometer
// calls the necessary functions from drumBeat interface when certain events triggered

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