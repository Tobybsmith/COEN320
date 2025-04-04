#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <vector>
#include <pthread.h>
#include "../../Shared_mem/src/Shared_mem.h"

using namespace std;

const char* SHM_NAME = "/Shared_mem";
const size_t MAX_AIRCRAFT = 100; // Maximum number of aircraft as defined in shared memory

// Shared memory structures (as defined in your shared memory header)

struct SharedClock {
    int currentTimeInSeconds;
    pthread_mutex_t clockMutex;
};

struct PSRData {
    int id;
    float x, y, z;
};

struct SSRData {
    int id;
    float x, y, fl;  // fl: flight level (altitude)
    float xspeed, yspeed, zspeed;
};

struct SharedMemory {
    SharedClock simClock;
    PSRData psrData[MAX_AIRCRAFT];
    int psrDataCount;
    SSRData ssrData[MAX_AIRCRAFT];
    int ssrDataCount;
    pthread_mutex_t radarDataMutex;
};

volatile bool keepRunning = true;
void intHandler(int) { keepRunning = false; }

// Check if two SSRData entries satisfy the separation constraints.
// Horizontal separation must be >= 3000 units and vertical (flight level) separation >= 1000 units.
bool checkSeparation(const SSRData &a1, const SSRData &a2) {
    float dx = a1.x - a2.x;
    float dy = a1.y - a2.y;
    float horizontalDistance = sqrt(dx * dx + dy * dy);
    float verticalDiff = fabs(a1.fl - a2.fl);
    return (horizontalDistance >= 3000 && verticalDiff >= 1000);
}

int main() {
    // Register signal handler for graceful termination (Ctrl+C)
    signal(SIGINT, intHandler);

    // Create or open the shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Failed to open shared memory." << endl;
        return 1;
    }

    // Set the size of the shared memory region to the size of SharedMemory
    size_t shm_size = sizeof(SharedMemory);
    if (ftruncate(shm_fd, shm_size) == -1) {
        cerr << "Failed to resize shared memory." << endl;
        close(shm_fd);
        return 1;
    }

    // Map the shared memory into our address space
    SharedMemory* sharedMem = (SharedMemory*) mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMem == MAP_FAILED) {
        cerr << "Failed to map shared memory." << endl;
        close(shm_fd);
        return 1;
    }

    // Close the file descriptor (the mapping remains active)
    close(shm_fd);

    // Initialize dummy SSRData for testing purposes.
    // Some pairs are placed too close to trigger separation violations.
    sharedMem->ssrDataCount = 7;
    sharedMem->ssrData[0] = {1, 10000.0f, 10000.0f, 5000.0f, 300.0f, 0.0f, 0.0f};
    sharedMem->ssrData[1] = {2, 10500.0f, 10200.0f, 5100.0f, 300.0f, 0.0f, 0.0f};
    sharedMem->ssrData[2] = {3, 20000.0f, 20000.0f, 7000.0f, 300.0f, 0.0f, 0.0f};
    sharedMem->ssrData[3] = {4, 40000.0f, 40000.0f, 9000.0f, 300.0f, 0.0f, 0.0f};
    sharedMem->ssrData[4] = {5, 41000.0f, 40500.0f, 9100.0f, 300.0f, 0.0f, 0.0f};
    sharedMem->ssrData[5] = {6, 15000.0f, 15000.0f, 6000.0f, 300.0f, 0.0f, 0.0f};
    sharedMem->ssrData[6] = {7, 15050.0f, 14950.0f, 6100.0f, 300.0f, 0.0f, 0.0f};

    cout << "ATC Computer System started. Monitoring SSR data for separation violations..." << endl;

    // Periodically check for separation violations among SSR aircraft.
    while (keepRunning) {
        int nAircraft = sharedMem->ssrDataCount;
        vector<pair<int, int>> violations; // Holds pairs of aircraft IDs that violate separation

        // Check every unique pair of SSR aircraft
        for (int i = 0; i < nAircraft; i++) {
            for (int j = i + 1; j < nAircraft; j++) {
                if (!checkSeparation(sharedMem->ssrData[i], sharedMem->ssrData[j])) {
                    violations.push_back({sharedMem->ssrData[i].id, sharedMem->ssrData[j].id});
                }
            }
        }

        if (!violations.empty()) {
            for (auto &p : violations) {
                cout << "Alert: Separation violation detected between SSR aircraft "
                     << p.first << " and " << p.second << endl;
            }
        } else {
            cout << "No separation violations detected among SSR aircraft." << endl;
        }

        // Sleep for 5 seconds before the next check
        sleep(5);
    }

    // Clean up the shared memory mapping
    munmap(sharedMem, shm_size);

    cout << "ATC Computer System shutting down." << endl;
    return 0;
}
