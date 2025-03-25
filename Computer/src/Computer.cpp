#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <vector>

using namespace std;

const char* SHM_NAME = "/ATC_SharedData";
const size_t MAX_AIRCRAFT = 10;

// Structure representing an aircraft's data
struct Aircraft {
    int id;
    double x;
    double y;
    double z;
    double speedX;
    double speedY;
    double speedZ;
};

// Structure stored in shared memory
struct SharedData {
    int numAircraft;
    Aircraft aircrafts[MAX_AIRCRAFT];
};

volatile bool keepRunning = true;
void intHandler(int) { keepRunning = false; }

// Check if two aircraft satisfy the separation constraints.
// Horizontal separation must be >= 3000 units and vertical separation >= 1000 units.
bool checkSeparation(const Aircraft &a1, const Aircraft &a2) {
    double dx = a1.x - a2.x;
    double dy = a1.y - a2.y;
    double horizontalDistance = sqrt(dx * dx + dy * dy);
    double verticalDiff = fabs(a1.z - a2.z);
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

    // Set the size of the shared memory region
    size_t shm_size = sizeof(SharedData);
    if (ftruncate(shm_fd, shm_size) == -1) {
        cerr << "Failed to resize shared memory." << endl;
        close(shm_fd);
        return 1;
    }

    // Map the shared memory into our address space
    SharedData* sharedData = (SharedData*) mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedData == MAP_FAILED) {
        cerr << "Failed to map shared memory." << endl;
        close(shm_fd);
        return 1;
    }

    // Close the file descriptor (mapping remains)
    close(shm_fd);

    // Reinitialize shared data with dummy aircraft data for testing
    sharedData->numAircraft = 7;

    // Dummy aircraft data:
    // Aircraft 1 and 2 are placed very close -> violation.
    sharedData->aircrafts[0] = {1, 10000, 10000, 5000, 300, 0, 0};
    sharedData->aircrafts[1] = {2, 10500, 10200, 5100, 300, 0, 0};

    // Aircraft 3 is well separated.
    sharedData->aircrafts[2] = {3, 20000, 20000, 7000, 300, 0, 0};

    // Aircraft 4 and 5 are placed close -> violation.
    sharedData->aircrafts[3] = {4, 40000, 40000, 9000, 300, 0, 0};
    sharedData->aircrafts[4] = {5, 41000, 40500, 9100, 300, 0, 0};

    // Aircraft 6 and 7 are placed very close -> violation.
    sharedData->aircrafts[5] = {6, 15000, 15000, 6000, 300, 0, 0};
    sharedData->aircrafts[6] = {7, 15050, 14950, 6100, 300, 0, 0};

    cout << "Computer System started. Monitoring aircraft for separation violations..." << endl;

    // Periodically check for safety violations
    while (keepRunning) {
        int nAircraft = sharedData->numAircraft;
        vector<pair<int, int>> violations; // to hold all violation pairs

        // Check every unique pair of aircraft
        for (int i = 0; i < nAircraft; i++) {
            for (int j = i + 1; j < nAircraft; j++) {
                if (!checkSeparation(sharedData->aircrafts[i], sharedData->aircrafts[j])) {
                    violations.push_back({sharedData->aircrafts[i].id, sharedData->aircrafts[j].id});
                }
            }
        }

        if (!violations.empty()) {
            for (auto &p : violations) {
                cout << "Alert: Separation violation detected between aircraft "
                     << p.first << " and " << p.second << endl;
            }
        } else {
            cout << "No separation violations detected." << endl;
        }

        // Sleep for a period (simulate periodic checking, e.g., every 5 seconds)
        sleep(5);
    }

    // Clean up shared memory mapping
    munmap(sharedData, shm_size);

    // Optionally, unlink the shared memory object if no longer needed.
    // shm_unlink(SHM_NAME);

    cout << "Computer System shutting down." << endl;
    return 0;
}
