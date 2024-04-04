#ifndef _BUZZER_H
#define _BUZZER_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include "timing.h"

void Buzzer_init(void);
void Buzzer_cleanup(void);
void Buzzer_playHit(void);
void Buzzer_playMiss(void);

#endif