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

using namespace std;

// Forward-declare or include your shared memory structures
struct AircraftCommand {
    int id;
    float xspeed;
    float yspeed;
    float zspeed;
    // ... other fields if needed
};

struct SSRData {
    int id;
    float x, y, fl;
    float xspeed, yspeed, zspeed;
};

#define MAX_AIRCRAFT 100

// Example shared memory structure (adjust to match your actual code)
struct SharedMemory {
    SSRData ssrData[MAX_AIRCRAFT];
    int ssrDataCount;
    AircraftCommand consoleCommand;    // A place to store the command
    pthread_mutex_t radarDataMutex;    // Mutex to protect SSR data/commands
};

volatile bool keepRunning = true;
void intHandler(int) { keepRunning = false; }

// Example separation check (horizontal >=3000, vertical >=1000)
bool checkSeparation(const SSRData &a1, const SSRData &a2) {
    float dx = a1.x - a2.x;
    float dy = a1.y - a2.y;
    float horizontalDist = sqrtf(dx * dx + dy * dy);
    float verticalDiff = fabs(a1.fl - a2.fl);
    return (horizontalDist >= 3000.0f && verticalDiff >= 1000.0f);
}

int main() {
    signal(SIGINT, intHandler);

    // Open or create shared memory
    const char* SHM_NAME = "/ATC_SharedData";
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Failed to open shared memory." << endl;
        return 1;
    }

    size_t shm_size = sizeof(SharedMemory);
    if (ftruncate(shm_fd, shm_size) == -1) {
        cerr << "Failed to resize shared memory." << endl;
        close(shm_fd);
        return 1;
    }

    SharedMemory* sharedMem = (SharedMemory*)mmap(nullptr, shm_size,
        PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMem == MAP_FAILED) {
        cerr << "Failed to map shared memory." << endl;
        close(shm_fd);
        return 1;
    }
    close(shm_fd); // Not needed after mmap

    cout << "Computer system started. Checking SSR data for separation violations..." << endl;

    while (keepRunning) {
        int nAircraft = sharedMem->ssrDataCount;

        // We'll collect violation pairs just to print them, or you can
        // immediately write commands for each violation.
        vector<pair<int, int>> violations;

        // Lock if you're reading from ssrData and possibly writing commands
        pthread_mutex_lock(&sharedMem->radarDataMutex);

        for (int i = 0; i < nAircraft; i++) {
            for (int j = i + 1; j < nAircraft; j++) {
                if (!checkSeparation(sharedMem->ssrData[i], sharedMem->ssrData[j])) {
                    // We have a violation
                    violations.push_back({sharedMem->ssrData[i].id, sharedMem->ssrData[j].id});

                    // Example: Write a command for one or both aircraft
                    // (Here we just write a command for the first one in the pair)
                    AircraftCommand ac;
                    ac.id = sharedMem->ssrData[i].id;
                    // Decide new speeds or some other action
                    ac.xspeed = 200.0f;
                    ac.yspeed = 0.0f;
                    ac.zspeed = -100.0f;

                    // Write to shared memory so another process (e.g. a console or flight manager)
                    // can see and act on this command
                    sharedMem->consoleCommand = ac;
                }
            }
        }

        pthread_mutex_unlock(&sharedMem->radarDataMutex);

        if (!violations.empty()) {
            for (auto &v : violations) {
                cout << "Violation between aircraft " << v.first
                     << " and " << v.second << endl;
            }
        } else {
            cout << "No separation violations detected." << endl;
        }

        // Sleep before next check
        sleep(5);
    }

    // Unmap the shared memory
    munmap(sharedMem, shm_size);

    cout << "Computer system shutting down." << endl;
    return 0;
}
