#include "zenCapeControls.h"
#include "hal/pruDriver.h"

#define NO_INPUT -1
#define JOYSTICK_UP 0
#define JOYSTICK_RIGHT 1
#define JOYSTICK_DOWN 2
#define JOYSTICK_LEFT 3
#define JOYSTICK_IN 4

#define JOYSTICK_DEBOUNCE_TIME 200
#define ACCELEROMETER_BASS_DEBOUNCE_TIME 100
#define ACCELEROMETER_HIHAT_DEBOUNCE_TIME 100
#define ACCELEROMETER_SNARE_DEBOUNCE_TIME 100

// 1 G is around 16000
#define X_G_DIVISOR 12000 //decrease this value to make Hihat more sensitive
#define Y_G_DIVISOR 14000 //decrease this value to make Snare more sensitive
#define Z_G_DIVISOR 16000 //decrease this value to make Bass more sensitive
#define _1_G_DIVISOR 16000

#define CORRECT_TARGET_NOISE 0.05

static pthread_cond_t* mainCondVar;

static void* joystickInputThread();
static void* accelerometerSamplingThread();
static pthread_t joyStickThreadId;
static pthread_t accelerometerThreadId;

static bool is_initialized = false;
static bool isRunning = false;
static bool onTarget = false;

static double target_x;
static double target_y;

static void generateNewTarget(void)
{
    // Generate a random target (x, y) where each is in the range [-0.5, 0.5]
    // Generate a random number between 0 and 1, then subtract 0.5
    target_x = ((double)rand() / RAND_MAX) - 0.5;
    target_y = ((double)rand() / RAND_MAX) - 0.5;
}

void zenCapeControls_init(pthread_cond_t* stopCondVar)
{
    assert(!is_initialized);
    srand(time(NULL));
    isRunning = true;
    mainCondVar = stopCondVar;
    generateNewTarget();
    pthread_create(&joyStickThreadId, NULL, joystickInputThread, NULL);
    pthread_create(&accelerometerThreadId, NULL, accelerometerSamplingThread, NULL);
    is_initialized = true;
}

void zenCapeControls_cleanup(void)
{
    assert(is_initialized);
    isRunning = false;
    pthread_join(joyStickThreadId, NULL);
    pthread_join(accelerometerThreadId, NULL);
    is_initialized = false;
}

// Thread to sample joystick input
static void* joystickInputThread()
{
    long long debounceTimestamp = getTimeInMs();
    bool isPressedIn = false;
    while(isRunning){
        if (getTimeInMs() - debounceTimestamp > JOYSTICK_DEBOUNCE_TIME){
        //     int joystickID = joystick_getJoyStickPress();
        //     if (joystickID != NO_INPUT){
        //         switch (joystickID) {
        //             case JOYSTICK_IN:
        //                 // change beat
        //                 if(!isPressedIn){
        //                     drumBeat_cycleBeat();
        //                 }
        //                 break;
        //             case JOYSTICK_UP:
        //                 // volume up by 5
        //                 drumBeat_adjustVolume(5);
        //                 break;
        //             case JOYSTICK_DOWN:
        //                 // volume down by 5
        //                 drumBeat_adjustVolume(-5);
        //                 break;
        //             case JOYSTICK_LEFT:
        //                 // BPM down by 5
        //                 drumBeat_adjustBPM(-5);
        //                 break;
        //             case JOYSTICK_RIGHT:
        //                 // BPM up by 5
        //                 drumBeat_adjustBPM(5);
        //                 break;
        //         }
        //         debounceTimestamp = getTimeInMs();
        //     }
        //     isPressedIn = joystick_isPressedIn(); // for stopping loop between modes
        // }
        // //sample joystick every 10s
        
        // if (PruDriver_isPressedRight()){
        //     PruDriver_setAllLeds(0x000f0f00);
        // } else {
        //     PruDriver_setAllLeds(0x0f000f00);
        // }
            if (PruDriver_isPressedRight()) {
                if (onTarget){
                    printf("hit!!\n");
                    generateNewTarget();
                }
                
                printf("hi\n");
                // pthread_cond_signal(mainCondVar);
                // isRunning = false;
                debounceTimestamp = getTimeInMs();
            }
        }
        sleepForMs(10);
    }
    pthread_exit(NULL);
}

// Thread to sample accelerometer values
static void* accelerometerSamplingThread()
{
    // X normal range = {176, 352}
    // Y normal range = {-384, -32}
    // Z normal range = {16464, 16672}
    // 1 G is about 16000 register val
    // long long debounceTimerBass = getTimeInMs();
    // long long debounceTimerHiHat = debounceTimerBass;
    // long long debounceTimerSnare = debounceTimerBass;
    // unsigned char* accInitial = accelerometer_readOutVals();
    // int16_t x_last = ((accInitial[1] << 8) | accInitial[0]) / X_G_DIVISOR;
    // int16_t y_last = ((accInitial[3] << 8) | accInitial[2]) / Y_G_DIVISOR;
    // int16_t z_last = ((accInitial[5] << 8) | accInitial[4]) / Z_G_DIVISOR;
    // free(accInitial);
    while(isRunning){
        unsigned char* accelerometerOutput = accelerometer_readOutVals();
        int16_t x_data_p = ((accelerometerOutput[1] << 8) | accelerometerOutput[0]);
        int16_t y_data_p = ((accelerometerOutput[3] << 8) | accelerometerOutput[2]);
        int16_t z_data_p = ((accelerometerOutput[5] << 8) | accelerometerOutput[4]);
        double x_data = (double)x_data_p / (double)_1_G_DIVISOR;
        double y_data = (double)y_data_p / (double)_1_G_DIVISOR;
        double z_data = (double)z_data_p / (double)_1_G_DIVISOR;
        
        //Algorithm: Baseline G's for X and Y direction is 0, Z is 1
        //    If the last reading on X or Y was lower G value at zero, and current reading is baseline or above, trigger a sound
        //    Similar case for Z direction just reversed
        //    G calculations for X and Y have been adjusted as it's a bit harder to generate 1 full G for them as opposed to Z
        //    Essentially -1 G's for X or Y will generate a hi-hat/snare and 2G's for Z will generate bass drum
        //    X and Y are calculated on negative G values simply based on the direction - as shown in Dr. Brian's video demonstration
        // if ((getTimeInMs() - debounceTimerHiHat > ACCELEROMETER_HIHAT_DEBOUNCE_TIME) 
        //     && x_last < 0 
        //     && x_data >= 0){
        //     drumBeat_playHiHat();
        //     debounceTimerHiHat = getTimeInMs();
        // }
        // if ((getTimeInMs() - debounceTimerSnare > ACCELEROMETER_SNARE_DEBOUNCE_TIME) 
        //     && y_last < 0 
        //     && y_data >= 0){
        //     drumBeat_playHardSnare();
        //     debounceTimerSnare = getTimeInMs();
        // }
        // if ((getTimeInMs() - debounceTimerBass > ACCELEROMETER_BASS_DEBOUNCE_TIME) 
        //     && z_last > 1 
        //     && z_data <= 1){
        //     drumBeat_playBass();
        //     debounceTimerBass = getTimeInMs();
        // }
        // x_last = x_data;
        // y_last = y_data;
        // z_last = z_data;
        // printf("(x, y, z) = (%.4f, %.4f, %.4f)\n", x_data, y_data, z_data);
        if (x_data < (target_x - CORRECT_TARGET_NOISE)) {
            onTarget = false;
            PruDriver_setAllLeds(0x01000000);
        } else if (x_data > (target_x + CORRECT_TARGET_NOISE)) {
            onTarget = false;
            PruDriver_setAllLeds(0x00010000);
        } else {
            onTarget = true;
            PruDriver_setAllLeds(0x00000100);
        }
        // if (x_data_p < -10000){ //Left - Red
        //     PruDriver_setAllLeds(0x000f0000);
        // } else if (x_data_p > 10000){ //Right - Green
        //     PruDriver_setAllLeds(0x0f000000);
        // } else if (y_data_p < -10000){ //Up - Yellow
        //     PruDriver_setAllLeds(0x0f0f0000);
        // } else if (y_data_p > 10000){ //Down - Teal
        //     PruDriver_setAllLeds(0x0f000f00);
        // } else {
        //     PruDriver_setAllLeds(0x000f0f00);
        // }
        free(accelerometerOutput);
        sleepForMs(10); // sample every 10ms
    }
    pthread_exit(NULL);
}