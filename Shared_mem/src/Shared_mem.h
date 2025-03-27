#pragma once

#include <pthread.h>

// Shared clock structure
struct SharedClock {
    int currentTimeInSeconds;
    pthread_mutex_t clockMutex;
};

// Placeholder for other shared data structures
// Example: Add PSRData, SSRData, Alerts, Commands, etc.
struct PSRData {
    int id;
    float x, y, z;
};

struct SSRData {
    int id;
    float x, y, fl;
    float xspeed, yspeed, zspeed;
};

#define MAX_AIRCRAFT 100

struct SharedMemory {
    SharedClock simClock;

    // Radar data
    PSRData psrData[MAX_AIRCRAFT];
    int psrDataCount;

    SSRData ssrData[MAX_AIRCRAFT];
    int ssrDataCount;

    pthread_mutex_t radarDataMutex;

    // You can add more sections here (e.g., commands, alerts, logs)
};
