#pragma once

#include <pthread.h>

// shared clock structure
struct SharedClock {
    int currentTimeInSeconds;
    pthread_mutex_t clockMutex;
};


// shared PSRData
struct PSRData {
	float x, y, z;
	int id;
    // Add more if needed
};
// shared SSRData
struct SSRData {
	int id;
	float fl, xspeed, yspeed, zspeed, x, y;
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
