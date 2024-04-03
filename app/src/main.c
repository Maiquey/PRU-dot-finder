// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <stdbool.h>
#include "zenCapeControls.h"
#include "hal/pruDriver.h"

pthread_mutex_t mutexMain;
pthread_cond_t condVarFinished;

int main()
{
    // Initialize main mutex and cond var

    pthread_mutex_init(&mutexMain, NULL);
    pthread_cond_init(&condVarFinished, NULL);

    // Initialize all modules; HAL modules first
    PruDriver_init();
    accelerometer_init();
    zenCapeControls_init(&condVarFinished);
    // Network_init(&condVarFinished);
    
    // main logic
    pthread_mutex_lock(&mutexMain);
    pthread_cond_wait(&condVarFinished, &mutexMain);
    pthread_mutex_unlock(&mutexMain);

    // Cleanup all modules (HAL modules last)
    // Network_cleanup();
    zenCapeControls_cleanup();
    accelerometer_cleanup();
    PruDriver_cleanup();

    // Free mutex and cond var 
    
    pthread_mutex_destroy(&mutexMain);
    pthread_cond_destroy(&condVarFinished);

}