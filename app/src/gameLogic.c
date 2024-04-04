#include "gameLogic.h"
#include "hal/pruDriver.h"

#define NO_INPUT -1

#define JOYSTICK_DEBOUNCE_TIME 200

#define _1_G_DIVISOR 16000

#define CORRECT_TARGET_NOISE 0.1

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

static int targetsHit = 0;

enum displayColour{
    GREEN,
    BLUE,
    RED
};

static void generateNewTarget(void)
{
    // Generate a random target (x, y) where each is in the range [-0.5, 0.5]
    // Generate a random number between 0 and 1, then subtract 0.5
    target_x = ((double)rand() / RAND_MAX) - 0.5;
    target_y = ((double)rand() / RAND_MAX) - 0.5;
}

void gameLogic_init(pthread_cond_t* stopCondVar)
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

void gameLogic_cleanup(void)
{
    assert(is_initialized);
    isRunning = false;
    pthread_join(joyStickThreadId, NULL);
    pthread_join(accelerometerThreadId, NULL);
    PruDriver_setAllLeds(0x00000000);
    is_initialized = false;
}

// Thread to sample joystick input
static void* joystickInputThread()
{
    long long debounceTimestamp = getTimeInMs();
    bool isPressedIn = false;
    while(isRunning){
        if (getTimeInMs() - debounceTimestamp > JOYSTICK_DEBOUNCE_TIME){
            if (PruDriver_isPressedDown()) {
                if (onTarget){
                    generateNewTarget();
                    targetsHit++;
                    debounceTimestamp = getTimeInMs();
                    Buzzer_playHit();
                } else {
                    Buzzer_playMiss();
                }
                
            } else if (PruDriver_isPressedRight()) {
                pthread_cond_signal(mainCondVar);
                isRunning = false;
            }
        }
        SegDisplay_setNumber(targetsHit);
        sleepForMs(10);
    }
    pthread_exit(NULL);
}

// Thread to sample accelerometer values
static void* accelerometerSamplingThread()
{
    while(isRunning){
        unsigned char* accelerometerOutput = accelerometer_readOutVals();
        int16_t x_data_p = ((accelerometerOutput[1] << 8) | accelerometerOutput[0]);
        int16_t y_data_p = ((accelerometerOutput[3] << 8) | accelerometerOutput[2]);
        double x_data = (double)x_data_p / (double)_1_G_DIVISOR;
        double y_data = (double)y_data_p / (double)_1_G_DIVISOR;
        
        enum displayColour currentColour;
        if (x_data < (target_x - CORRECT_TARGET_NOISE)) {
            currentColour = GREEN;
        } else if (x_data > (target_x + CORRECT_TARGET_NOISE)) {
            currentColour = RED;
        } else {
            currentColour = BLUE;
        }
        int index;
        if (y_data <= (target_y + CORRECT_TARGET_NOISE) && y_data >= (target_y - CORRECT_TARGET_NOISE)){
            switch(currentColour){
                case GREEN:
                    onTarget = false;
                    PruDriver_setAllLeds(0x0f000000);
                    break;
                case BLUE:
                    onTarget = true;
                    PruDriver_setAllLeds(0x00000f00);
                    break;
                case RED:
                    onTarget = false;
                    PruDriver_setAllLeds(0x000f0000);
                    break;
            }
        } else{
            if (y_data > (target_y + CORRECT_TARGET_NOISE)){ 
                // too low, display 4-8 
                // (8 is out of bounds, but this is safe to pass into the trioLED pruDriver method)
                double distance = y_data - target_y;
                index = 8;
                while (index > 3){
                    if (distance > CORRECT_TARGET_NOISE*(index-3)){
                        break;
                    }
                    index--;
                }
            } else {
                // too high, display -1-3
                // (-1 is out of bounds, but this is safe to pass into the trioLED pruDriver method)
                double distance = y_data - target_y;
                index = -1;
                while (index < 4){
                    if (distance < (-1)*CORRECT_TARGET_NOISE*(4-index)){
                        break;
                    }
                    index++;
                }
            }
            switch(currentColour){
                case GREEN:
                    onTarget = false;
                    PruDriver_setTrioLeds(index, 0x0f000000, 0x01000000);
                    break;
                case BLUE:
                    onTarget = false;
                    PruDriver_setTrioLeds(index, 0x00000f00, 0x00000100);
                    break;
                case RED:
                    onTarget = false;
                    PruDriver_setTrioLeds(index, 0x000f0000, 0x00010000);
                    break;
            }
        } 
        
        free(accelerometerOutput);
        sleepForMs(10); // sample every 10ms
    }
    pthread_exit(NULL);
}