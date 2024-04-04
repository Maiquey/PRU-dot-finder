#include "hal/buzzer.h"

#define BUZZER_PERIOD_FILE "/dev/bone/pwm/0/a/period"
#define BUZZER_DUTY_CYCLE_FILE "/dev/bone/pwm/0/a/duty_cycle"
#define BUZZER_ENABLE_FILE "/dev/bone/pwm/0/a/enable"

static pthread_t thread;

static bool is_initialized = false;
static bool isRunning = false;

static bool playingHit = false;
static bool playingMiss = false;

static void* soundPlaybackThread(); // thread function

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

// Helper to get period based on a frequency
static int getPeriod(double frequency)
{
    return (int)((1.0 / frequency)*1000000000);
}

void Buzzer_init(void)
{
    assert(!is_initialized);
    runCommand("config-pin p9_22 pwm");
    is_initialized = true;
    isRunning = true;
    pthread_create(&thread, NULL, soundPlaybackThread, NULL);
}

void Buzzer_cleanup(void){
    assert(is_initialized);
    is_initialized = false;
    isRunning = false;
    pthread_join(thread, NULL);
    writeValueToFile(BUZZER_ENABLE_FILE, 0);
}

// Interface function for registering a hit
void Buzzer_playHit(void){
    if (!playingHit){
        playingHit = true;
    }
}

// Interface function for registering a miss
void Buzzer_playMiss(void){
    if (!playingHit && !playingMiss){
        playingMiss = true;
    }
}

//background thread
static void* soundPlaybackThread()
{
    while (isRunning) {
        if (playingHit){
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
            playingHit = false;
        } else if (playingMiss) {
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
            playingMiss = false;
        }
    }
    pthread_exit(NULL);
}