#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "../../Shared_mem/src/Shared_mem.h"

#define SHM_NAME "/Shared_mem"

using namespace std;

int main() {
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Map the shared memory into process address space
    SharedMemory* sharedMem = (SharedMemory*) mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMem == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(1);
    }

    cout << "[COMM] Communication system running. Monitoring for new commands...\n";

    while (true) {
        // Check Console Command
        pthread_mutex_lock(&sharedMem->displayConsoleMutex);
        AircraftCommand consoleCmd = sharedMem->consoleCommand;
        pthread_mutex_unlock(&sharedMem->displayConsoleMutex);

        if (consoleCmd.id != -1) {
            cout << "[COMM] Command from CONSOLE -> Aircraft ID: " << consoleCmd.id
                 << " | New Speed: vx=" << consoleCmd.xspeed
                 << ", vy=" << consoleCmd.yspeed
                 << ", vz=" << consoleCmd.zspeed << endl;
        }

        // Check Computer Command (placeholder)
        // You must add `AircraftCommand computerCommand;` to SharedMemory structure
        AircraftCommand computerCmd = sharedMem->computerCommand; // assumed added
        if (computerCmd.id != -1) {
            cout << "[COMM] Command from COMPUTER -> Aircraft ID: " << computerCmd.id
                 << " | New Speed: vx=" << computerCmd.xspeed
                 << ", vy=" << computerCmd.yspeed
                 << ", vz=" << computerCmd.zspeed << endl;
        }

        sleep(2);
    }

    munmap(sharedMem, sizeof(SharedMemory));
    close(shm_fd);
    return 0;
}
