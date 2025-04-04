#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "Shared_mem.h"

#define SHM_NAME "/Shared_mem"

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    SharedMemory* sharedMem = (SharedMemory*) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&sharedMem->simClock.clockMutex, &mutexAttr);
    pthread_mutex_init(&sharedMem->radarDataMutex, &mutexAttr);
    pthread_mutex_init(&sharedMem->displayConsoleMutex, &mutexAttr);

    sharedMem->simClock.currentTimeInSeconds = 0;
    sharedMem->psrDataCount = 0;
    sharedMem->ssrDataCount = 0;
    sharedMem->displayDetailId = -1;


    std::cout << "Shared memory initialized. Press Ctrl+C to terminate.\n";

    while (true) {
        pthread_mutex_lock(&sharedMem->simClock.clockMutex);
        int time = sharedMem->simClock.currentTimeInSeconds;
        pthread_mutex_unlock(&sharedMem->simClock.clockMutex);

        pthread_mutex_lock(&sharedMem->radarDataMutex);

        std::cout << "\n=== Shared Memory Snapshot ===\n";
        std::cout << "Simulation Time: " << time << "s\n";

        std::cout << "\nPSR Data (" << sharedMem->psrDataCount << " aircraft):\n";
        for (int i = 0; i < sharedMem->psrDataCount; ++i) {
            PSRData p = sharedMem->psrData[i];
            std::cout << "  [PSR] ID: " << p.id << " | x=" << p.x << " y=" << p.y << " z=" << p.z << "\n";
        }

        std::cout << "\nSSR Data (" << sharedMem->ssrDataCount << " aircraft):\n";
        for (int i = 0; i < sharedMem->ssrDataCount; ++i) {
            SSRData s = sharedMem->ssrData[i];
            std::cout << "  [SSR] ID: " << s.id << " | x=" << s.x << " y=" << s.y << " z=" << s.fl
                      << " | vx=" << s.xspeed << " vy=" << s.yspeed << " vz=" << s.zspeed << "\n";
        }

        pthread_mutex_unlock(&sharedMem->radarDataMutex);

        pthread_mutex_lock(&sharedMem->displayConsoleMutex);

        AircraftCommand cmd = sharedMem->consoleCommand;
        if (cmd.id != -1) {
            std::cout << "\n[Console Command] Target ID: " << cmd.id
                      << " | New Speed: vx=" << cmd.xspeed
                      << " vy=" << cmd.yspeed
                      << " vz=" << cmd.zspeed << "\n";
        } else {
            std::cout << "\n[Console Command] No command issued.\n";
        }

        AircraftCommand cc = sharedMem->computerCommand;
        if (cc.id != -1) {
            std::cout << "[Computer Command] Target ID: " << cc.id
                      << " | New Speed: vx=" << cc.xspeed
                      << " vy=" << cc.yspeed
                      << " vz=" << cc.zspeed << "\n";
        } else {
            std::cout << "[Computer Command] No command issued.\n";
        }

        pthread_mutex_unlock(&sharedMem->displayConsoleMutex);

        sleep(3);
    }

    munmap(sharedMem, sizeof(SharedMemory));
    shm_unlink(SHM_NAME);
    return 0;
}
