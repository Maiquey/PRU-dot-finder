#ifndef _BUZZER_H
#define _BUZZER_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <string.h>
#include "timing.h"

void Buzzer_init(void);
void Buzzer_cleanup(void);
void Buzzer_playSound(void);

#endif