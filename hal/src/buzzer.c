#include "hal/buzzer.h"

#define BUZZER_PERIOD_FILE "/dev/bone/pwm/0/a/period"
#define BUZZER_DUTY_CYCLE_FILE "/dev/bone/pwm/0/a/duty_cycle"
#define BUZZER_ENABLE_FILE "/dev/bone/pwm/0/a/enable"

#define NOTE_C 261.63
#define NOTE_D 293.66
#define NOTE_E 329.63
#define NOTE_F 349.23
#define NOTE_G 392.00
#define NOTE_A 440.00
#define NOTE_B 493.88
#define NOTE_HIGH_C 523.25

static pthread_t thread;

static bool is_initialized = false;
static bool isRunning = true;

// // Main thread function
// // Continuosly samples the a2d reading of potentiometer and adjusts frequency of led accordingly
// static void* updatePWM()
// {
//     long long startTime = getTimeInMs() + 100; //start to initially set the frequency
//     while (isRunning) {
//         long long currentTime = getTimeInMs();
//         if (currentTime - startTime >= 100){
//                 int a2dReading = getVoltage0Reading();
//                 if (a2dReading != potReading){
//                 potReading = a2dReading;
//                 currentFreq = potReading / FREQUENCY_DIV_FACTOR;
//                 int period = NANOSECONDS_IN_A_SECOND / currentFreq;
//                 int dutyCycle = period / 2;
//                 if (currentFreq == 0){
//                     writeValueToFile(LED_ENABLE_FILE, 0);
//                     ledOn = false;
//                 } else {
//                     writeValueToFile(LED_PERIOD_FILE, period);
//                     writeValueToFile(LED_DUTY_CYCLE_FILE, dutyCycle);
//                     if (!ledOn){
//                         writeValueToFile(LED_ENABLE_FILE, 1);
//                         ledOn = true;
//                     }
//                 }
//             }
//             startTime = currentTime;
//         }
//     }
//     pthread_exit(NULL);
// }

// Write integer value to pwm files
static void writeValueToFile(const char* filePath, int value)
{
    FILE *f = fopen(filePath, "w");
    if (!f) {
        printf("ERROR: Unable to open file.\n");
        exit(-1);
    }
    int charWritten = fprintf(f, "%d", value);
    if (charWritten <= 0) {
        printf("ERROR WRITING DATA");
        exit(1);
    }
    fclose(f);
}

// From Assignment 1
static void runCommand(char* command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}

static int getPeriod(double frequency)
{
    return (int)((1.0 / frequency)*1000000000);
}

void Buzzer_init(void)
{
    assert(!is_initialized);
    runCommand("config-pin p9_22 pwm");
    is_initialized = true;
}

void Buzzer_cleanup(void){
    assert(is_initialized);
    is_initialized = false;
}

void Buzzer_playHit(void){
    
    writeValueToFile(BUZZER_ENABLE_FILE, 1);
    double freq = 1000.00;
    while (freq > 0){
        int period = getPeriod(freq);
        writeValueToFile(BUZZER_PERIOD_FILE, period);
        writeValueToFile(BUZZER_DUTY_CYCLE_FILE, period / 2);
        freq -= 20.00;
        sleepForMs(10); 
    }
    writeValueToFile(BUZZER_ENABLE_FILE, 0);
}

void Buzzer_playMiss(void){

    writeValueToFile(BUZZER_ENABLE_FILE, 1);
    for (int i = 0; i < 4; i++){
        double freq = 600.00;
        while (freq > 200.00){
            int period = getPeriod(freq);
            writeValueToFile(BUZZER_PERIOD_FILE, period);
            writeValueToFile(BUZZER_DUTY_CYCLE_FILE, period / 2);
            freq -= 20.00;
            sleepForMs(6); 
        }
    }
    writeValueToFile(BUZZER_ENABLE_FILE, 0);
}