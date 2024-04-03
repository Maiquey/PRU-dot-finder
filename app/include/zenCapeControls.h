// High-level module for sampling user input on the zen cape
// spawns two threads to continuously monitor and sample the joystick and accelerometer
// calls the necessary functions from drumBeat interface when certain events triggered

#ifndef _ZENCAPE_CONTROLS_H_
#define _ZENCAPE_CONTROLS_H_

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include "hal/accelerometer.h"
#include "hal/timing.h"

void zenCapeControls_init(pthread_cond_t* stopCondVar);
void zenCapeControls_cleanup(void);

#endif