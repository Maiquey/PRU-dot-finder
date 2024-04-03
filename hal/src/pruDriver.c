#include "hal/pruDriver.h"

static bool isInitialized = false;

static volatile void *pPruBase;
static volatile sharedMemStruct_t *pSharedPru0;

// Return the address of the PRU's base memory
static volatile void* getPruMmapAddr(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem");
        exit(EXIT_FAILURE);
    }

    // Points to start of PRU memory.
    volatile void* pPruBase = mmap(0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    
    if (pPruBase == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return pPruBase;
}

static void freePruMmapAddr(volatile void* pPruBase){
    if (munmap((void*) pPruBase, PRU_LEN)) {
        perror("PRU munmap failed");
        exit(EXIT_FAILURE);
    }
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

void runSimplePruProgram(void) {
    printf("Sharing memory with PRU\n");
    printf(" LED should toggle each second\n");
    printf(" Press the button to see its state here.\n");

    // Get access to shared memory for my uses
    volatile void *pPruBase = getPruMmapAddr();
    volatile sharedMemStruct_t *pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);

    // Drive it
    for (int i = 0; i < 20; i++) {
        // Drive LED
        pSharedPru0->isLedOn = (i % 2 == 0);

        // Print button
        printf("Button: %d\n", pSharedPru0->isButtonPressed);
        
        // Timing
        sleep(1);
    }

    // Cleanup
    freePruMmapAddr(pPruBase);
}

void PruDriver_init(void) {
    runCommand("config-pin P8.11 pruout");
    runCommand("config-pin P8.12 pruout");
    runCommand("config-pin P8.15 pruin");
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
    isInitialized = true;
}

void PruDriver_cleanup(void) {
    freePruMmapAddr(pPruBase);
    isInitialized = false;
}

bool PruDriver_isPressedRight(void) {
    return pSharedPru0->isButtonPressed;
}

void PruDriver_setAllLeds(uint32_t colour)
{
    for (int i = 0; i < STR_LEN; i++){
        pSharedPru0->ledStrip[i] = colour;
    }
}