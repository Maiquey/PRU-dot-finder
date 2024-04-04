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

void PruDriver_init(void) {
    runCommand("config-pin P8.11 pruout");
    // runCommand("config-pin P8.12 pruout"); // for driving seg display with PRU (not needed)
    runCommand("config-pin P8.15 pruin");
    runCommand("config-pin P8.16 pruin");
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
    isInitialized = true;
}

void PruDriver_cleanup(void) {
    freePruMmapAddr(pPruBase);
    isInitialized = false;
}

bool PruDriver_isPressedRight(void) {
    return (pSharedPru0->isRightPressed == 0);
}

bool PruDriver_isPressedDown(void) {
    return (pSharedPru0->isDownPressed == 0);
}

void PruDriver_setAllLeds(uint32_t colour)
{
    for (int i = 0; i < STR_LEN; i++){
        pSharedPru0->ledStrip[i] = colour;
    }
}

void PruDriver_setTrioLeds(int index, uint32_t colourStrong, uint32_t colourWeak)
{
    for (int i = 0; i < STR_LEN; i++){
        if (i == index){
            pSharedPru0->ledStrip[i] = colourStrong;
        } else if (i == index - 1 || i == index + 1){
            pSharedPru0->ledStrip[i] = colourWeak;
        } else {
            pSharedPru0->ledStrip[i] = 0x00000000;
        }
    }
}